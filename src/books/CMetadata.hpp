#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <mutex>
#include <map>
#include "json.hpp"

class CMetadata
{
private:
    nlohmann::json m_metadata;

public:
    
    CMetadata();

    /**
    * @param[in] jMetadata json with metadata
    */
    CMetadata(nlohmann::json jMetadata);
 
    /*
    * @param[in] jMetadata new metadata
    */
    void setMetadata(nlohmann::json jMetadata);


    //************ get metadata *******************//

    nlohmann::json getMetadata();

    /**
    * getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @return string 
    */
    std::string getMetadata(std::string sSearch);
    
    /**
    * getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @parameter string (sFrom: from which json (f.e. title -> data -> title) 
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom);

    /**
    * getter function to return selected metadata
    * @parameter string (sSearch: which metadata (f.e. title, date...)
    * @parameter string (sFrom2: from which json (f.e. title -> data -> title) 
    * @parameter string (sFrom2: in json from which json (f.e. author -> data creators -> author)
    * @return string 
    */
    std::string getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2);

    /**
    * getter function to return selected metadata
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

    /**
    * @return title of book
    */
    std::string getTitle();

    /**
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();

    /**
    * @return string with Auhtor + first 6 words 15 words of title + date
    */
    std::string getShow();
};
