#include "CBookManager.hpp"


/**
* @return map of all book
*/
const std::map<std::string, CBook>& CBookManager::getMapOfBooks() {
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
        //Ignore files that aren't books
        if(e_allItems->d_name[0] == '.')
            continue;

        //Create book
        std::string sPath = "web/books/";
        sPath.append(e_allItems->d_name); 
        CBook book (sPath);

        //Add book to map of all books
        m_mapBooks[book.getKey()] = book;
    }

    createMapWords();
    createMapWordsTitle();

    return true;
}


/**
* @brief search function calling fitting function from search class
* @param[in] searchOPts 
* @return list of all found books
*/
std::list<CBook*>& CBookManager::search(std::string sWord, bool ocr, bool title)
{
    CSearch search(sWord);

    //Create empty map of searchResults
    std::map<std::string, CBook*> mapSearchresults;

    //Search in ocr and/ or in title
    if(title == false)
        search.normalSearch(m_mapWordsTitle, mapSearchresults);
    if(ocr == false)
        search.normalSearch(m_mapWordsTitle, mapSearchresults);

    //Convert map to list of books
    std::list<CBook*> listSearchResults;
    for(auto it=mapSearchresults.begin(); it!=mapSearchresults.end(); it++) {
        listSearchResults.push_back(it->second);
    }
    
    return listSearchResults;
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

        //Add new words to map of all words
        addWords(it->second, it->second.getMapWords());
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
        CFunctions function;
        std::map<std::string, int> mapWords; 
        function.createMapofWordsFromString(it->second.getTitle(), mapWords);

        //Add new words to map of all words
        addWords(it->second, mapWords);
    }

    unsigned int counter = 0;
}

/**
* @brief Iterate over all words in this book. Check whether word already exists in list off all words.
* @param[in] mapWords map of all words in current book)
*/
void CBookManager::addWords (CBook& book, const std::map<std::string, int>& mapWords)
{
    //Iterate over all words in this book. Check whether word already exists in list off all words.
    //If yes:   add book to list of books matching to current word.
    //If no:    add word to map of all words and book to list mathcing to this word.
    for(auto yt=mapWords.begin(); yt!=mapWords.end(); yt++)
    {
        //Add book to list of books matching to current word.
        if(m_mapWordsTitle.count(yt->first) > 0)
            m_mapWordsTitle.at(yt->first).push_back(&book);

        //add word to map of all words and book to list mathcing to this word.
        else
        {
            std::list<CBook*> listBooks;
            listBooks.push_back(&book);
            m_mapWordsTitle[yt->first] = listBooks;
        }
    }
}
