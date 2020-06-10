#include "CMetadata.hpp"

CMetadata::CMetadata () {}


/**
* @param[in] jMetadata json with metadata
*/
CMetadata::CMetadata(nlohmann::json jMetadata) {
    m_metadata = jMetadata;
}


/*
* @param[in] jMetadata new metadata
*/
void CMetadata::setMetadata(nlohmann::json jMetadata) {
    m_metadata = jMetadata;
}
    


//************ get metadata *******************//

nlohmann::json CMetadata::getMetadata() {
    return m_metadata;
}

/**
* getter function to return selected metadata
* @parameter string (sSearch: which metadata (f.e. title, date...)
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch)
{
    std::string returnSearch = m_metadata.value(sSearch, "");
    return returnSearch;
} 
 
/**
* getter function to return selected metadata
* @parameter string (sSearch: which metadata (f.e. title, date...)
* @parameter string (sFrom: from which json (f.e. title -> data -> title) 
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch, std::string sFrom)
{
    if(m_metadata.count(sFrom) == 0)
        return "";
    return m_metadata[sFrom].value(sSearch, ""); 
}

/**
* getter function to return selected metadata
* @parameter string (sSearch: which metadata (f.e. title, date...)
* @parameter string (sFrom2: from which json (f.e. title -> data -> title) 
* @parameter string (sFrom2: in json from which json (f.e. author -> data creators -> author)
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
* getter function to return selected metadata
* @parameter string (sSearch: which metadata (f.e. title, date...)
* @parameter string (sFrom2: from which json (f.e. title -> data -> title) 
* @parameter string (sFrom2: in json from which json (f.e. author -> data creators -> author)
* @parameter int (index: in case of list: which element from list)
* @return string 
*/
std::string CMetadata::getMetadata(std::string sSearch, std::string sFrom1, std::string sFrom2, int in)
{
    if(m_metadata.count(sFrom1) == 0)
        return "";
    if(m_metadata[sFrom1].count(sFrom2) == 0)
        return "";
    if(m_metadata[sFrom1][sFrom2].size() == 0)
        return "";
    return m_metadata[sFrom1][sFrom2][in].value(sSearch, "");
}


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

/**
* @return lastName, or Name of author
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

std::vector<std::string> CMetadata::getAuthors()
{
    std::vector<std::string> v_sAuthors;
    if(m_metadata.count("data") == 0)
        return v_sAuthors;
    if(m_metadata["data"].count("creators") == 0)
        return v_sAuthors;

    std::vector<nlohmann::json> v_jAuthors = m_metadata["data"]["creators"];
    for(const auto &it : v_jAuthors)
    {
        std::string sAuthor = it.value("lastName", it.value("name", ""));
        if(it.value("firstName", "") != "")
            sAuthor += ", " + it.value("firstName", "");
        v_sAuthors.push_back(sAuthor);
    }

    return v_sAuthors;
}

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

/**
* @return title of book
*/
std::string CMetadata::getTitle() {
    return getMetadata("title", "data");
}

/**
* @return short-title of book (Title if not exists)
*/
std::string CMetadata::getShortTitle() {
    std::string sReturn = getMetadata("shortTitle", "data");
    if(sReturn == "")
        return getTitle();
    else
        return sReturn;
}

/**
* @return date or -1 if date does not exists or is currupted
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

/**
* @return string with Auhtor + first 6 words 15 words of title + date
*/
std::string CMetadata::getShow()
{
    // *** Add Author *** //
    std::string sResult = getAuthor();

    if(sResult == "") sResult = "No Author";

    if(getDate() != -1) 
        sResult += ", " + std::to_string(getDate());
    sResult += ".";

    return sResult;
}

std::string CMetadata::getZit(size_t page)
{
    std::string sZit = getShow();
    sZit.pop_back();
    sZit.insert(0, "(");
    if(page != 1000000)
        sZit.append(", S. " + std::to_string(page) + ".)");
    else
        sZit.append(".)");

    return sZit;
}

/**
* @return string with Auhtor + first 6 words 15 words of title + date
*/
std::string CMetadata::getShow2()
{
    // *** Add Author *** //
    std::string sResult = getAuthor();
    if(sResult == "") sResult = "Unknown author";

    if(getTitle() != "")
        sResult += ", <i>";

    // *** Add first [num] words of title *** //
    std::vector<std::string> vStrs;
    func::split(getTitle(), " ", vStrs);
    for(unsigned int i=0; i<10 && i<vStrs.size(); i++)
        sResult += vStrs[i] + " ";

    sResult.pop_back();
    if(vStrs.size() > 10 && getDate() != -1)
        sResult += "...</i>, " + std::to_string(getDate()) + ".";
    else if(vStrs.size() > 10)
        sResult += "...</i>.";
    else if(getDate() != -1)
        sResult += "</i>, " + std::to_string(getDate()) + ".";
    else
        sResult += "</i>.";

    return sResult;
}

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

