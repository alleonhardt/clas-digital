#ifndef SRC_SEARCH_BOOKMANAGER_DATABASE_H_
#define SRC_SEARCH_BOOKMANAGER_DATABASE_H_

#include <cstddef>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string>
#include <list>
#include <map>
#include <vector>

class Database {
  public:
    /**
     * Opens database at given path and creates tables for documents and pages 
     * if these do not already exist.
     * @param[in] path_to_database
     */
    Database(std::string path_to_database);

    /**
     * Closes database.
     */
    ~Database();

    /**
     * Clears database. Removes all documents and pages.
     */
    bool ClearDatabase();

    /**
     * Adds one document to queued documents. To acctually write to database,
     * call `AddDocuments()` after all documents have been added to queue.
     * @param[in] document_key to add.
     */
    void AddDocument(std::string document_key);

    /** 
     * Adds one page to queued pages. When max queue size is reached, all queued
     * pages are written to database. To assure, that all pages are written,
     * call `AddPages()` once all pages have been added to queue.
     * @param[in] id of this page. The id is build from the document-key + page-number. 
     * @param[in] page to save.
     * @param[in] max_queue_size defining when queue is stored. Default=10000.
     */
    void AddPage(std::pair<std::string, int> id, std::string page, size_t max_queue_size=10000);

    /**
     * Updates the index-map of a document.
     * In contrast to `AddPage()` and `AddDocument()` the index-map is stored to
     * the database directly.
     * @param[in] document_key of desired document to update.
     * @param[in] index_map to store. 
     * @return boolean indicating success/ failiure.
     */
    bool AddIndex(std::string document_key, std::string index_map);

    /**
     * Writes all queued documents to the database.
     * If a document already exists, it will be ignored.
     * @return boolean indicating success/ failiure.
     */
    bool AddDocuments();

    /** 
     * Writes all queued pages to the database.
     * If a pages already exists, it will be overwitten.
     * @return boolean indicating success/ failiure.
     */
    bool AddPages();

    /**
     * Selects given page from given document. 
     * @param[in] document_key 
     * @param[in] page_num
     * @return found page or empty string.
     */
    std::string GetPage(std::string document_key, int page_num);

    /**
     * Selects index-map of given document.
     * @param[in] document_key
     * @return found index-map or empty string.
     */
    std::string GetIndexMap(std::string document_key);

    /**
     * Returns page with of given document, however it searches in the queued
     * pages, bevor selecting from database. This can be usefull, when the page
     * needs to be accessed, bevor all pages have been written to the database.
     * @param[in] id of this page. The id is build from the document-key + page-number. 
     * @return found page or empty string.
     */
    std::string GetQueuedPage(std::pair<std::string, int> id);

  private:
    sqlite3 *db_; ///< Pointer to database.
    std::list<std::string> queued_documents_;  ///< queued-documents for batch inserting.
    std::map<std::pair<std::string, int>, std::string> queued_pages_; ///< queued-pages for batch inserting.

    /**
     * Helper function to generalize database operations, which do not return a
     * value other than success/ failiure (creating, table, deleting and inserting rows).
     * @param[in] query
     * @param[in] parameters to be inserted into query via bind.
     * @return boolean indicating success/ failiure.
     */
    bool Query(const char* query, const std::vector<std::string>& parameters = {});
};

#endif
