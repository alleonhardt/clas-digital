/** 
 * @author fux
 * Class storing a result objects. 
 * Result objects are store for each book which was found during search.
 * The reults-object has the purpose to indicate, where the word was found 
 * (metadata/ corpus), to store the (metadata- and corpus-) score and to store 
 * the pointer to the book.
 */

#ifndef SRC_SEARCH_DATA_MODELS_RESULTOBJECT_H_
#define SRC_SEARCH_DATA_MODELS_RESULTOBJECT_H_

#include "book_manager/book.h"
class ResultObject {

  public:
    /**
     * Constructor for result object. 
     * @param book
     */
    ResultObject() {}

    // getter:
    bool found_in_metadata() const;
    bool found_in_corpus() const;
    short scope() const;
    double score() const;
    std::map<std::string, short> words_with_scope() const;
    Book* book() const;

    // setter:
    void set_book(Book* book);

    /**
     * Sets found_in_metadata to true and sets initial metadata score.
     * @param init_score
     */
    void NewResult(std::string word, short scope, double score);

    /**
     * Join to result objects.
     * - Update found in metadata/ corpus.
     * - Add score.
     * - (Bitwise) add scopes for each world.
     */
    void Join(ResultObject& result_object);

    void Print(std::string word, std::string preview);
    
    /**
     * Returns a vectror of all searched-words, with the original word, the
     * converted word, and the scope in which the word was found in each entry.
     * @return list of all searched words.
     */
    std::vector<SearchedWordObject> GetSearchWords(std::map<std::string, std::string> converted_to_original_map);
      
  private:
    short scope_;
    double score_; 
    std::map<std::string, short> words_with_scope_;
    Book* book_;
};

#endif
