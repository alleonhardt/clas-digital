#include "database.h"
#include "func.hpp"

Database::Database(std::string path_to_database) {
  char* err_msg;
  int rc;

  // Open database
  rc = sqlite3_open(path_to_database.c_str(), &db_);

  if (rc) {
    std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
    throw "Failed to start database"; 
  }
  else {
    std::cout << "Successfully opened database." << std::endl;
  }

  // Create table for documents and pages.

  // Create sql statement.
  const char* sql_documents = "CREATE TABLE IF NOT EXISTS DOCUMENTS (" \
         "ID TEXT PRIMARY KEY NOT NULL, INDEXMAP TEXT);";

  // Execute sql statement.
  rc = sqlite3_exec(db_, sql_documents, callback, 0, &err_msg);

  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  } 
  else {
    fprintf(stdout, "Table \"DOCUMENTS\" created successfully\n");
  }

      // Create sql statement.
  const char* sql_pages = "CREATE TABLE IF NOT EXISTS PAGES (" \
         "DOCUMENT_KEY TEXT NOT NULL, PAGE_NUM INTEGER NOT NULL, PAGE TEXT," \
         "PRIMARY KEY (DOCUMENT_KEY, PAGE_NUM));";

  // Execute sql statement.
  rc = sqlite3_exec(db_, sql_pages, callback, 0, &err_msg);

  if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", err_msg);
    sqlite3_free(err_msg);
  } 
  else {
    fprintf(stdout, "Table \"PAGES\" created successfully\n");
  }
}

Database::~Database() {
  sqlite3_close(db_);
  std::cout << "Database closed successfully." << std::endl;
}

bool Database::ClearDatabase() {
  char* err_msg;
  int rc;

  /* Create merged SQL statement */
  const char* sql_delete_documents = "DELETE FROM DOCUMENTS;";
  /* Execute SQL statement */
  rc = sqlite3_exec(db_, sql_delete_documents, callback, 0, &err_msg);
  if( rc != SQLITE_OK ) {
     fprintf(stderr, "SQL error: %s\n", err_msg);
     sqlite3_free(err_msg);
     return false;
  } else {
     fprintf(stdout, "Emptied table \"DOCUMENTS\"\n");
  }

  /* Create merged SQL statement */
  const char* sql_delete_pages = "DELETE FROM PAGES;";
  /* Execute SQL statement */
  rc = sqlite3_exec(db_, sql_delete_pages, callback, 0, &err_msg);
  if( rc != SQLITE_OK ) {
     fprintf(stderr, "SQL error: %s\n", err_msg);
     sqlite3_free(err_msg);
     return false;
  } else {
     fprintf(stdout, "Emptied table \"PAGES\"\n");
  }

  return true;
}

void Database::AddDocument(std::string document_key) {
  // Add new page to queued pages.
  queued_documents_.push_back(document_key);
}

void Database::AddPage(std::pair<std::string, int> id, std::string page) {
  // Add new page to queued pages.
  queued_pages_[id] =  page;
  
  // If more than 10000 pages are queued, insert as batch insert.
  if (queued_pages_.size() > 10000) {
    AddPages(); // queued_pages_ will be cleared here, when all successfull.
  }
}

bool Database::AddIndex(std::string document_key, std::string index_map) {
  char* err_msg;
  int rc;

  // Create sql statement.
  std::string sql_add_index_map = "update DOCUMENTS set INDEXMAP=? where ID=?;";

  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, sql_add_index_map.c_str(), sql_add_index_map.length(), &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, index_map.c_str(), index_map.length(), SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, document_key.c_str(), document_key.length(), SQLITE_STATIC);

  // Execute statement.
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    std::cout << "Updated row." << std::endl;
  };

  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while performing sql (AddIndex): %s\n", sqlite3_errmsg(db_));
    printf("ret_code = %d\n", ret_code);
  }

  sqlite3_finalize(stmt);
  return true;
}

