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
    m_mapBooks[sKey].setPath("web/books/" + sKey);
    m_mapBooks[sKey].createMapWords();
}
    

/**
* @brief search function calling fitting function from search class
* @param[in] searchOPts 
* @return list of all found books
*/
std::map<std::string, CBook*>* CBookManager::search(unsigned long long id)
{
    //Create empty map of results
    std::map<std::string, CBook*>* results1 = new std::map<std::string, CBook*>;

    if(m_mapSearchs.count(id) == 0)
        return results1;

    CSearch* search = m_mapSearchs[id];

    //Create vector of seperated words
    std::vector<std::string> vWords; 
    func::split(search->getSearchedWord(), "+", vWords);

    //Set searched word
    search->setWord(vWords[0]);

    //Search main word
    results1 = search->search(m_mapWords, m_mapWordsTitle);

    for(unsigned int i=1; i<vWords.size(); i++)
    {
        //Change searched word
        search->setWord(vWords[i]);

        //Get results for second word
        std::map<std::string, CBook*>* results2 = search->search(m_mapWords, m_mapWordsTitle);

        //remove all books, that don't contain both words
        for(auto it=results1->begin(); it!=results1->end(); it++)
        {
            if(results2->count(it->first) == 0)
                results1->erase(it);
        }

        //Delete additional search results
        delete results2;
    }

    deleteSearch(id);

    //Return search results
    return results1;
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
        std::map<std::string, std::vector<size_t>> mapWords;
        it->second.loadPages(mapWords);

        unsigned int counter = 0;
        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
        {
            counter++;
            m_mapWords[yt->first][it->first] = &it->second;
        }
    }

    //unsigned int counter = 0;
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

    //unsigned int counter = 0;
}

/**
* @brief add a new search
*/
void CBookManager::addSearch(CSearch* search) {
    std::unique_lock lck(m_searchLock);
    m_mapSearchs[search->getID()] = search;
}

/**
* @brief get progress of given search
*/
float CBookManager::getProgress(unsigned long long id) {
    std::shared_lock lck(m_searchLock);
    if(m_mapSearchs.count(id) > 0)
        return m_mapSearchs[id]->getProgress();
    else
        return 1;
}


/**
* @brief delete given search and erase from map
*/
void CBookManager::deleteSearch(unsigned long long id) {
    std::unique_lock lck(m_searchLock);
    for(auto it=m_mapSearchs.begin(); it!=m_mapSearchs.end(); it++)
    {
        if(it->first == id) {
            it->second->deleteSearchOptions();
            delete it->second;
        }
        m_mapSearchs.erase(it);
    }
}

