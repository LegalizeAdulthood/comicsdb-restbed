#include "comic.h"

#include <restbed>

#include <iostream>
#include <string>
#include <vector>

std::vector<comicsdb::Comic> load()
{
  std::vector<comicsdb::Comic> db;
  db.emplace_back(comicsdb::fromJson(
      R"json({"title":"The Fantastic Four","issue":1,"writer":"Stan Lee","penciler":"Jack Kirby","inker":"George Klein","letterer":"Artie Simek","colorist":"Stan Goldberg"})json"));
  {
    comicsdb::Comic comic;
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

void publishResources(restbed::Service &service, std::vector<comicsdb::Comic> &db)
{
    auto comicResource = std::make_shared<restbed::Resource>();
    comicResource->set_path("/comic/{id: [[:digit:]]+}");
    comicResource->set_method_handler(
        "GET",
        [&db](const std::shared_ptr<restbed::Session> &session)
        {
          const auto &request = session->get_request();
          if (request->has_path_parameter("id"))
          {
            const std::size_t id = request->get_path_parameter("id", 0);
            if (id < db.size())
            {
              const std::string json = toJson(db[id]);
              session->close(restbed::OK, json,
                             {{"Content-Length", std::to_string(json.size())}});
            }
            else
            {
              const std::string msg{"Not Acceptable, id out of range"};
              session->close(restbed::NOT_ACCEPTABLE, msg,
                             {{"Content-Length", std::to_string(msg.size())},
                              {"Content-Type", "text/plain"}});
            }
          }
          else
          {
              const std::string msg{"Not Acceptable, missing id"};
              session->close(restbed::NOT_ACCEPTABLE, msg,
                             {{"Content-Length", std::to_string(msg.size())},
                              {"Content-Type", "text/plain"}});
          }
        });

    service.publish(comicResource);
}

int main()
{
  try
  {
    std::vector<comicsdb::Comic> db = load();

    restbed::Service service;
    publishResources(service, db);
    service.start(getSettings());
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
