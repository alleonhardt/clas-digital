#include "database.h"
#include "func.hpp"
#include <cstddef>
#include <cstring>
#include <vector>

Database::Database(std::string path_to_database) {
  // Open database
  int rc = sqlite3_open(path_to_database.c_str(), &db_);

  if (rc) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
    throw "Failed to start database"; 
  }
  else {
    std::cout << "Successfully opened database." << std::endl;
  }

  // Create table for documents and pages.
  Query("CREATE TABLE IF NOT EXISTS DOCUMENTS (ID TEXT PRIMARY KEY NOT NULL, INDEXMAP TEXT);");

  // Create table for pages.
  Query("CREATE TABLE IF NOT EXISTS PAGES (" \
         "DOCUMENT_KEY TEXT NOT NULL, PAGE_NUM INTEGER NOT NULL, PAGE TEXT," \
         "PRIMARY KEY (DOCUMENT_KEY, PAGE_NUM));");
}

Database::~Database() {
  sqlite3_close(db_);
  std::cout << "Database closed successfully." << std::endl;
}

bool Database::ClearDatabase() {
  std::cout << "Database::ClearDatabase" << std::endl;
  // Remove all documents.
  if (!Query("DELETE FROM DOCUMENTS;"))
    return false;

  // Remove all pages.
  if (!Query("DELETE FROM PAGES;"))
    return false;

  return true;
}

void Database::AddDocument(std::string document_key) {
  // Add new page to queued pages.
  queued_documents_.push_back(document_key);
}

void Database::AddPage(std::pair<std::string, int> id, std::string page, size_t max_queue_size) {
  // Add new page to queued pages.
  queued_pages_[id] = page;
  
  // If more than 10000 pages are queued, insert as batch insert.
  if (queued_pages_.size() >= max_queue_size) {
    AddPages(); // queued_pages_ will be cleared here, when all successfull.
  }
}

bool Database::AddIndex(std::string document_key, std::string index_map) {
  // Create sql statement.
  return Query("update DOCUMENTS set INDEXMAP=? where ID=?;", {index_map, document_key});
}

bool Database::AddDocuments() {
  sqlite3_mutex_enter(sqlite3_db_mutex(db_));
  char* error_essage;
  sqlite3_exec(db_, "PRAGMA synchronous=OFF", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA count_changes=OFF", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA journal_mode=MEMORY", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA temp_store=MEMORY", NULL, NULL, &error_essage);

  sqlite3_exec(db_, "BEGIN TRANSACTION", NULL, NULL, &error_essage);

  const char* sql = "INSERT OR IGNORE INTO DOCUMENTS (ID,INDEXMAP) VALUES (?,'');";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare(db_, sql, std::strlen(sql), &stmt, nullptr);

  if (rc == SQLITE_OK) {
    for (const auto& document_key : queued_documents_) {
      // Bind value.
      sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);
      
      // Check whether commit succeeded.
      int retVal = sqlite3_step(stmt);
      if (retVal != SQLITE_DONE) {
        printf("Commit Failed! %d: %s\n", retVal, document_key.c_str());
      }

      sqlite3_reset(stmt);
    }

    sqlite3_exec(db_, "COMMIT TRANSACTION", NULL, NULL, &error_essage);
    sqlite3_finalize(stmt);
  }
  else {
    fprintf(stderr, "SQL error: %s\n", error_essage);
    sqlite3_free(error_essage);
  }

  sqlite3_mutex_leave(sqlite3_db_mutex(db_)); 
  std::cout << "All " << queued_documents_.size() << " documents inserted." << std::endl;
  queued_documents_.clear();
  return true;
}

