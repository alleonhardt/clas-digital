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
#include "searched_word_object.h"
class ResultObject {

  public:
    /**
     * Constructor for result object. 
     * @param book
     */
    ResultObject() : scope_(0), score_(0) {}

    // getter:
    bool found_in_metadata() const;
    bool found_in_corpus() const;
    short scope() const;
    double score() const;
    std::map<std::string, FoundWordsObject> matches() const;
    std::vector<FoundWordsObject> matches_as_list() const;
    Book* book() const;

    // setter:
    void set_score(double score);
    void set_book(Book* book);
    void set_original_words(std::map<std::string, std::string> converted_to_original_map);

    /**
     * Sets found_in_metadata to true and sets initial metadata score.
     * @param init_score
     */
    void NewResult(std::string searched_word, std::string found_word, short scope, double score, double rel);
    void NewResult(std::string searched_word, std::string found_word, short scope, double score, double rel, int word_count);

    /**
     * Join to result objects.
     * - Update found in metadata/ corpus.
     * - Add score.
     * - (Bitwise) add scopes for each world.
     */
    void Join(ResultObject& result_object);

    void Print(std::string word, std::string preview);
    
  private:
    short scope_;
    double score_; 
    std::map<std::string, FoundWordsObject> matches_;
    Book* book_;
};

#endif
