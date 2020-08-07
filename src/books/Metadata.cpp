#include "CMetadata.hpp"

///Empty constructor, not initializing metadata.
CMetadata::CMetadata () {}

/**
* @brief Constructor initializing metadata.
* @param[in] jMetadata json with metadata
*/
CMetadata::CMetadata(nlohmann::json jMetadata) {
    m_metadata = jMetadata;
}

// *** GETTER *** //

/**
* @brief return metadata as json
*/
nlohmann::json CMetadata::getMetadata() {
    return m_metadata;
}


// *** SETTER *** //

/*
* @brief Set metadata, f.e. when json is updated from zotero
* @param[in] jMetadata (New metadata)
*/

void CMetadata::setMetadata(nlohmann::json jMetadata) {
    m_metadata = jMetadata;
}
    


//************ RETRIEVE DATA FROM JSON *******************//

// *** GENERAL *** //

/**
* @brief return data from json.
* @param[in] sSearch (which metadata (f.e. title, date...)
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch)
{
    std::string returnSearch = m_metadata.value(sSearch, "");
    return returnSearch;
} 

/**
* @brief return data from metadata.
* @param[in] sSearch (which metadata (f.e. title, date...)
* @param[in] sFrom (from which json (f.e. title -> data -> title) 
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch, std::string sFrom)
{
    if(m_metadata.count(sFrom) == 0)
        return "";
    return m_metadata[sFrom].value(sSearch, ""); 
}

/**
* @brief return data from metadata
* @param[in] sSearch (which metadata (f.e. title, date...)
* @param[in] sFrom1 (from which json (f.e. title -> data -> title) 
* @param[in] sFrom2 (in json from which json (f.e. author -> data creators -> author)
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2)
{
    if(m_metadata.count(sFrom1) == 0)
        return "";
    if(m_metadata[sFrom1].count(sFrom2) == 0)
        return "";
    return m_metadata[sFrom1][sFrom2].value(sSearch, "");
} 

/**
* @brief return data from metadata
* @param[in] sSearch (which metadata (f.e. title, date...)
* @param[in] sFrom1 (from which json (f.e. title -> data -> title) 
* @param[in] sFrom2 (in json from which json (f.e. author -> data creators -> author)
* @param[in] in (in case of list: which element from list)
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2,int in)
{
    if(m_metadata.count(sFrom1) == 0)
        return "";
    if(m_metadata[sFrom1].count(sFrom2) == 0)
        return "";
    if(m_metadata[sFrom1][sFrom2].size() == 0)
        return "";
    return m_metadata[sFrom1][sFrom2][in].value(sSearch, "");
}


// *** SPECIFIC *** // 


// --- collection --- //

/**
* @return vector with all collections this book is in
*/
std::vector<std::string> CMetadata::getCollections()
{
    //Create empty vector as default
    std::vector<std::string> vec;

    if(m_metadata.count("data") == 0)
        return vec;
    return m_metadata["data"].value("collections", vec);
}


// --- author --- //

/**
* @return "lastName", or (if not exists) "name" of author
*/
std::string CMetadata::getAuthor()
{
    //Get author. Try to find "lastName"
    std::string sAuthor = getMetadata("lastName", "data", "creators", 0);

    //If string is empty, try to find "name"
    if(sAuthor.size() == 0)
        sAuthor = getMetadata("name", "data", "creators", 0);

    //Return result: either Name of author or empty string
    return sAuthor;
}

/**
* @return vector of strings in the form: "[lastName/ name], [firstName]"
*/
std::vector<std::string> CMetadata::getAuthors()
{
    //Check is list if authors exists, if not: return empty vector
    if(m_metadata.count("data") == 0 || m_metadata["data"].count("creators") == 0)
        return std::vector<std::string>();

    std::vector<std::string> v_sAuthors;
    //Iterate and create "lastname, firstname" string to push
    for(const auto &it : m_metadata["data"]["creators"])
    {
        std::string sAuthor = it.value("lastName", it.value("name", ""));
        if(it.value("firstName", "") != "")
            sAuthor += ", " + it.value("firstName", "");
        v_sAuthors.push_back(sAuthor);
    }
    return v_sAuthors;
}

/*
* return a vector containing:
* {"lastname": "[lastName/ name]"
* {"fullname": "[lastName/ name], [firstName]"
* {"key": "[firstName]-[lastName/ name]" (Lower-case. Replacements: " "->"-", "/"->",")
*/
std::vector<std::map<std::string, std::string>> CMetadata::getAuthorsKeys()
{
    //Check is list if authors exists, if not: return empty vector
    if(m_metadata.count("data") == 0 || m_metadata["data"].count("creators") == 0)
        return std::vector<std::map<std::string, std::string>>();

    std::vector<std::map<std::string, std::string>> v_sAuthors;
    //Iterate and push map including "lastname", "fullname" and "key"
    for(const auto &it : m_metadata["data"]["creators"])
    {
        std::map<std::string, std::string> author;
        
        //Generate last name [0]
        std::string lastName = it.value("lastName", it.value("name", ""));
        author["lastname"] = lastName;

        //Generate  lastName, firstname [1]
        author["fullname"] = lastName;
        std::string firstname = it.value("firstName", "");
        if(firstname != "")
            author["fullname"] += ", " + firstname;

        //Generate key firstname-lastName
        std::string key = func::returnToLower(firstname)+"-"+func::returnToLower(lastName);
        std::replace(key.begin(), key.end(), ' ', '-');
        std::replace(key.begin(), key.end(), '/', ',');
        author["key"] = key;

        author["creatorType"] = it.value("creatorType", "undefined");

        //Add to results
        v_sAuthors.push_back(author);
    }

    return v_sAuthors;
}

