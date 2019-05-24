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
        std::cerr<< "No books found.\n";
        return false;
    }

    //Go though all books and create book
    while((e_allItems = readdir(dir_allItems)) != NULL)
    {
        if(m_mapBooks.count(e_allItems->d_name) > 0)
            addBook(e_allItems->d_name);
        else
            std::cout << e_allItems->d_name << " found which isn't in map!\n";
    }

    //Create map of all words + and of all words in all titles
    createMapWords();
    std::cout << "Map Words: " << m_mapWords.size() << "\n";
    createMapWordsTitle();
    std::cout << "Map Titles: " << m_mapWordsTitle.size() << "\n";

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
    m_mapBooks[sKey].setPath("web/books/" + sKey);
    m_mapBooks[sKey].createMapWords();
}
    

/**
* @brief search function calling fitting function from search class
* @param[in] searchOPts 
* @return list of all found books
*/
std::map<std::string, CBook*>* CBookManager::search(CSearchOptions* searchOpts)
{
    CSearch search(searchOpts);

    //Create empty map of searchResults
    std::map<std::string, CBook*>* mapSearchresults = new std::map<std::string, CBook*>;

    //Normal search (full-match)
    if (searchOpts->getFuzzyness() == 0)
    {
        //Search in ocr and/ or in title
        if(searchOpts->getOnlyTitle() == false)
            search.normalSearch(m_mapWords, mapSearchresults);
        if(searchOpts->getOnlyOcr() == false)
            search.normalSearch(m_mapWordsTitle, mapSearchresults);
    }

    //Contains-Search 
    else if (searchOpts->getFuzzyness() == 1)
    {
        //Search in ocr and/ or in title
        if(searchOpts->getOnlyTitle() == false)
            search.containsSearch(m_mapWords, mapSearchresults);
        if(searchOpts->getOnlyOcr() == false)
            search.containsSearch(m_mapWordsTitle, mapSearchresults);
    }

    //Fuzzy Search
    else
    {
        //Search in ocr and/ or in title
        if(searchOpts->getOnlyTitle() == false)
            search.fuzzySearch(m_mapWords, mapSearchresults);
        if(searchOpts->getOnlyOcr() == false)
            search.fuzzySearch(m_mapWordsTitle, mapSearchresults);
    }

    //Check search-options and remove books from search results, that don't match
    search.removeBooks(mapSearchresults);

    return mapSearchresults;
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

        //Get map of words of current book
        std::map<std::string, int> mapWords = it->second.getMapWords();

        unsigned int counter = 0;
        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
        {
            counter++;
            m_mapWords[yt->first][it->first] = &it->second;
        }
    }

    unsigned int counter = 0;
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
            m_mapWordsTitle[yt->first][it->first] = &it->second;
    }

    unsigned int counter = 0;
}

