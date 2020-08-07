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


    // *** GETTER *** //

    /**
    * @brief return metadata as json
    */
    nlohmann::json getMetadata();


    // *** SETTER *** //

    /*
    * @brief Set metadata, f.e. when json is updated from zotero
    * @param[in] jMetadata (New metadata)
    */
    void setMetadata(nlohmann::json jMetadata);


    
    //************ RETRIEVE DATA FROM JSON *******************//

    
    // *** GENERAL *** //

    /**
    * @brief return data from json.
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


    // --- collection --- //

    /**
    * @return vector with all collections this book is in
    */
    std::vector<std::string> getCollections();


    // --- author --- //

    /**
    * @return "lastName", or (if not exists) "name" of author
    */
    std::string getAuthor();

    /**
    * @return vector of strings in the form: "[lastName/ name], [firstName]"
    */
    std::vector<std::string> getAuthors();

    /*
    * return a vector containing:
    * {"lastname": "[lastName/ name]"
    * {"fullname": "[lastName/ name], [firstName]"
    * {"key": "[firstName]-[lastName/ name]" (Lower-case. Replacements: " "->"-", "/"->",")
    */
    std::vector<std::map<std::string, std::string>> getAuthorsKeys();

    /**
    * Get whether author needs to be shown according to book creator Type.
    * @param[in] creatorType
    * @return whether to show author in catalogue
    */
    bool isAuthorEditor(std::string creatorType);


        
    // --- title --- //

    /**
    * @return title of book
    */
    std::string getTitle();

    /**
    * @return "shortTitle" of book (return "title" if not exists)
    */
    std::string getShortTitle();


    // --- date --- //

    /**
    * @return date or -1 if date does not exists or is corrupted
    */
    int getDate();


    // --- bibliography and citation --- //

    /**
    * Custom short-citation (author, title[first 10 words], date)
    * @return string with "[lastName/ name], [title](first 10 words), [date]"
    */
    std::string getShow2(bool html=true);
    
    /**
    * Self created version of bibliography with escaped html.
    * @return Returns an escaped version of the bibliography
    */
    std::string getBibliographyEscaped();


    // --- others --- //

    /**
    * @param[in] sTag (tag to search for)
    * @return return whether book has a given tag
    */
    bool hasTag(std::string sTag);
};
