#ifndef SRC_SEARCH_BOOKMANAGER_DATABASE_H_
#define SRC_SEARCH_BOOKMANAGER_DATABASE_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string>
#include <list>
#include <map>

class Database {
  public:
    Database(std::string path_to_database);

    ~Database();

    bool ClearDatabase();

    void AddDocument(std::string document_key);
    void AddPage(std::pair<std::string, int> id, std::string page);

    bool AddIndex(std::string document_key, std::string index_map);
    bool AddDocuments();
    bool AddPages();

    std::string GetPage(std::string document_key, int page_num);
    std::string GetQueuedPage(std::pair<std::string, int> id);
    std::string GetIndexMap(std::string document_key);

  private:
    sqlite3 *db_;

    std::list<std::string> queued_documents_;
    std::map<std::pair<std::string, int>, std::string> queued_pages_;

    static int callback(void *data, int argc, char **argv, char **azColName);
};

#endif
