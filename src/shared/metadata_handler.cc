#include "metadata_handler.h"

///Empty constructor, not initializing metadata.
MetadataHandler::MetadataHandler() {}

/**
* @brief Constructor initializing metadata.
* @param[in] metadata json with metadata
*/
MetadataHandler::MetadataHandler(nlohmann::json metadata) {
  metadata_ = metadata;
}

// *** GETTER *** //

nlohmann::json MetadataHandler::GetJson() {
  return metadata_;
}


// *** SETTER *** //

void MetadataHandler::set_json(nlohmann::json metadata) {
  metadata_ = metadata;
}
    


//************ RETRIEVE DATA FROM JSON *******************//

// *** GENERAL *** //

/**
* @brief return data from json.
* @param[in] search (which metadata (f.e. title, date...)
* @return string 
*/
std::string MetadataHandler::GetMetadata(std::string search) {
  std::string returnSearch = metadata_.value(search, "");
  return returnSearch;
} 

/**
* @brief return data from metadata.
* @param[in] search (which metadata (f.e. title, date...)
* @param[in] from (from which json (f.e. title -> data -> title) 
* @return string 
*/
std::string MetadataHandler::GetMetadata(std::string search, std::string from) {
  if (metadata_.count(from) == 0)
    return "";
  return metadata_[from].value(search, ""); 
}

/**
* @brief return data from metadata
* @param[in] search (which metadata (f.e. title, date...)
* @param[in] from1 (from which json (f.e. title -> data -> title) 
* @param[in] from2 (from which json (f.e. author -> data -> creators -> author)
* @return string 
*/
std::string MetadataHandler::GetMetadata(std::string search, std::string from1, 
                                         std::string from2) {
  if (metadata_.count(from1) == 0)
    return "";
  if (metadata_[from1].count(from2) == 0)
    return "";
  return metadata_[from1][from2].value(search, "");
} 

/**
* @brief return data from metadata
* @param[in] search (which metadata (f.e. title, date...)
* @param[in] from1 (from which json (f.e. title -> data -> title) 
* @param[in] from2 (from which json (f.e. author -> data -> creators -> author)
* @param[in] in (in case of list: which element from list)
* @return string 
*/
std::string MetadataHandler::GetMetadata(std::string search, std::string from1,
                                         std::string from2, int in) {
  if (metadata_.count(from1) == 0)
    return "";
  if (metadata_[from1].count(from2) == 0)
    return "";
  if (metadata_[from1][from2].size() == 0)
    return "";
  return metadata_[from1][from2][in].value(search, "");
}


// *** SPECIFIC *** // 


// --- collection --- //

/**
* @return vector with all collections this book is in
*/
std::vector<std::string> MetadataHandler::GetCollections()
{
  if (metadata_.count("data") == 0) 
    return {};
  return metadata_["data"].value("collections", std::vector<std::string>());
}


// --- author --- //

/**
* @return "lastName", or (if not exists) "name" of author
*/
std::string MetadataHandler::GetAuthor()
{
  //Get author. Try to find "lastName"
  std::string author = GetMetadata("lastName", "data", "creators", 0);

  //If string is empty, try to find "name"
  if (author.size() == 0)
    author = GetMetadata("name", "data", "creators", 0);

  //Return result: either Name of author or empty string
  return author;
}

/**
* @return vector of strings in the form: "[lastName/ name], [firstName]"
*/
std::vector<std::string> MetadataHandler::GetAuthors()
{
  //Check is list if authors exists, if not: return empty vector
  if (metadata_.count("data") == 0 || metadata_["data"].count("creators") == 0)
    return std::vector<std::string>();

  std::vector<std::string> v_authors;
  //Iterate and create "lastname, firstname" string to push
  for (const auto &it : metadata_["data"]["creators"]) {
    std::string author = it.value("lastName", it.value("name", ""));
    if (it.value("firstName", "") != "")
      author += ", " + it.value("firstName", "");
    v_authors.push_back(author);
  }
  return v_authors;
}

/*
* Return a vector of specific author attributes.
* Each entry contains:
* {"lastname": "[lastName/ name]"
* {"fullname": "[lastName/ name], [firstName]"
* {"key": "[firstName]-[lastName/ name]" 
* -> lower-case and replacements made: " " -> "-", "/" -> ",")
* @return vector of maps
*/
std::vector<std::map<std::string, std::string>> MetadataHandler::GetAuthorsKeys() {
  //Check is list if authors exists, if not: return empty vector
  if (metadata_.count("data") == 0 || metadata_["data"].count("creators") == 0)
    return std::vector<std::map<std::string, std::string>>();

  std::vector<std::map<std::string, std::string>> v_authors;
  //Iterate and push map including "lastname", "fullname" and "key"
  for (const auto &it : metadata_["data"]["creators"]) {
    std::map<std::string, std::string> author;
    
    //Generate last name [0]
    std::string lastName = it.value("lastName", it.value("name", ""));
    author["lastname"] = lastName;

    //Generate  lastName, firstname [1]
    author["fullname"] = lastName;
    std::string firstname = it.value("firstName", "");
    if (firstname != "")
      author["fullname"] += ", " + firstname;

    //Generate key firstname-lastName
    std::string key = func::returnToLower(firstname) 
                      + "-"
                      + func::returnToLower(lastName);
    std::replace(key.begin(), key.end(), ' ', '-');
    std::replace(key.begin(), key.end(), '/', ',');
    author["key"] = key;

    author["creator_type"] = it.value("creatorType", "undefined");

    //Add to results
    v_authors.push_back(author);
  }

  return v_authors;
}

