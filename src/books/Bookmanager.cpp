#include "CBookManager.hpp"
#include "debug/debug.hpp"

/**
* @return map of all book
*/
std::unordered_map<std::string, CBook*>& CBookManager::getMapOfBooks() {
    return m_mapBooks;
}


/**
* @brief load all books.
*/
bool CBookManager::initialize()
{
    std::cout << "Starting initualizeing..." << std::endl;

    //Load directory of all books 
    DIR *dir_allItems;
    struct dirent* e_allItems;
    dir_allItems = opendir("web/books");

    std::cout << "reached this" << std::endl;

    //Check whether files where found
    if(!dir_allItems)
        return false;
    
    std::cout << "extracting books." << std::endl;
    //Go though all books and create book
    while((e_allItems = readdir(dir_allItems)) != NULL) {
        if(m_mapBooks.count(e_allItems->d_name) > 0)
            addBook(e_allItems->d_name);
    }

    std::cout << "Createing map of books." << std::endl;

    //Create map of all words + and of all words in all titles
    createMapWords();
    createMapWordsTitle();
    std::cout << "\n";
    std::cout << "Map words: " << m_mapWords.size() << "\n";
    std::cout << "Map title: " << m_mapWordsTitle.size() << "\n";

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
            m_mapBooks[it["key"]]->getMetadata().setMetadata(it);

        //If it does not exits, create new book and add to map of all books
        else
            m_mapBooks[it["key"]] = new CBook(it);
    }
}

/**
* @brief add a book, or rather: add ocr to book
* @param[in] sKey key to book
*/
void CBookManager::addBook(std::string sKey) {
    if(!std::experimental::filesystem::exists("web/books/" + sKey))
        return;

    m_mapBooks[sKey]->createBook("web/books/" + sKey);
}
    

/**
* @brief search function calling fitting function from search class
* @param[in] searchOPts 
* @return list of all found books
*/
std::list<std::string>* CBookManager::search(CSearchOptions* searchOpts)
{
    //Select search
    CSearch search(searchOpts);

    //Search
    std::map<std::string, double>* results = search.search(m_mapWords, m_mapWordsTitle, m_mapBooks);

    //Sort results results
    return convertToList(results, searchOpts->getFilterResults());
}



/**
* @brief convert to list and sort list
* @param[in] mapBooks map of books that have been found to contains the searched word
* @param[in, out] matches Map of books and there match with the searched word
* @return list of searchresulst
*/
std::list<std::string>* CBookManager::convertToList(std::map<std::string, double>* mapSR, int sorting)
{
    DBG_INF();
    std::list<std::string>* listBooks = new std::list<std::string>;
    if(mapSR->size() == 0) return listBooks;
    else if(mapSR->size() == 1) {
        listBooks->push_back(mapSR->begin()->first);
        return listBooks;
    }
    DBG_INF();

	// Declaring the type of Predicate that accepts 2 pairs and return a bool
	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
 
	// Defining a lambda function to compare two pairs. It will compare two pairs using second field
	Comp compFunctor;
    if (sorting == 0)
        compFunctor =
			[](auto elem1, auto elem2)
			{
                if (elem1.second == elem2.second) elem1.second+=0.0001;
				return elem1.second > elem2.second;
			};
    else if (sorting == 1)
        compFunctor =
            [this](auto elem1, auto elem2)
			{
                int date1=m_mapBooks[elem1.first]->getDate(), date2=m_mapBooks[elem2.first]->getDate();
                if (date1 == date2) date1+=0.0001;
				return date1 < date2;
			};
    else
        compFunctor =
            [this](auto elem1, auto elem2)
			{
				return m_mapBooks[elem1.first]->getMetadata().getShow() < m_mapBooks[elem2.first]->getMetadata().getShow();
			};

    DBG_INF();
    //Sort by defined sort logic
	std::set<std::pair<std::string, double>, Comp> sorted(mapSR->begin(), mapSR->end(), compFunctor);

    //Convert to list
    for(std::pair<std::string, double> element : sorted)
        listBooks->push_back(element.first); 
    DBG_INF();

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
        if(it->second->getOcr() == false)
            continue;

        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt : it->second->getMapWordsPages())
            m_mapWords[yt.first][it->first] = static_cast<double>(std::get<1>(it->second->getMapWordsPages()[yt.first]))/it->second->getNumPages();
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
        std::string sTitle = it->second->getMetadata().getTitle();
        sTitle=func::convertStr(sTitle);
        std::map<std::string, int> mapWords = func::extractWordsFromString(sTitle);

        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
            m_mapWordsTitle[yt->first][it->first] = 0.1;
    }
}

