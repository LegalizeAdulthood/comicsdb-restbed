#include "comic.h"

#include <restbed>

#include <iostream>
#include <mutex>
#include <string>
#include <vector>

namespace comicsdb
{

std::mutex g_dbMutex;
using ComicDb = std::vector<Comic>;
using SessionPtr = std::shared_ptr<restbed::Session>;

class CustomLogger : public restbed::Logger
{
  public:
    void stop() override {}

    void start(const std::shared_ptr<const restbed::Settings> &) override {}

    void log(const Level, const char *format, ...) override
    {
        std::va_list arguments;
        va_start(arguments, format);

        vfprintf(stderr, format, arguments);
        fprintf(stderr, "\n");

        va_end(arguments);
    }

    void log_if(bool expression, const Level level, const char *format,
                ...) override
    {
        if (expression)
        {
            std::va_list arguments;
            va_start(arguments, format);
            log(level, format, arguments);
            va_end(arguments);
        }
    }
};

ComicDb load()
{
    ComicDb db;
    db.emplace_back(fromJson(
        R"json({"title":"The Fantastic Four","issue":1,"writer":"Stan Lee","penciler":"Jack Kirby","inker":"George Klein","letterer":"Artie Simek","colorist":"Stan Goldberg"})json"));
    {
        Comic comic;
        comic.title = "The Fantastic Four";
        comic.issue = 3;
        comic.writer = "Stan Lee";
        comic.penciler = "Jack Kirby";
        comic.inker = "Sol Brodsky";
        comic.letterer = "Artie Simek";
        comic.colorist = "Stan Goldberg";
        db.push_back(comic);
    }
    return db;
}

std::shared_ptr<restbed::Settings> getSettings()
{
    auto settings = std::make_shared<restbed::Settings>();
    settings->set_default_header("Connection", "close");
    return settings;
}

void notAcceptable(const SessionPtr &session, const std::string &msg)
{
    session->close(restbed::NOT_ACCEPTABLE, msg,
                   {{"Content-Type", "text/plain"},
                    {"Content-Length", std::to_string(msg.size())}});
}

bool validId(const SessionPtr &session, const ComicDb &db, std::size_t &id)
{
    const auto &request = session->get_request();
    if (request->has_path_parameter("id"))
    {
        id = request->get_path_parameter("id", 0);
        if (id < db.size() && db[id].issue != Comic::DELETED_ISSUE)
        {
            return true;
        }
        notAcceptable(session, "Not Acceptable, id out of range");
    }
    else
    {
        notAcceptable(session, "Not Acceptable, missing id");
    }
    return false;
}

void readComic(const SessionPtr &session, const ComicDb &db)
{
    std::size_t id{};
    if (validId(session, db, id))
    {
        const std::string json = toJson(db[id]);
        session->close(restbed::OK, json,
                       {{"Content-Type", "application/json"},
                        {"Content-Length", std::to_string(json.size())}});
    }
}

void deleteComic(const SessionPtr &session, ComicDb &db)
{
    std::size_t id{};
    if (validId(session, db, id))
    {
        {
            std::unique_lock<std::mutex> lock(g_dbMutex);
            db[id] = Comic{};
        }
        session->close(restbed::OK);
    }
}

void updateComic(const SessionPtr &session, ComicDb &db)
{
    std::size_t id{};
    if (!validId(session, db, id))
        return;

    auto &request = session->get_request();
    std::size_t length{};
    length = request->get_header("Content-Length", length);
    if (length == 0)
    {
        notAcceptable(session, "Not Acceptable, empty request body");
        return;
    }

    session->fetch(
        length,
        [&db, id](const SessionPtr &session, const restbed::Bytes &data)
        {
            const std::string json{reinterpret_cast<const char *>(data.data()),
                                   data.size()};
            Comic comic = fromJson(json);
            if (comic.title.empty() || comic.issue < 1 ||
                comic.writer.empty() || comic.penciler.empty() ||
                comic.inker.empty() || comic.letterer.empty() ||
                comic.colorist.empty())
            {
                notAcceptable(session, "Not Acceptable, invalid JSON");
                return;
            }

            {
                std::unique_lock<std::mutex> lock(g_dbMutex);
                db[id] = comic;
            }
            session->close(restbed::OK);
        });
}

void createComic(const SessionPtr &session, ComicDb &db)
{
    auto &request = session->get_request();
    std::size_t length{};
    length = request->get_header("Content-Length", length);
    if (length == 0)
    {
        notAcceptable(session, "Not Acceptable, empty body");
        return;
    }

    session->fetch(
        length,
        [&db](const SessionPtr &session, const restbed::Bytes &data)
        {
            const std::string json{reinterpret_cast<const char *>(data.data()),
                                   data.size()};
            Comic comic = fromJson(json);
            if (comic.title.empty() || comic.issue < 1 ||
                comic.writer.empty() || comic.penciler.empty() ||
                comic.inker.empty() || comic.letterer.empty() ||
                comic.colorist.empty())
            {
                notAcceptable(session, "Not Acceptable, invalid JSON");
                return;
            }

            {
                std::unique_lock<std::mutex> lock(g_dbMutex);
                db.push_back(comic);
            }
            session->close(restbed::OK);
        });
}

void publishResources(restbed::Service &service, ComicDb &db)
{
    auto comicResource = std::make_shared<restbed::Resource>();
    comicResource->set_path("/comic/{id: [[:digit:]]+}");
    comicResource->set_method_handler("GET", [&db](const SessionPtr &session)
                                      { return readComic(session, db); });
    comicResource->set_method_handler("DELETE", [&db](const SessionPtr &session)
                                      { return deleteComic(session, db); });
    comicResource->set_method_handler("PUT", [&db](const SessionPtr &session)
                                      { return updateComic(session, db); });
    service.publish(comicResource);

    auto createComicResource = std::make_shared<restbed::Resource>();
    createComicResource->set_path("/comic");
    auto createComicCallback = [&db](const SessionPtr &session)
    { return createComic(session, db); };
    createComicResource->set_method_handler("PUT", createComicCallback);
    createComicResource->set_method_handler("POST", createComicCallback);
    service.publish(createComicResource);
}

void runService()
{
    ComicDb db = load();

    restbed::Service service;
    publishResources(service, db);
    service.set_logger(std::make_shared<CustomLogger>());
    service.start(getSettings());
}

} // namespace comicsdb

int main()
{
    try
    {
        comicsdb::runService();
    }
    catch (const std::exception &bang)
    {
        std::cerr << bang.what() << '\n';
        return 1;
    }
    catch (...)
    {
        return 1;
    }

    return 0;
}
