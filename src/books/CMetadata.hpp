#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <mutex>
#include <map>
#include "json.hpp"
#include "func.hpp"

class CMetadata
{
private:
    nlohmann::json m_metadata;  ///< Json with book metadata

public:
    
    ///Empty constructor, not initializing metadata.
    CMetadata();

    /**
    * @brief Constructor initializing metadata.
    * @param[in] jMetadata json with metadata
    */
    CMetadata(nlohmann::json jMetadata);
 
    /*
    * @brief Set metadata, f.e. when json is updated from zotero
    * @param[in] jMetadata (New metadata)
    */
    void setMetadata(nlohmann::json jMetadata);


    //************ get metadata *******************//

    // *** GETTER *** //

    /**
    * @brief return metadata as json
    */
    nlohmann::json getMetadata();

    
    // *** GENERAL *** //

    /**
    * @brief return data from metadata.
    * @param[in] sSearch (which metadata (f.e. title, date...)
    * @return string 
    */
    std::string getMetadata(std::string sSearch);
    
    /**
    * @brief return data from metadata.
    * @param[in] sSearch (which metadata (f.e. title, date...)
    * @param[in] sFrom (from which json (f.e. title -> data -> title) 
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom);

    /**
    * @brief return data from metadata
    * @param[in] sSearch (which metadata (f.e. title, date...)
    * @param[in] sFrom1 (from which json (f.e. title -> data -> title) 
    * @param[in] sFrom2 (in json from which json (f.e. author -> data creators -> author)
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2);

    /**
    * @brief return data from metadata
    * @param[in] sSearch (which metadata (f.e. title, date...)
    * @param[in] sFrom1 (from which json (f.e. title -> data -> title) 
    * @param[in] sFrom2 (in json from which json (f.e. author -> data creators -> author)
    * @param[in] in (in case of list: which element from list)
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2, int in);


    // *** SPECIFIC *** // 

    /**
    * @return vector with all collections this book is in
    */
    std::vector<std::string> getCollections();

    /**
    * @return "lastName", or (if not exists) "name" of author
    */
    std::string getAuthor();

    /**
    * @return vector of strings in the form: "[lastName/ name], [firstName]"
    */
    std::vector<std::string> getAuthors();

    /**
    * @param[in] sTag (tag to search for)
    * @return return whether book has a given tag
    */
    bool hasTag(std::string sTag);

    /**
    * @return title of book
    */
    std::string getTitle();

    /**
    * @return "shortTitle" of book (return "title" if not exists)
    */
    std::string getShortTitle();

    /**
    * @return date or -1 if date does not exists or is corrupted
    */
    int getDate();

    /**
    * @return string with "[lastName/ name], [date]"
    */
    std::string getShow();

    std::string getZit(size_t page);

    /**
    * @return string with Auhtor + first 10 words title + date
    */
    std::string getShow2();

};
