/*
* @author Jan van Dick
*/

#ifndef CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_BOOK_H_
#define CLASDIGITAL_SRC_SEARCH_BOOKMANAGER_BOOK_H_

#include <cstddef>
#include <fstream>
#include <filesystem>
#include <iostream> 
#include <list>
#include <map>
#include <string>
#include <set>
#include <vector>

#include "searched_word_object.h"
#include "search_object.h"
#include "word_info.h"
#include "tmp_word_info.h"
#include "func.hpp"
#include "fuzzy.hpp"
#include "gramma.h"
#include "metadata_handler.h"
#include "sorted_matches.h"

#define private public

class Book {

public:

  Book();

  /**
  * Constructor initializing metadata.
  * @param[in] metadata for this book.
  */
  Book(std::map<short, std::string> metadata);

  // **** Getter **** //
  const std::string& key();
  const std::string& path();
  std::string ocr_path();
  bool has_ocr() const;
  bool has_images() const;
  int num_pages();
  int date();
  std::vector<std::string> collections();
  typedef std::unordered_map<std::string, std::vector<WordInfo>> index_map_type;
  index_map_type& words_on_pages();
  index_map_type& words_in_tags();
  typedef std::unordered_map<std::string, SortedMatches> fuzzy_result_type;
  fuzzy_result_type& corpus_fuzzy_matches();
  fuzzy_result_type& metadata_fuzzy_matches();
  
  ///< Return whether book has images or ocr
  bool HasContent() const;

  std::string GetFromMetadata(std::string tag) const;

  /**
   * Retuns the length of the document.
   * The length of the document is the number of different words in the corpus
   * and the number of different words in metadata combined.
   */
  size_t GetLength() const;

  bool IsPublic() const;

  // **** setter **** //
  static void set_dict(Dict* dict);
  static void set_metadata_tag_reference(std::map<short, std::pair<std::string, double>> ref);
  static void set_reverted_tag_reference(std::map<std::string, short> reverted_ref);

  void SetPath(std::string sPath);

  void UpdateMetadata(std::map<short, std::string> metadata);
  

  // **** create book and maps (pages, relevance, preview) **** // 

  /**
   * Sets path, has_ocr and has_images.
   * @param path to book.
   */
  void InitializeBook(std::string path, bool reload_pages);

  /**
  * Pre-processes ocr-file.
  * Creates/loads map of pages, safes to json.
  * @param[in] sPath (path to book)
  */
  void InitializePreProcessing(bool reload_pages);

  /**
   * Add a single new pages, generate by Teseract. 
   * Also save the page to disc and add it to complete ocr. 
   * @param[in] page_text (text on page)
   * @param[in] page (page number) 
   * @param[in] max_page (number of pages in book)
   */
  void AddPage(std::string page_text, std::string pnum, std::string max_pnum);


  // *** get-pages functions *** //
  
  /**
  * Checks whether all words found occur on the same page or in metadata.
  * @param[in] sWords (words to check)
  * @param[in] fuzzyness (fuzzy-search yes/ no)
  * @return boolean indicating whether words or on the same page or not
  */
  bool OnSamePage(std::vector<std::string> words, bool fuzzyness);

  /**
  * GetPages calls findPages (extracting all pages from given word) for each 
  * word searched and removes duplicates and/ or pages, where not all words searched
  * occur.
  * @param search_object storing search word(s) and fuzzyness.
  * @return map of pages, with vector of words on this page.
  */
  std::map<int, std::vector<std::string>> GetPages(SearchObject search_object);


  // ***** GET PREVIEW - functions ***** //

  /**
  * Get a preview for each searched word. Try to highlight/ find words
  * in one preview. Only add a new preview if word could not be found in
  * the preview created so far.
  * @param words list of all searched words each entry containing the original
  * word, the converted word and the scope in which the word was found, as a
  * SearchedWord-Object.
  * @return one ore more previews ready formatted for output.
  */
  std::string GetPreview(const std::vector<SearchedWordObject>& words, bool fuzzy_search);

private:

  static Dict* dict_;
  static std::map<short, std::pair<std::string, double>> metadata_tag_reference_;
  static std::map<std::string, short> reverted_tag_reference_;

  // *** member variables *** //
  std::string key_;  ///< Key of the book
  std::string path_;  ///< Path to book (if exists)
  bool has_ocr_;  ///< Book has ocr path
  bool has_images_; ///has images or ocr

