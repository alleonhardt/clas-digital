/*
* @author Jan van Dick
*/

#ifndef CLASDIGITAL_SRC_BOOKS_METADATAHANDLER_H_
#define CLASDIGITAL_SRC_BOOKS_METADATAHANDLER_H_

#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <regex>
#include <string>

#include "func.hpp"
#include "json.hpp"

class MetadataHandler
{
public:
    
  ///Empty constructor, not initializing metadata.
  MetadataHandler();

  /**
  * @brief Constructor initializing metadata.
  * @param[in] metadata json with metadata
  */
  MetadataHandler(nlohmann::json metadata);


  // *** GETTER *** //

  nlohmann::json get_json();


  // *** SETTER *** //

  void set_json(nlohmann::json metadata);


  
  //************ RETRIEVE DATA FROM JSON *******************//

  
  // *** GENERAL *** //

  /**
  * @brief return data from json.
  * @param[in] search (which metadata (f.e. title, date...)
  * @return string 
  */
  std::string GetMetadata(std::string search);
  
  /**
  * @brief return data from metadata.
  * @param[in] search (which metadata (f.e. title, date...)
  * @param[in] from (from which json (f.e. title -> data -> title) 
  * @return string 
  */
  std::string GetMetadata(std::string search, std::string from);

  /**
  * @brief return data from metadata
  * @param[in] search (which metadata (f.e. title, date...)
  * @param[in] from1 (from which json (f.e. title -> data -> title) 
  * @param[in] from2 (in json from which json (f.e. author -> data creators -> author)
  * @return string 
  */
  std::string GetMetadata(std::string search, std::string from1, std::string from2);

  /**
  * @brief return data from metadata
  * @param[in] search (which metadata (f.e. title, date...)
  * @param[in] from1 (from which json (f.e. title -> data -> title) 
  * @param[in] from2 (in json from which json (f.e. author -> data creators -> author)
  * @param[in] in (in case of list: which element from list)
  * @return string 
  */
  std::string GetMetadata(std::string search, std::string from1, std::string from2, int in);



  // *** SPECIFIC *** // 


  // --- collection --- //

  /**
  * @return vector with all collections this book is in
  */
  std::vector<std::string> GetCollections();


  // --- author --- //

  /**
  * @return "lastName", or (if not exists) "name" of author
  */
  std::string GetAuthor();

  /**
  * @return vector of strings in the form: "[lastName/ name], [firstName]"
  */
  std::vector<std::string> GetAuthors();

  /*
  * Return a vector of specific author attributes.
  * Each entry contains:
  * {"lastname": "[lastName/ name]"
  * {"fullname": "[lastName/ name], [firstName]"
  * {"key": "[firstName]-[lastName/ name]" 
  * -> lower-case and replacements made: " " -> "-", "/" -> ",")
  * @return vector of maps
  */
  std::vector<std::map<std::string, std::string>> GetAuthorsKeys();

  /**
  * Get whether author needs to be shown according to book creator Type.
  * @param[in] creatorType
  * @return whether to show author in catalogue
  */
  bool IsAuthorEditor(std::string creatorType);


      
  // --- title --- //

  /**
  * Extract title from json.
  * @return title of book
  */
  std::string GetTitle();

  /**
  * Extract short-title from json.
  * @return "shortTitle" of book (return "title" if not exists)
  */
  std::string GetShortTitle();


  // --- date --- //

  /**
  * Extract date from json.
  * @return date or -1 if date does not exists or is corrupted
  */
  int GetDate();


  // --- bibliography and citation --- //

  /**
  * Custom short-citation (author, title[first 10 words], date)
  * @return string with "[lastName/ name], [title](first 10 words), [date]"
  */
  std::string GetShow2(bool html=true);
  
  /**
  * Self created version of bibliography with escaped html.
  * @return Returns an escaped version of the bibliography
  */
  std::string GetBibliographyEscaped();


  // --- others --- //

  /**
  * @param[in] sTag (tag to search for)
  * @return return whether book has a given tag
  */
  bool HasTag(std::string sTag);

private:
  nlohmann::json metadata_;  ///< Json with book metadata

};

#endif
