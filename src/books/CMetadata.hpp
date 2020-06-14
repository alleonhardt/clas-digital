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
    nlohmann::json m_metadata;

public:
    
    CMetadata();

    /**
    * @brief constructor
    * @param[in] jMetadata json with metadata
    */
    CMetadata(nlohmann::json jMetadata);
 
    /*
    * @param[in] jMetadata new metadata
    */
    void setMetadata(nlohmann::json jMetadata);


    //************ get metadata *******************//

    /**
    * @brief return metadata as json
    */
    nlohmann::json getMetadata();

    /**
    * @brief getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @return string 
    */
    std::string getMetadata(std::string sSearch);
    
    /**
    * @brief getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @parameter string (sFrom: from which json (f.e. title -> data -> title) 
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom);

    /**
    * @brief getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @parameter string (sFrom2: from which json (f.e. title -> data -> title) 
    * @parameter string (sFrom2: in json from which json (f.e. author -> data creators -> author)
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2);

    /**
    * @brief getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @parameter string (sFrom2: from which json (f.e. title -> data -> title) 
    * @parameter string (sFrom2: in json from which json (f.e. author -> data creators -> author)
    * @parameter int (index: in case of list: which element from list)
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2, int in);

    /**
    * @return vector with all collections this book is in
    */
    std::vector<std::string> getCollections();

    /**
    * @return lastName, or Name of author
    */
    std::string getAuthor();

    std::vector<std::string> getAuthors();

    /**
    * return a vector contaiing
    * [laszname] lastname
    * [fullname] lastname, firstname
    * [key] key (firstname-lastname)
    */
    std::vector<std::map<std::string, std::string>> getAuthorsKeys();

    /**
    * Get whether author needs to be shown acording to book key.
    * @param[in] creatorType
    */
    bool isAuthorEditor(std::string creatorType);
    
    /**
     * @return Returns an escaped version of the bibliography
     */
    std::string getBibliographyEscaped();

    /**
    * @param[in] sTag tag to search for
    * @return return whether book has a certain tag
    */
    bool hasTag(std::string sTag);

    /**
    * @return title of book
    */
    std::string getTitle();

    /**
    * @return shrottitle of book (titel if not exists)
    */
    std::string getShortTitle();

    /**
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();

    /**
    * @return string with "[Auhtor], [date]"
    */
    std::string getShow();

    std::string getZit(size_t page);

    /**
    * @return string with Auhtor + first 10 words title + date
    */
    std::string getShow2();

};
