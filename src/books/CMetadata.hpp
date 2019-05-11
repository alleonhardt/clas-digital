#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <map>
#include "json.hpp"

class CMetadata
{
private:
    nlohmann::json m_metadata;

public:
    
    /**
    *@param[in] sMetadata path to metadata
    */
    CMetadata(std::string sMetadata);

    /**
    *@return metadata
    */
    nlohmann::json getJson();


    //************ get metadata *******************//

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
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();
};
