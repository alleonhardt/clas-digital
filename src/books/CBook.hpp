/*
* @author Jan van Dick
*/

#include <iostream> 
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <set>
#include "func.hpp"
#include "fuzzy.hpp"
#include "metadata_handler.h"

#pragma once 

class CBook
{
public:

    CBook();

    /**
    * Brief Constructor
    * @param[in] sPath Path to book
    * @param[in] map map of words in book
    */
    CBook(nlohmann::json jMetadata);


    // **** Getter **** //
    const std::string& get_key();
    const std::string& get_path();
    std::string get_ocr_path();
    bool has_ocr() const;
    bool has_images() const;
    int get_num_pages();
    MetadataHandler& get_metadata();
    std::string get_author();
    int get_date();
    std::vector<std::string> get_collections();
    std::string get_author_date();
    std::unordered_map<std::string, std::tuple<std::vector<size_t>, int, size_t>>&   
    get_map_words_pages();
    std::unordered_map<std::string, std::list<std::pair<std::string, double>>>& 
    get_found_fuzzy_matches();
    std::unordered_map<std::string, std::list<std::string>>& 
    get_found_grammatical_matches();
    
    
    ///Return whether book is publicly accessible 
    bool getPublic();

    ///Return whether book has images or ocr
    bool hasContent() const;

    ///Return whether book has title, author or date
    bool checkJson();

    ///Return "[author], [date]" and add "book not yet scanned", when hasOcr == false
    std::string getAuthorDateScanned();

 
    // **** setter **** //
    
    void setPath(std::string sPath);
    

    // **** create book and maps (pages, relevance, preview) **** // 

    /**
    * Sets m_hasOcr/Images,
    * if hasOcr==true, safes json to disc creates/ loads map of words/pages.
    * Creates/loads map of pages, sets hasOCR and hasImages and safes json
    * @param[in] sPath (path to book)
    */
    void createBook(std::string sPath);

    ///Add a single new pages, generate by Teseract
    void addPage(std::string sInput, std::string sPage, std::string sMaxPage);


    // *** get-pages functions *** //
    
    /**
    * Checks whether all words found occur on the same page or in metadata.
    * @param[in] sWords (words to check)
    * @param[in] fuzzyness (fuzzy-search yes/ no)
    * @return boolean indicating whether words or on the same page or not
    */
    bool onSamePage(std::vector<std::string> sWords, bool fuzzyness);

    /**
    * GetPages calls findPages (extracting all pages from given word) for each 
    * word searched and removes duplicates and/ or pages, where not all words searched
    * occur.
    * @param[in] sInput (list of searched words as a string, separated by ' ' or + 
    * @param[in] fuzzyness (boolean indicating whether fuzziness is set or not)
    * @return map of pages, with vector of words on this page 
    * (all the same if fuzziness==false)
    */
    std::map<int, std::vector<std::string>>* getPages(std::string sInput, bool fuzzyness);



    // ***** GET PREVIEW - functions ***** //

    /**
    * Get a preview of the page where the searched word has been found
    * @param sWord (searched word)
    * @return Preview
    */
    std::string getPreview(std::string sWord);

    
private:

    // *** member variables *** //

    std::string key_;  ///< Key of the book
    std::string path_;  ///< Path to book (if exists)
    bool has_ocr_;  ///< Book has ocr path
    bool has_images_; ///has images or ocr

    //From json/ metadata-handler
    MetadataHandler metadata_;  ///< Json with all metadata 
    std::string author_;  ///< Author for fast access during search
    int date_;  ///< Date for fast access during search
    std::vector<std::string> collections_; ///< Collections (fast access)
    std::string author_date_;  ///< [author], [date] (fast access)

    ///Map of matches found with fuzzy-search (contains/ fuzzy)
    std::unordered_map<std::string, std::list<std::pair<std::string, double>>> 
    found_fuzzy_matches_;
    ///Map of matches found via different grammatical forms
    std::unordered_map<std::string, std::list<std::string>> 
    found_grammatical_matches_; 

    ///Map of words_pages_pos_relevance
    std::unordered_map<std::string, std::tuple<std::vector<size_t>, int, size_t>>
    map_words_pages_;

    int num_pages_;  ///< Number of pages in book


    // ***** private functions *** //
   
    // *** create book and maps (pages, relevance and preview) *** // 

    ///Create map of all pages and safe.
    void CreatePages();

    /**
    * Function adding all words from one page to map of words. Writes the
    * page to disc as single file. Add the page break line to a string used to 
    * convert new format to old/ normal format. 
    * @param[in] sBuffer (string holding current page)
    * @param[in, out] sConvert (copy of complete ocr in page-mark-format)
    * @param[in] page (number indexing current page)
    * @param[in] mark (page-mark-format yes/no)
    */
    void CreatePage(std::string sBuffer, std::string& sConvert, size_t page, 
                    bool mark);

    /**
    * Find preview position for each word in map of words/pages.
    */
    void CreateMapPreview();

    /**
    * Safe map of all words and pages to disc
    */
    void SafePages();

    /**
    * Load words and pages on which word occurs into map
    */
    void LoadPages();
    
    
    // *** get-pages functions *** //

    /**
    * Create map of pages and found words on page. As words found may differ 
    * from searched word. (F.e. "Löwe" may match for "Löwin" even if fuzziness
    * == false).
    * @param[in] sWord (word search)
    * @param[in] fuzzyness (boolean indicating whether fuzziness is set or not)
    * @return map of all pages on which word was found.
    */
    std::map<int, std::vector<std::string>>* FindPages(std::string sWord, 
                                                       bool fuzzyness);

    /**
    * Remove all elements from results-1, which do not exist in results-2. 
    * @param[in, out] results1 (map of pages and words on page).
    * @param[in] results2 (map of param and words on page).
    */
    void RemovePages(std::map<int, std::vector<std::string>>* results1, 
                     std::map<int, std::vector<std::string>>* results2);

    /** 
    * Check whether all words occur in metadata
    * @param[in] vWords (words to check)
    * @param[in] fuzzyness (fuzzy-search yes/ no)
    * @return boolean whether all words are in metadata or not.
    */
    bool MetadataCmp(std::vector<std::string> vWords, bool fuzzyness);

    /**
    * Generates list of all pages the searched word occurs on.
    * This function also checks preview fuzzy-matches found when searching for
    * this word. 
    * @param[in] word to generate list of pages for.
    * @return list of pages
    */
    std::vector<size_t> PagesFromWord(std::string sWord, bool fuzzyness);


    // *** Previews *** //
    std::string GetOnePreview(std::string sWord);
    std::string GetPreviewText(std::string& sWord, size_t& pos, size_t& page);
    std::string GetPreviewTitle(std::string& sWord, size_t& pos);
    /*
    * Find preview with matched word + pages.
    * @param[in] sWord (best Match)
    * @param[in] page (page on which match was found)
    * @return preview for this book
    */
    size_t GetPreviewPosition(std::string sWord);
    void ShortenPreview(std::string& finalResult);
};