  // New Metadata
  std::map<short, std::string> metadata_;

  // From json/ metadata-handler
  int date_;  ///< Date for fast access during search
  std::vector<std::string> collections_; ///< Collections (fast access)

  ///Map of matches found with fuzzy-search (contains/ fuzzy)
  std::unordered_map<std::string, SortedMatches> corpus_fuzzy_matches_;
  std::unordered_map<std::string, SortedMatches> metadata_fuzzy_matches_;

  ///Map of words_pages_pos_relevance
  std::unordered_map<std::string, std::vector<WordInfo>> words_on_pages_;
  std::unordered_map<std::string, std::vector<WordInfo>> words_in_tags_;

  int num_pages_;  ///< Number of pages in book

  // ***** private functions *** //
 
  // *** create book and maps (pages, relevance and preview) *** // 

  void CreateCorpusIndex();
  void CreateMetadataIndex();
  
  typedef  std::map<std::string, TempWordInfo> temp_index_map;
  ///Create map of all pages and safe.
  void SeperatePages(temp_index_map& temp_map_pages);

  /**
  * Function adding all words from one page to map of words. 
  * Writes the page to disc as single file. 
  * @param[in] buffer (string holding current page)
  * @param[in] page (number indexing current page)
  */
  void CreatePage(temp_index_map& temp_map_pages, std::string buffer, size_t page);

  /**
  * Find preview position for each word in map of words/pages.
  */
  void CreateMapPreview(temp_index_map& temp_map_pages);

  /**
   * Find preview with matched word + pages.
   * @param[in] sWord (best Match)
   * @param[in] page (page on which match was found)
   * @return preview for this book
   */
  size_t GetPreviewPosition(std::string word, size_t page);


  /**
   * Convert all words and handle resulting duplicates.
   * All words are converted to lowercase and non-utf8-characters are replaced. 
   * Also if duplicate are resulting the word-infos are "joined".
   * @param[in, out] temp_map_pages.
   */
  void ConvertWords(temp_index_map& temp_map_pages);
  /**
   * Convert index map to a map of base-forms and conjunction.
   * @param[in, out] temp_map_pages.
   */
  void GenerateBaseFormMap(temp_index_map& temp_map_pages, 
      std::unordered_map<std::string, std::vector<WordInfo>>& index_map);

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
   * Create map of pages and found words on page. As words found may differ from 
   * searched word. (F.e. "Löwe" may match for "Löwin" even if fuzziness == false).
   * @param word searched.
   * @param fuzzyness indicating whether to search fuzzy or not.
   * @return map of all pages on which word was found.
   */
  std::map<int, std::vector<std::string>> FindPagesAndMatches(std::string word, bool fuzzyness);

  /**
   * Create map of only pages and found words on page.
   * @param word searched.
   * @param fuzzyness indicating whether to search fuzzy or not.
   * @return map of all pages.
   */
  std::vector<size_t> FindOnlyPages(std::string word, bool fuzzyness);

  /**
   * Remove all elements from results1, which do not exist in results2. 
   * Also add all matches on a page in results2 to matches on same page in results1.
   * @param[in, out] results1 (map of pages and words on page).
   * @param[in] results2 (map of param and words on page).
   */
  void RemovePages(std::map<int, std::vector<std::string>>& results1, 
                   std::map<int, std::vector<std::string>>& results2);

  /**
   * Find all base-forms matching the searched word. 
   * Find matching base-form(s). In case of fuzzy search these are all
   * corresponding entries in found_fuzzy_matches (might be corpus or metadata). 
   * @param word search for.
   * @param fuzzy_search indicating whether to search fuzzy.
   * @param found_fuzzy_matches map of already found fuzzy matches (corpus or
   * metadata).
   * @return vector of all base-forms matching the searched word.
   */
  std::vector<std::string> FindBaseForms(std::string word, bool fuzzy_search,
    const index_map_type& index_map, const fuzzy_result_type& found_fuzzy_matches) const;

  WordInfo FindBestWordInfo(std::string original_word, std::string converted_word, 
      bool fuzzy_search, const index_map_type& index_map, const fuzzy_result_type& found_fuzzy_matches) const;

  // *** Previews *** //

  /**
   * @brief Get a preview of a single word. (~150 characters + highlighting)
   * @param sWord (searched word)
   * @return Preview.
   */
  std::string GetOnePreview(std::string orginal, std::string converted, short scope, bool fuzzy_search);
};

#endif