bool Database::AddPages() {
  sqlite3_mutex_enter(sqlite3_db_mutex(db_));
  char* error_message;
  sqlite3_exec(db_, "PRAGMA synchronous=OFF", NULL, NULL, &error_message);
  sqlite3_exec(db_, "PRAGMA count_changes=OFF", NULL, NULL, &error_message);
  sqlite3_exec(db_, "PRAGMA journal_mode=MEMORY", NULL, NULL, &error_message);
  sqlite3_exec(db_, "PRAGMA temp_store=MEMORY", NULL, NULL, &error_message);

  sqlite3_exec(db_, "BEGIN TRANSACTION", NULL, NULL, &error_message);

  // Create SQL statement.
  const char* sql = "INSERT INTO PAGES (DOCUMENT_KEY,PAGE_NUM,PAGE) VALUES (?, ?, ?)";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare(db_, sql, std::strlen(sql), &stmt, nullptr);

  if (rc == SQLITE_OK) {
    for (const auto& it : queued_pages_) {
      // Bind value.
      sqlite3_bind_text(stmt, 1, it.first.first.c_str(), it.first.first.length(), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 2, it.first.second);
      sqlite3_bind_text(stmt, 3, it.second.c_str(), it.second.length(), SQLITE_STATIC);
      
      // Check whether commit succeeded.
      if (sqlite3_step(stmt) != SQLITE_DONE)
        printf("Commit Failed: %s, %d\n", it.first.first.c_str(), it.first.second);

      sqlite3_reset(stmt);
    }

    sqlite3_exec(db_, "COMMIT TRANSACTION", NULL, NULL, &error_message);
    sqlite3_finalize(stmt);
  }
  else {
    fprintf(stderr, "SQL error: %s\n", error_message);
    sqlite3_free(error_message);
  }

  sqlite3_mutex_leave(sqlite3_db_mutex(db_)); 
  std::cout << queued_pages_.size() << " pages inserted." << std::endl;
  queued_pages_.clear();
  return true;
}

std::string Database::GetQueuedPage(std::pair<std::string, int> id) {
  // Check in queued pages.
  if (queued_pages_.count(id) > 0)
    return queued_pages_.at(id);

  // If not found, page might already have been added to database.
  return GetPage(id.first, id.second);
}

std::string Database::GetPage(std::string document_key, int page_num) {
  // Create SQL statement.
  const char* sql = "SELECT PAGE FROM PAGES WHERE DOCUMENT_KEY = ? AND PAGE_NUM = ?;";
  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, sql, std::strlen(sql), &stmt, nullptr);

  // Bind values.
  sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, page_num);

  // Execute SQL statement.
  std::string page = "";
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    page = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
  }
  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while performing sql (GetPage): %s\n", sqlite3_errmsg(db_));
  }

  // Return page and release resources.
  sqlite3_finalize(stmt);
  return page;
}

std::string Database::GetIndexMap(std::string document_key) {
  // Create SQL statement.
  const char* sql = "SELECT INDEXMAP FROM DOCUMENTS WHERE ID = ?;";
  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, sql, std::strlen(sql), &stmt, nullptr);

  // Bind values.
  sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);

  // Execute SQL statement.
  std::string index_map = "";
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    index_map = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
  }
  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while performing sql (GetIndexMap): %s\n", sqlite3_errmsg(db_));
  }

  // Return found index map and release resources.
  sqlite3_finalize(stmt);
  return index_map;
}

bool Database::Query(const char* query, const std::vector<std::string>& parameters) {
  // Create SQL statement.
  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, query, std::strlen(query), &stmt, nullptr);

  // Bind values.
  for (size_t i=0; i<parameters.size(); i++) {
    sqlite3_bind_text(stmt, i+1, parameters[i].c_str(), parameters[i].length(), SQLITE_STATIC);
  }
  
  // Execute SQL statement.
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    std::cout << "updated row" << std::endl;
  };

  // Print error message in case of failiure.
  bool success = true;
  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while executing sql-statement (%s): %s\n", query, sqlite3_errmsg(db_));
    success = false;
  }

  // Return success and release resources.
  sqlite3_finalize(stmt);
  return success;
}