bool Database::AddDocuments() {
  sqlite3_mutex_enter(sqlite3_db_mutex(db_));
  char* error_essage;
  sqlite3_exec(db_, "PRAGMA synchronous=OFF", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA count_changes=OFF", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA journal_mode=MEMORY", NULL, NULL, &error_essage);
  sqlite3_exec(db_, "PRAGMA temp_store=MEMORY", NULL, NULL, &error_essage);

  sqlite3_exec(db_, "BEGIN TRANSACTION", NULL, NULL, &error_essage);

  std::string sql = "INSERT OR IGNORE INTO DOCUMENTS (ID,INDEXMAP) VALUES (?,'');";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare(db_, sql.c_str(), sql.length(), &stmt, nullptr);

  if (rc == SQLITE_OK) {
    for (const auto& document_key : queued_documents_) {
      // bind the value
      sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);
      
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

  std::string sql = "INSERT INTO PAGES (DOCUMENT_KEY,PAGE_NUM,PAGE) VALUES (?, ?, ?)";
  sqlite3_stmt* stmt;
  int rc = sqlite3_prepare(db_, sql.c_str(), sql.length(), &stmt, nullptr);

  if (rc == SQLITE_OK) {
    for (const auto& it : queued_pages_) {
      // bind the value
      sqlite3_bind_text(stmt, 1, it.first.first.c_str(), it.first.first.length(), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 2, it.first.second);
      sqlite3_bind_text(stmt, 3, it.second.c_str(), it.second.length(), SQLITE_STATIC);
      
      int retVal = sqlite3_step(stmt);
      if (retVal != SQLITE_DONE)
        printf("Commit Failed! %d: %s, %d\n", retVal, it.first.first.c_str(), it.first.second);

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
  func::pause();
  queued_pages_.clear();
  return true;
}

std::string Database::GetQueuedPage(std::pair<std::string, int> id) {
  // Check in queued pages.
  if (queued_pages_.count(id) > 0)
    return queued_pages_.at(id);
  // If not found, page might already have been added to database, thus search
  // in database.
  return GetPage(id.first, id.second);
}

std::string Database::GetPage(std::string document_key, int page_num) {
  char* err_msg;
  int rc;

  /* Create SQL statement */
  std::string sql = "SELECT * FROM PAGES WHERE DOCUMENT_KEY = ? AND PAGE_NUM = ?;";
  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);
  sqlite3_bind_int(stmt, 2, page_num);

  // Execute SQL statement.
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    return std::string(reinterpret_cast<const char*>(
      sqlite3_column_text(stmt, 2)
    ));
  }
  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while performing sql (GetPage): %s\n", sqlite3_errmsg(db_));
    printf("ret_code = %d\n", ret_code);
  }

  std::cout << "SQL: Page not found" << std::endl;

  //release resources
  sqlite3_finalize(stmt);
  sqlite3_close(db_);
  return "";
}

std::string Database::GetIndexMap(std::string document_key) {
  char* err_msg;
  int rc;

  /* Create SQL statement */
  std::string sql = "SELECT * FROM DOCUMENTS WHERE ID = ?;";
  sqlite3_stmt * stmt;
  sqlite3_prepare_v2(db_, sql.c_str(), sql.length(), &stmt, nullptr);

  sqlite3_bind_text(stmt, 1, document_key.c_str(), document_key.length(), SQLITE_STATIC);

  // Execute SQL statement.
  int ret_code = 0;
  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    return std::string(reinterpret_cast<const char*>(
      sqlite3_column_text(stmt, 1)
    ));
  }
  if (ret_code != SQLITE_DONE) {
    printf("ERROR: while performing sql (GetIndexMap): %s\n", sqlite3_errmsg(db_));
    printf("ret_code = %d\n", ret_code);
  }

  printf("SQL: index map not found");

  //release resources
  sqlite3_finalize(stmt);
  sqlite3_close(db_);
  return "";
}

int Database::callback(void *data, int argc, char **argv, char **azColName) {
  int i;
  for(i = 0; i<argc; i++) {
     printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
     data = argv[i];
  }
  printf("\n");
  return 0;
}
