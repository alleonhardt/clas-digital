#include "CBookManager.hpp"

/**
* @return map of all book
*/
std::map<std::string, CBook>& CBookManager::getMapOfBooks() {
    return m_mapBooks;
}


/**
* @brief load all books.
*/
bool CBookManager::initialize()
{
    //Load directory of all books 
    DIR *dir_allItems;
    struct dirent* e_allItems;
    dir_allItems = opendir("web/books");

    //Check whether files where found
    if(!dir_allItems)
    {
        alx::cout.write(alx::console::red_black, "No books found.\n");
        return false;
    }

    //Go though all books and create book
    while((e_allItems = readdir(dir_allItems)) != NULL)
    {
        if(m_mapBooks.count(e_allItems->d_name) > 0)
            addBook(e_allItems->d_name);
        else
            alx::cout.write(alx::console::red_black, (const char*)e_allItems->d_name, " found which isn't in map!\n");
    }

    //Create map of all words + and of all words in all titles
    createMapWords();
    alx::cout.write("Map Words: ", (int)m_mapWords.size(), "\n");
    createMapWordsTitle();
    alx::cout.write("Map Titles: ", (int)m_mapWordsTitle.size(), "\n");

    return true;
}

/**
* @brief parse json of all items. If item exists, change metadata of item, create new book.
* @param[in] j_items json with all items
*/
void CBookManager::updateZotero(nlohmann::json j_items)
{
    //Iterate over all items in json
    for(auto &it:j_items)
    {
        //If book already exists, simply change metadata of book
        if(m_mapBooks.count(it["key"]) > 0)
            m_mapBooks[it["key"]].getMetadata().setMetadata(it);

        //If it does not exits, create new book and add to map of all books
        else
        {
            CBook book(it);
            m_mapBooks[it["key"]] = book;
        }
    }
}

/**
* @brief add a book, or rather: add ocr to book
* @param[in] sKey key to book
*/
void CBookManager::addBook(std::string sKey) {

    if(!std::experimental::filesystem::exists("web/books/" + sKey))
        return;

    m_mapBooks[sKey].setPath("web/books/" + sKey);
    m_mapBooks[sKey].createMapWords();
}
    

/**
* @brief search function calling fitting function from search class
* @param[in] searchOPts 
* @return list of all found books
*/
std::list<CBook*>* CBookManager::search(unsigned long long id)
{
    //Create empty map of results and matches
    std::map<std::string, double>* results1 = new std::map<std::string, CBook*>;

    //Check whether search exists
    if(m_mapSearchs.count(id) == 0)
        return new std::list<CBook*>;

    //Select search
    CSearch* search = m_mapSearchs[id];

    results1 = search->search(m_mapWords, m_mapWordsTitle);
    alx::cout.write("num results: ", results1->size(), "\n");

    //Delete search
    deleteSearch(id);

    //Sort results results
    return convertToList;
}



/**
* @brief convert to list and sort list
* @param[in] mapBooks map of books that have been found to contains the searched word
* @param[in, out] matches Map of books and there match with the searched word
* @return list of searchresulst
*/
std::list<std::string>* CBookManager::convertToList(std::map<std::string, double>* mapSR)
{
    std::list<CBook*>* listBooks = new std::list<CBook*>;

    //Change identical entrys
    double counter;
    for(auto it=mapSR.begin(); it!= mapSR.end(); it++) { it->second+=counter; counter+=0.000001; }

	// Declaring the type of Predicate that accepts 2 pairs and return a bool
	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
 
	// Defining a lambda function to compare two pairs. It will compare two pairs using second field
	Comp compFunctor =
			[](std::pair<std::string, double> elem1, std::pair<std::string, double> elem2)
			{
				return elem1.second < elem2.second;
			};

	// Declaring a set that will store the pairs using above comparision logic
	std::set<std::pair<std::string, double>, Comp> sorted(matches.begin(), matches.end(), compFunctor);

    for(std::pair<std::string, double> element : sorted)
        listBooks->push_back(element.first); 

    delete mapSR;
    return listBooks;
}


/**
* @brief create map of all words (key) and books in which the word occurs (value)
*/
void CBookManager::createMapWords()
{
    //Iterate over all books. Add words to list (if the word does not already exist in map of all words)
    //or add book to list of books (if word already exists).
    for(auto it=m_mapBooks.begin(); it!=m_mapBooks.end(); it++)
    {
        //Check whether book has ocr
        if(it->second.getOcr() == false)
            continue;

        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt : it->second.getMapWordsPages())
            double relavance = 

            m_mapWords[yt.first][it->first] = static_cast<double>(&it->second->getMapRelevance[yt.first])/&it->second->getNumPages();
    }
} 

/**
* @brief create map of all words (key) and book-titles in which the word occurs (value)
*/
void CBookManager::createMapWordsTitle()
{
    //Iterate over all books. Add words to list (if the word does not already exist in map of all words)
    //or add book to list of books (if word already exists).
    for(auto it=m_mapBooks.begin(); it!=m_mapBooks.end(); it++)
    {
        //Get map of words of current book)
        std::map<std::string, int> mapWords; 
        func::extractWordsFromString(it->second.getTitle(), mapWords);

        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
            m_mapWordsTitle[yt->first][it->first] = 0;
    }
}

/**
* @brief add a new search
*/
void CBookManager::addSearch(CSearch* search) {
    std::unique_lock lck(m_searchLock);
    m_mapSearchs[search->getID()] = search;
}


/**
* @brief delete given search and erase from map
*/
void CBookManager::deleteSearch(unsigned long long id) 
{
    std::unique_lock lck(m_searchLock);

    //Iterate over all 
    for(auto it=m_mapSearchs.begin(); it!=m_mapSearchs.end(); it++)
    {
        //If matches, then delete searchOptions, search, and erase object
        if(it->first == id) {
            it->second->deleteSearchOptions();
            delete it->second;
            m_mapSearchs.erase(it);
            break;
        }
    }
}