/**
* Get whether author needs to be shown according to book creator Type.
* @param[in] creator_type
* @return whether to show author in catalogue
*/
bool MetadataHandler::IsAuthorEditor(std::string creator_type) {
  if (GetMetadata("itemType", "data") == "bookSection")
    return creator_type == "author";
  return creator_type == "author" || creator_type == "editor";
}


// --- title --- //

/**
* @return title of book
*/
std::string MetadataHandler::GetTitle() {
  //Get title
  std::string title = GetMetadata("title", "data");

  //Escape html and return 
  func::escapeHTML(title);
  return title;
}

/**
* @return "shortTitle" of book (return "title" if not exists)
*/
std::string MetadataHandler::GetShortTitle() {
  std::string short_title = GetMetadata("shortTitle", "data");
  if (short_title == "")
    return GetTitle();
  return short_title;
}


// --- date --- //

/**
* @return date or -1 if date does not exists or is corrupted
*/
int MetadataHandler::GetDate() {
  //Create regex
  std::regex date3(".*(\\d{3}).*");
  std::regex date4(".*(\\d{4}).*");
  std::smatch m;

  //Get date from json
  std::string date = GetMetadata("date", "data");

  //Check whether regex find match with a 4-digit. 
  if (std::regex_search(date, m, date4))
    return std::stoi(m[1]);
  //Check whether regex find match with a 3-digit. 
  if (std::regex_search(date, m, date3))
    return std::stoi(m[1]);

  //Return -1 if regex nether matched
  return -1;
}


// --- bibliography and citation --- //

/**
* Custom short-citation (author, title[first 10 words], date)
* @return string with "[lastName/ name], [title](first 10 words), [date]"
*/
std::string MetadataHandler::GetShow2(bool html) {
  // *** Add Author *** //
  std::string result = GetAuthor();
  if (result == "") 
    result = "Unknown author";

  // *** Add title *** //
  if (GetTitle() != "" && html == true)
    result += ", <i>";
  else if (GetTitle() != "")
    result += ", ";
      
  //Add first [num] words of title
  std::vector<std::string> words_in_title = func::split2(GetTitle(), " "); 
  for (unsigned int i=0; i<10 && i<words_in_title.size(); i++)
    result += words_in_title[i] + " ";

  //Do some formatting
  result.pop_back();
  if (html == true)
    result+="</i>";
  if (words_in_title.size() > 10)
    result += "...";

  // *** Add date *** //
  if (GetDate() != -1)
    result += ", " + std::to_string(GetDate());
  return result + ".";
}


/**
* Self created version of bibliography with escaped html.
* @return Returns an escaped version of the bibliography
*/
std::string MetadataHandler::GetBibliographyEscaped() {
  std::string new_bib;

  auto authors = GetAuthors();
  if (!authors.empty()) {
	  for(auto it : GetAuthors()) {
	    new_bib+=it;
	    new_bib+="/";
	  }
	  new_bib.pop_back();
	  new_bib+=": ";
  }
    
  std::string tmp;
  if ((tmp=GetTitle())!="") {
    new_bib+=tmp;
    new_bib+=".";
  }

  if (GetMetadata("itemType")=="bookSection") {
    if((tmp=GetMetadata("itemType","data"))!="") {
      new_bib+=" ";
      new_bib+=tmp;
    }
  }
  else {
	  if ((tmp=GetMetadata("publicationTitle","data"))!="") {
	    new_bib+=" ";
	    new_bib+=tmp;
	  }
  }

  if ((tmp=GetMetadata("volume","data"))!="") {
    new_bib+=" ";
    new_bib+=tmp;
    new_bib+=".";
  }
    
  if ((tmp=GetMetadata("place","data"))!="") {
    new_bib+=" ";
    new_bib+=tmp;
  }

  if ((tmp=GetMetadata("date","data"))!="") {
    new_bib+=" ";
    new_bib+=tmp;
    new_bib+=".";
  }

  if ((tmp=GetMetadata("pages","data"))!="") {
    new_bib+=" S. ";
    new_bib+=tmp;
    new_bib+=".";
  }
  return new_bib;
}

// --- others --- //

/**
* @param[in] sTag tag to search for
* @return return whether book has a certain tag
*/
bool MetadataHandler::HasTag(std::string tag) 
{
  if ((metadata_.count("data") == 0) || metadata_["data"].count("tags") == 0)
    return false;

  for(auto it : metadata_["data"]["tags"].get<std::vector<nlohmann::json>>()) {
    if (it["tag"] == tag)
      return true;
  }
  return false;
}

///Return whether book is publicly accessible 
bool MetadataHandler::GetPublic() {
  std::time_t ttime = time(0);
  tm *local_time = localtime(&ttime);

  //Check if rights are set in metadata.
  if (GetMetadata("rights", "data") == "CLASfrei")
    return true;

  //Check for year books is published in. 
  if (GetDate() == -1 || GetDate() >= local_time->tm_year+1800)
    return false;
  return true;
}

///Return whether the basic metadata is set
bool MetadataHandler::CheckJsonSet() {
  if (GetAuthor() == "" && GetTitle() == "" && GetDate() == -1)
    return false;
  return true;
}