/**
* Get whether author needs to be shown according to book creator Type.
* @param[in] creatorType
* @return whether to show author in catalogue
*/
bool CMetadata::isAuthorEditor(std::string creatorType)
{
    if(getMetadata("itemType", "data") == "bookSection")
        return creatorType == "author";
    else
        return creatorType == "author" || creatorType == "editor";
}


// --- title --- //

/**
* @return title of book
*/
std::string CMetadata::getTitle() {

    //Get title
    std::string title = getMetadata("title", "data");

    //Escape html
    func::escapeHTML(title);
    return title;
}

/**
* @return "shortTitle" of book (return "title" if not exists)
*/
std::string CMetadata::getShortTitle() {
    std::string sReturn = getMetadata("shortTitle", "data");
    if(sReturn == "")
        return getTitle();
    else
        return sReturn;
}


// --- date --- //

/**
* @return date or -1 if date does not exists or is corrupted
*/
int CMetadata::getDate()
{
    //Create regex
    std::regex date3(".*(\\d{3}).*");
    std::regex date4(".*(\\d{4}).*");
    std::smatch m;

    //Get date from json
    std::string sDate = getMetadata("date", "data");

    //Check whether regex find match with a 4-digit. 
    if(std::regex_search(sDate, m, date4))
        return std::stoi(m[1]);
    //Check whether regex find match with a 3-digit. 
    if(std::regex_search(sDate, m, date3))
        return std::stoi(m[1]);

    //Return -1 if regex nether matched
    return -1;
}


// --- bibliography and citation --- //

/**
* Custom short-citation (author, title[first 10 words], date)
* @return string with "[lastName/ name], [title](first 10 words), [date]"
*/
std::string CMetadata::getShow2(bool html)
{
    // *** Add Author *** //
    std::string sResult = getAuthor();
    if(sResult == "") 
        sResult = "Unknown author";

    // *** Add title *** //
    if(getTitle() != "" && html == true)
        sResult += ", <i>";
    else if(getTitle() != "")
        sResult += ", ";
        
    //Add first [num] words of title
    std::vector<std::string> vStrs;
    func::split(getTitle(), " ", vStrs);
    for(unsigned int i=0; i<10 && i<vStrs.size(); i++)
        sResult += vStrs[i] + " ";

    //Do some formatting
    sResult.pop_back();
    if(html == true)
        sResult+="</i>";
    if(vStrs.size() > 10)
        sResult += "...";

    // *** Add date *** //
    if(getDate() != -1)
        sResult += ", " + std::to_string(getDate());
    return sResult + ".";
}


/**
* Self created version of bibliography with escaped html.
* @return Returns an escaped version of the bibliography
*/
std::string CMetadata::getBibliographyEscaped()
{
    std::string new_bib;
    auto authors = getAuthors();
    if(!authors.empty())
    {
	for(auto it : getAuthors())
	{
	    new_bib+=it;
	    new_bib+="/";
	}
	new_bib.pop_back();
	new_bib+=": ";
    }
    
    std::string tmp;
    if((tmp=getTitle())!="")
    {
	new_bib+=tmp;
	new_bib+=".";
    }

    if(getMetadata("itemType")=="bookSection")
    {
	if((tmp=getMetadata("itemType","data"))!="")
	{
	    new_bib+=" ";
	    new_bib+=tmp;
	}
    }
    else
    {
	if((tmp=getMetadata("publicationTitle","data"))!="")
	{
	    new_bib+=" ";
	    new_bib+=tmp;
	}
    }

    if((tmp=getMetadata("volume","data"))!="")
    {
	new_bib+=" ";
	new_bib+=tmp;
	new_bib+=".";
    }
    
    if((tmp=getMetadata("place","data"))!="")
    {
	new_bib+=" ";
	new_bib+=tmp;
    }

    if((tmp=getMetadata("date","data"))!="")
    {
	new_bib+=" ";
	new_bib+=tmp;
	new_bib+=".";
    }

    
    if((tmp=getMetadata("pages","data"))!="")
    {
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
bool CMetadata::hasTag(std::string sTag) 
{
    if(m_metadata.count("data") == 0)
        return false;
    if(m_metadata["data"].count("tags") == 0)
        return false;
    for(auto it = m_metadata["data"]["tags"].begin(); it!=m_metadata["data"]["tags"].end(); it++)
    {
        if(it.value()["tag"] == sTag)
            return true;
    }
    return false;
}
