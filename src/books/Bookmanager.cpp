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
    createListWords();
    std::cout << "Set words: " << m_listWords.size() << "\n";

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
    std::vector<std::string> searchedWords = func::split2(searchOpts->getSearchedWord(), "+");

    //Select search
    CSearch search(searchOpts);
    search.setWord(searchedWords[0]);

    //Search
    std::map<std::string, double>* results = search.search(m_mapWords, m_mapWordsTitle, m_mapBooks);

    if(searchedWords.size() > 1) {
        search.setWord(searchedWords[1]);
        results = search.search(m_mapWords, m_mapWordsTitle, m_mapBooks);

        for(auto it=results->begin(); it!=results->end(); ++it) {
            if(m_mapBooks[it->first]->getPages(searchOpts->getSearchedWord(), searchOpts->getFuzzyness())->size() == 0)
                results->erase(it);
        }
    }

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
    std::list<std::string>* listBooks = new std::list<std::string>;
    if(mapSR->size() == 0) return listBooks;
    else if(mapSR->size() == 1) {
        listBooks->push_back(mapSR->begin()->first);
        return listBooks;
    }

	// Declaring the type of Predicate that accepts 2 pairs and return a bool
	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;
 
	// Defining a lambda function to compare two pairs. It will compare two pairs using second field
	Comp compFunctor;
    if (sorting == 0)
        compFunctor =
			[](const auto &a,const auto &b)
			{
			if(a.second == b.second)    return a.first > b.first;
			return a.second > b.second;
			};
    else if (sorting == 1)
        compFunctor =
            [this](const auto &elem1,const auto &elem2)
			{
                int date1=m_mapBooks[elem1.first]->getDate(), date2=m_mapBooks[elem2.first]->getDate();
			    if(date1==date2) return elem1.first < elem2.first;
			    return date1 < date2;
			};
    else
        compFunctor =
            [this](const auto &elem1,const auto &elem2)
			{
			    auto &bk1 = m_mapBooks[elem1.first];
			    auto &bk2 = m_mapBooks[elem2.first];
			    std::string s1= bk1->getAuthor();
			    std::string s2= bk2->getAuthor();
			    if(s1==s2)
				return elem1.first < elem2.first;
			    return s1 < s2;
			};

    //Sort by defined sort logic
	std::set<std::pair<std::string, double>, Comp> sorted(mapSR->begin(), mapSR->end(), compFunctor);

    //Convert to list
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

/**
* @brief create list of all words and relevance, ordered by relevance
*/
void CBookManager::createListWords()
{
    std::map<std::string, size_t> mapWords;
    for(auto it = m_mapWords.begin(); it!=m_mapWords.end(); it++) {
        mapWords[it->first] = it->second.size();
    }

	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;

    Comp compFunctor = 
        [](const auto &a, const auto &b)
        {
            if(a.second == b.second) return a.first > b.first;
            return a.second > b.second;
        };

    std::set<std::pair<std::string, size_t>, Comp> sorted(mapWords.begin(), mapWords.end(), compFunctor);

    for(auto elem : sorted)
        m_listWords.push_back({elem.first, elem.second});
}

/**
* @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
*/
std::list<std::string>* CBookManager::getSuggestions_fast(std::string sWord)
{
    std::list<std::string>* listSuggestions = new std::list<std::string>;
    size_t counter=0;
    for(auto it=m_listWords.begin(); it!=m_listWords.end() && counter < 10; it++) {
        if(fuzzy::fuzzy_cmp(it->first, sWord) <= 0.2) {
            listSuggestions->push_back(it->first);
            counter++;
        }
    }
    return listSuggestions;
}

/**
* @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
*/
std::list<std::string>* CBookManager::getSuggestions_acc(std::string sWord)
{
    std::map<std::string, double>* listSuggestions = new std::map<std::string, double>;

    for(auto it=m_listWords.begin(); it!=m_listWords.end(); it++) {
        double val = fuzzy::fuzzy_cmp(it->first, sWord);
        if(val <= 0.2) {
            double rel = (1-val)*it->second;
            if(listSuggestions->size() < 10) 
                (*listSuggestions)[it->first] = rel;
            else
            {
                for(auto yt=listSuggestions->begin(); yt!=listSuggestions->end(); yt++) {
                    if(yt->second < rel) {
                        (*listSuggestions)[it->first] = rel;
                        listSuggestions->erase(yt);
                        break;
                     }
                }
            }
        }
    }
    return convertToList(listSuggestions, 0);
}
            
/**
* @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
*/
std::list<std::string>* CBookManager::getSuggestions_acc2(std::string sWord, bool t, bool o)
{
    std::map<std::string, double>* listSuggestions = new std::map<std::string, double>;

    if(t == false)
    {
        for(auto it=m_mapWords.begin(); it!=m_mapWords.end(); it++) {
            double val = fuzzy::fuzzy_cmp(it->first, sWord);
            if(val <= 0.2) {
                double rel = (1-val)*it->second.size();
                if(listSuggestions->size() < 10) 
                    (*listSuggestions)[it->first] = rel;
                else
                {
                    for(auto yt=listSuggestions->begin(); yt!=listSuggestions->end(); yt++) {
                        if(yt->second < rel) {
                            (*listSuggestions)[it->first] = rel;
                            listSuggestions->erase(yt);
                            break;
                         }
                    }
                }
            }
        }
    }

    std::map<std::string, double>* listSuggestions2 = new std::map<std::string, double>;
    if(o==false)
    {
        for(auto it=m_mapWordsTitle.begin(); it!=m_mapWordsTitle.end(); it++) {
            double val = fuzzy::fuzzy_cmp(it->first, sWord);
            if(val <= 0.2) {
                double rel = (1-val)*it->second.size();
                if(listSuggestions2->size() < 10) 
                    (*listSuggestions2)[it->first] = rel;
                else
                {
                    for(auto yt=listSuggestions2->begin(); yt!=listSuggestions2->end(); yt++) {
                        if(yt->second < rel) {
                            (*listSuggestions2)[it->first] = rel;
                            listSuggestions2->erase(yt);
                            break;
                         }
                    }
                }
            }
        }
    }

    listSuggestions->insert(listSuggestions2->begin(), listSuggestions2->end());
    delete listSuggestions2;
    std::list<std::string>* results = convertToList(listSuggestions, 0);
    if(results->size() > 10) {
        auto it = results->begin();
        for(size_t i=0;i<results->size()-9;i++, it++);
        results->erase(it, results->end());
    }
    return results;
}            
    
