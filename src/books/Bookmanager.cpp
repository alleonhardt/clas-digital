#include "CBookManager.hpp"
#include "debug/debug.hpp"

//Constructor
CBookManager::CBookManager()
{
    std::string sBuffer;
    std::ifstream read("web/dictionary_low.txt");
    
    while(getline(read, sBuffer))
    {
        std::vector<std::string> vec = func::split2(sBuffer, ":");
        std::vector<std::string> vec2 = func::split2(vec[1], ",");
        for(size_t i=0; i<vec2.size(); i++) {
            m_dict[vec[0]].insert(vec2[i]);
            m_dict[vec2[i]].insert(vec[0]);
        }
    }
    std::cout << "Created dictionary. Size: " << m_dict.size() << "\n";
}

/**
* @return map of all book
*/
std::unordered_map<std::string, CBook*>& CBookManager::getMapOfBooks() {
    return m_mapBooks;
}

CBookManager::MAPWORDS& CBookManager::getMapofAuthors() {
    return m_mapWordsAuthors;
}

/**
* @return map of unique authors ([lastName]-[firstNam])
*/
std::map<std::string, std::vector<std::string>>& CBookManager::getMapofUniqueAuthors() {
    return m_mapUniqueAuthors;
}

void CBookManager::writeListofBooksWithBSB() {    
    
    std::string untracked_books = "Currently untracked books:\n";
    std::string inBSB_noOcr = "Buecher mit bsb-link, aber ohne OCR: \n";
    std::string gibtEsBeiBSB_noOCR = "Buecher mit tag \"gibtEsBeiBSB\" aber ohne ocr: \n";
    std::string gibtEsBeiBSB_OCR = "Buecher mit tag \"gibtEsBeiBSB\" aber mit ocr: \n";
    std::string BSBDownLoadFertig_noOCR = "Buecher mit tag \"BSBDownLoadFertig\" aber ohne ocr: \n";

    for(auto it : m_mapBooks)
    {
        if(it.second->hasOcr() == false)
        {
            //Check if book has a bsb-link (archiveLocation)
            if(it.second->getMetadata().getMetadata("archiveLocation", "data") != "")
                inBSB_noOcr+= it.second->getKey() + ": " + it.second->getMetadata().getShow2() + "\n";
            
            //Check if book has tag: "GibtEsBeiBSB"
            if(it.second->getMetadata().hasTag("GibtEsBeiBSB") == true)
                gibtEsBeiBSB_noOCR += it.second->getKey() + ": " + it.second->getMetadata().getShow2() + "\n";
    
            //Check if book has tag: "BSBDownLoadFertig"
            if(it.second->getMetadata().hasTag("BSBDownLoadFertig") == true)
                BSBDownLoadFertig_noOCR += it.second->getKey() + ": " + it.second->getMetadata().getShow2()+"\n";
        }
        else if(it.second->getMetadata().hasTag("GibtEsBeiBSB") == true)
            gibtEsBeiBSB_OCR += it.second->getKey() + ": " + it.second->getMetadata().getShow2() + "\n";
        
    }

    //Search for untracked books
    std::vector<std::filesystem::path> vec;
    for(const auto& entry : std::filesystem::directory_iterator("web/books"))
    {
	    std::string fileNameStr = entry.path().filename().string();
        if(entry.is_directory() && m_mapBooks.count(fileNameStr) == 0)
        {
            std::ifstream readJson(entry.path().string() + "/info.json");
            if(!readJson) 
                continue;
            nlohmann::json j;
            readJson >> j;
            if(j.count("data") == 0 || j["data"].count("creators") == 0 || j["data"]["creators"][0].size() == 0)
                continue;

            std::string sName = j["data"]["creators"][0].value("lastName", "unkown name");
            std::string sTitle = j["data"].value("title", "unkown title");
            untracked_books += fileNameStr + " - " + sName + ", \"" + sTitle + "\"\n";
            readJson.close(); 
            vec.push_back(entry); 
        }
    }

    //Move old zotero files to archive: "zotero_old"
    if(std::filesystem::exists("web/books/zotero_old") == false)
        std::filesystem::create_directories("web/books/zotero_old");
    for(const auto& it : vec)
    {
        std::string path=it.string().substr(0, 9)+"/zotero_old"+it.string().substr(9);
        try{
            std::filesystem::rename(it, path);
        }
        catch (std::filesystem::filesystem_error& e) {
            std::cout << e.what() << std::endl;
        }
    }

    //Save books with bsb-link but o ocr
    std::ofstream writeBSB_NoOCR("inBSB_noOcr.txt");
    writeBSB_NoOCR << inBSB_noOcr;
    writeBSB_NoOCR.close();

    //Save books with tag GibtEsBeiBSB but without ocr
    std::ofstream writeGIBT_NoOCR("gibtEsBeiBSB_noOCR.txt");
    writeGIBT_NoOCR << gibtEsBeiBSB_noOCR;
    writeGIBT_NoOCR.close();

    //Save books with tag GibtEsBeiBSB but with ocr
    std::ofstream writeGIBT_OCR("gibtEsBeiBSB_OCR.txt");
    writeGIBT_OCR << gibtEsBeiBSB_OCR;
    writeGIBT_OCR.close();

    //Save books with tag BSBDownLoadFertig but with ocr
    std::ofstream writeFERTIG_noOCR("BSBDownLoadFertig_noOCR.txt");
    writeFERTIG_noOCR << BSBDownLoadFertig_noOCR;
    writeFERTIG_noOCR.close();

    //Write all untracked books to seperate files
    if(std::filesystem::exists("untracked_books.txt"))
        std::filesystem::rename("untracked_books.txt", "web/books/zotero_old/untracked_books.txt");
    if(vec.size() > 0)
    {
        std::ofstream writeUntracked("untracked_books.txt");
        writeUntracked << untracked_books;
        writeUntracked.close();
    }
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
    createMapWordsAuthor();
    std::cout << "Map words:   " << m_mapWords.size() << "\n";
    std::cout << "Map title:   " << m_mapWordsTitle.size() << "\n";
    std::cout << "Map authors: " << m_mapWordsAuthors.size() << "\n";

    writeListofBooksWithBSB();
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
    std::vector<std::string> sWords = func::split2(searchOpts->getSearchedWord(), "+");

    //Start first search
    CSearch search(searchOpts, sWords[0]);
    std::map<std::string, double>* results = search.search(m_mapWords, m_mapWordsTitle, m_mapWordsAuthors, m_mapBooks, m_dict);

    for(size_t i=1; i<sWords.size(); i++)
    {
        if(sWords[i].length() == 0)
            continue;

        CSearch search2(searchOpts, sWords[i]);
        std::map<std::string, double>* results2 = search2.search(m_mapWords, m_mapWordsTitle, m_mapWordsAuthors, m_mapBooks, m_dict);

        for(auto it=results->begin(); it!=results->end();) {
            if(results2->count(it->first) == 0)
                 it = results->erase(it);
            else if(m_mapBooks[it->first]->hasOcr() == true && m_mapBooks[it->first]->onSamePage(sWords)==false)
                 it = results->erase(it);
            else
                ++it;
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
        if(it->second->hasOcr() == false)
            continue;

        //Iterate over all words in this book. Check whether word already exists in list off all words.
        for(auto yt : it->second->getMapWordsPages())
            m_mapWords[yt.first][it->first] = static_cast<double>(std::get<1>(it->second->getMapWordsPages()[yt.first]))/it->second->getNumPages();
    }

    createListWords(m_mapWords, m_listWords);
    std::cout << "List words: " << m_listWords.size() << "\n";
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
* @brief create map of all words (key) and author names in which the word occurs (value)
*/
void CBookManager::createMapWordsAuthor()
{
    //Iterate over all books. Add words to list (if the word does not already exist in map of all words)
    //or add book to list of books (if word already exists).
    for(auto it=m_mapBooks.begin(); it!=m_mapBooks.end(); it++)
    {
        for(auto author : it->second->getMetadata().getAuthorsKeys())
        {
            m_mapWordsAuthors[func::returnToLower(author["lastname"])][it->first] = 0.1;
            m_mapUniqueAuthors[author["key"]].push_back(it->first);
        }
    }
    createListWords(m_mapWordsAuthors, m_listAuthors);
}

/**
* @brief create list of all words and relevance, ordered by relevance
*/
void CBookManager::createListWords(MAPWORDS& mapWords, sortedList& listWords)
{
    std::map<std::string, size_t> mapRel;
    for(auto it = mapWords.begin(); it!=mapWords.end(); it++) {
        mapRel[it->first] = it->second.size();
    }

	typedef std::function<bool(std::pair<std::string, double>, std::pair<std::string, double>)> Comp;

    Comp compFunctor = 
        [](const auto &a, const auto &b)
        {
            if(a.second == b.second) return a.first > b.first;
            return a.second > b.second;
        };

    std::set<std::pair<std::string, size_t>, Comp> sorted(mapRel.begin(), mapRel.end(), compFunctor);

    for(auto elem : sorted)
        listWords.push_back({elem.first, elem.second});
}


std::list<std::string>* CBookManager::getSuggestions(std::string sWord, std::string sWhere)
{
    if(sWhere=="corpus") return getSuggestions(sWord, m_listWords);
    if(sWhere=="author") return getSuggestions(sWord, m_listAuthors);
    return NULL;
}

/**
* @brief return a list of 10 words, fitting search Word, sorted by in how many books they apear
*/
std::list<std::string>* CBookManager::getSuggestions(std::string sWord, sortedList& listWords)
{
    std::cout << sWord << std::endl;
    func::convertToLower(sWord);
    sWord = func::convertStr(sWord);
    std::map<std::string, double>* suggs = new std::map<std::string, double>;
    size_t counter=0;
    for(auto it=listWords.begin(); it!=listWords.end() && counter < 10; it++) {
        double value = fuzzy::fuzzy_cmp(it->first, sWord);
        if(value > 0 && value <= 0.2) {
            (*suggs)[it->first] = value*(-1);
            counter++;
        }
    }
    std::list<std::string>* listSuggestions = convertToList(suggs, 0);
    delete suggs;
    return listSuggestions;
}
            

