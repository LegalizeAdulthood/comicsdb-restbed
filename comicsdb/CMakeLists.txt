add_executable(comicsdb comicsdb.cpp
  comic.h
  comic.cpp
)
target_link_libraries(comicsdb PRIVATE restbed::restbed rapidjson)
