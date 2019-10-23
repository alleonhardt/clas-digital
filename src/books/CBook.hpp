#include <iostream> 
#include <fstream>
#include <map>
#include <list>
#include <string>
#include "func.hpp"
#include "CMetadata.hpp"

#pragma once 

class CBook
{
private:

    std::string m_sKey;                                 //Key of the book
    std::string m_sPath;                                //Path to book (if exists)
    bool m_bOcr;                                        //Book has ocr path

    //Metadata
    CMetadata m_metadata;                               //Json with all metadata 
    std::string m_sAuthor;
    int m_date;

    //Map of matches
    std::unordered_map<std::string, std::list<std::string>> m_mapFuzzy;

    //Map of words_pages
    std::unordered_map<std::string, std::vector<size_t>> m_mapWordsPages;

    //map of relavance
    std::unordered_map<std::string, int> m_mapRelevance;
    int m_numPages;


public:

    CBook();

    /**
    * @Brief Constructor
    * @param[in] sPath Path to book
    * @param[in] map map of words in book
    */
    CBook(nlohmann::json jMetadata);


    // **** Getter **** //

    /**
    * @return Key of the book, after extracting it from the path
    */
    const std::string& getKey();

    /**
    * @brief getter function to return the path to the directory of a book
    * @return string (Path to directory of the book)
    */
    const std::string& getPath();

    /**
    * @return Path to directory of the book
    */
    std::string getOcrPath();

    /**
    * @return Boolean, whether book contains ocr or not 
    */
    bool getOcr();

    /**
    * @return number of pages
    */
    int getNumPages();

    /**
    * @return info.json of book
    */
    CMetadata& getMetadata();

    /**
    * @return lastName, or Name of author
    */
    std::string getAuthor();

    /**
    * @return date or -1 if date does not exists or is currupted
    */
    int getDate();

    /**
    * @brief return whether book is publically accessable 
    */
    bool getPublic();


    std::unordered_map<std::string, std::vector<size_t>>&    getMapWordsPages();
    std::unordered_map<std::string, std::list<std::string>>& getMapFuzzy();
    std::unordered_map<std::string, int>&                    getMapRelevance();

 
    // **** SETTER **** //
    
    /**
    * @param[in] path set Path to book)
    */
    void setPath(std::string sPath);
    
    /**
    * @brief Create a map of all word of this book
    */
    void createMapWords();

    void createPages();

    /**
    * @brief safe map of all words and pages on which word occures to disc
    */
    void safePages();

    /**
    * @brief load words and pages on which word occures into map
    */
    void loadPages();

    /**
    * @brief getPages calls the matching getPages... function according to fuzzyness
    */
    std::map<int, std::vector<std::string>>* getPages(std::string sInput, int fuzzyness);

    std::map<int, std::vector<std::string>>* getPages(std::vector<std::string>& vWords, std::map<int, std::vector<std::string>>* (CBook::*find)(std::string, std::unordered_map<std::string, std::vector<size_t>>&));

    /**
    * @brief Create map of pages and found words for i-word (full-search).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPagesFull(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages);

    /**
    * @brief Create map of pages and found words for i-word (Contains).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPagesContains(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages);

    /** 
    * @brief Create map of pages and found words for i-word (fuzzy).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPagesFuzzy(std::string sWord, std::unordered_map<std::string, std::vector<size_t>>& mapWordsPages);

    /**
    * @brief Remove all elements from mapPages, which do not exist in results2. 
    */
    void removePages(std::map<int, std::vector<std::string>>* mapPages, std::map<int, std::vector<std::string>>* results2);


    int getMatches(std::string sInput, int fuzzyness);

    // ***** GET PREVIEW - functions ***** //

    /**
    * @brief get a preview of the page where the searched word has been found
    * @param sWord (searched word)
    * @param fuzzyness
    * @return Preview
    */
    std::string getPreview(std::string sWord);

    /**
    * @brief find page with best match. Deliver page and match
    * @param[in] sWord (searched word)
    * @param[in] fuzzyness
    * @param[in, out] sMatch (found match)
    * @return Page on which the match was found.
    */
    size_t getBestMatch(std::string sWord, std::string &sMatch);

    /*
    * @brief Find preview with matched word (best match), and page on which the match was found.
    * @param[in] sWord (best Match)
    * @param[in] page (page on which match was found)
    * @return preview for this book
    */
    std::string getPreviewMatch(std::string sWord, size_t page);

    void shortenPreview(std::string& finalResult);
};
