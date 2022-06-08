#include "comic.h"

#include <iostream>
#include <vector>

int main() {
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
  std::cout << comicsdb::toJson(db.back()) << '\n';
  return 0;
}
