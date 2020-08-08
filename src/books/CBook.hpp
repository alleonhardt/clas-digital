#include <iostream> 
#include <fstream>
#include <filesystem>
#include <map>
#include <list>
#include <string>
#include "func.hpp"
#include "fuzzy.hpp"
#include "CMetadata.hpp"

#pragma once 

class CBook
{
private:

    std::string m_sKey;         ///< Key of the book
    std::string m_sPath;        ///< Path to book (if exists)
    bool m_hasOcr;              ///< Book has ocr path
    bool m_hasImages;

    //Metadata
    CMetadata m_metadata;       ///< Json with all metadata 
    std::string m_sAuthor;      ///< Author for fast access during search
    int m_date;                 ///< Date for fast access during search
    std::vector<std::string> m_collections; ///< Collections for fast access during search
    std::string m_author_date;  ///< [author], [date] for fast acces during search.

    ///Map of matches found with fuzzy-search
    std::unordered_map<std::string, std::list<std::pair<std::string, double>>> m_mapFuzzy;
    ///Map of matches found with contains-search
    std::unordered_map<std::string, std::list<std::string>> m_mapFull;

    ///Map of words_pages_pos_relevance
    std::unordered_map<std::string, std::tuple<std::vector<size_t>, int, size_t>> m_mapWordsPages;

    int m_numPages;     ///< Number of pages in book


public:

    CBook();

    /**
    * @Brief Constructor
    * @param[in] sPath Path to book
    * @param[in] map map of words in book
    */
    CBook(nlohmann::json jMetadata);


    // **** Getter **** //

    ///Return Key of the book, after extracting it from the path
    const std::string& getKey();

    ///Getter function to return the path to the directory of a book
    const std::string& getPath();

    ///Return Path to directory of the book
    std::string getOcrPath();

    ///Return Boolean, whether book contains ocr or not 
    bool hasOcr() const;

    ///Returns whether book has images.
    bool hasImages() const;

    ///Return whether book has images or ocr
    bool hasContent() const;

    ///Return whether book has title, author or date
    bool checkJson();

    ///Return "[author], [date]" and add "book not yet scanned", when hasOcr == false
    std::string getAuthorDateScanned();

    ///Return number of pages
    int getNumPages();

    ///Return metadata-class to access all metadata 
    CMetadata& getMetadata();

    ///Return lastName, or Name of author
    std::string getAuthor();

    ///Return date or -1 if date does not exists or is currupted
    int getDate();

    ///Return collections the book is in
    std::vector<std::string> getCollections();

    ///Return "[author], [date]
    std::string getAuthorDate();

    ///Return whether book is publicly accessible 
    bool getPublic();

    ///Get map of words with list of pages, preview-position and relevance.
    std::unordered_map<std::string, std::tuple<std::vector<size_t>, int, size_t>>&   getMapWordsPages();
    
    ///Return matches found with fuzzy-search
    std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& getMapFuzzy();

    ///Return matches found with contains-search
    std::unordered_map<std::string, std::list<std::string>>& getMapFull();

 
    // **** SETTER **** //
    
    /**
    * @param[in] path set Path to book)
    */

    ///Set path to book-directory.
    void setPath(std::string sPath);
    

    // **** CREATE BOOK AND MAPS (PAGES, RELEVANCE, PREVIEW) **** // 

    /**
    * @brief sets m_hasOcr/Images,
    * if hasOcr==true, safes json to disc creates/ loads map of words/pages.
    * @brief creates/loads map of pages, sets hasOCR and hasImages and safes json
    * @param[in] sPath (path to book)
    */
    void createBook(std::string sPath);

    ///Create map of all pages and safe.
    void createPages();

    /**
    * @brief function adding all words from one page to map of words. Writes the
    * page to disc as single file. Add the page break line to a string used to convert 
    * new format to old/ normal format. 
    * @param[in] sBuffer (string holding current page)
    * @param[in, out] sConvert (string holding copy of complete ocr in page-mark-format)
    * @param[in] page (number indexing current page)
    * @param[in] mark (page-mark-format yes/no)
    */
    void createPage(std::string sBuffer, std::string& sConvert, size_t page, bool mark);

    /**
    * @brief Find preview position for each word in map of words/pages.
    */
    void createMapPreview();

    /**
    * @brief safe map of all words and pages to disc
    */
    void safePages();

    /**
    * @brief load words and pages on which word occurs into map
    */
    void loadPages();


    // **** GET PAGES FUNCTIONS **** //

    /**
    * @brief getPages calls the matching getPages... function according to fuzzyness
    */
    std::map<int, std::vector<std::string>>* getPages(std::string sInput, bool fuzzyness);

    /**
    * @brief Create map of pages and found words for i-word (full-search).
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* findPages(std::string sWord, bool fuzzyness);

    /**
    * @brief Remove all elements from mapPages, which do not exist in results2. 
    */
    void removePages(std::map<int, std::vector<std::string>>* results1, std::map<int, std::vector<std::string>>* results2);

    /**
    * @brief checks whether words found occur on the same page
    * @param[in] sWords (words to check)
    * @return boolean indicating whether words or on the same page or not
    */
    bool onSamePage(std::vector<std::string> sWords);

    /**
    * @brief generates list of all pages of this book, also checking fuzzy-matches
    * (but not contains-matches (please check!!!))
    * @param[in] word to generate list of pages for.
    * @return list of pages
    */
    std::vector<size_t> pages(std::string sWord);


    // ***** GET PREVIEW - functions ***** //

    /**
    * @brief get a preview of the page where the searched word has been found
    * @param sWord (searched word)
    * @return Preview
    */
    std::string getPreview(std::string sWord);
    std::string getOnePreview(std::string sWord);

    std::string getPreviewText(std::string& sWord, size_t& pos, size_t& page);
    std::string getPreviewTitle(std::string& sWord, size_t& pos);

    /*
    * @brief Find preview with matched word (best match), and page on which the match was found.
    * @param[in] sWord (best Match)
    * @param[in] page (page on which match was found)
    * @return preview for this book
    */
    size_t getPreviewPosition(std::string sWord);

    void shortenPreview(std::string& finalResult);

    ///Add a single new pages, generate by Teseract
    void addPage(std::string sInput, std::string sPage, std::string sMaxPage);
};
