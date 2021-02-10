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
    ResultObject(Book* book, bool found_in_corpus, double init_score);
    ResultObject() {}

    // getter:
    bool found_in_metadata() const;
    bool found_in_corpus() const;
    double metadata_score() const;
    double corpus_score() const;
    Book* book() const;

    /**
     * Sets found_in_metadata to true and sets initial metadata score.
     * @param init_score
     */
    void FoundInMetadataSetInitScore(double init_score);

    /**
     * Sets found_in_corpus to true and sets initial corpus score.
     * @param init_score
     */
    void FoundInCorpusSetInitScore(double init_score);

    /**
     * Increses metadata score by given value
     * @param inc value to increase score by.
     */
    void IncreaseMetadataScore(double inc);

    /**
     * Increses corpus score by given value
     * @param inc value to increase score by.
     */
    void IncreaseCorpusScore(double inc);

    /**
     * Returns a combined score. 
     * @return metadata_score+10*corpus_score.
     */
    double GetOverallScore() const;

    void Print(std::string word="") {
      std::cout << "--- " << book_->key() << " --- " << std::endl;  
      std::cout << "Found metadata: " << found_in_metadata_ << std::endl;
      std::cout << "Found corpus: " << found_in_corpus_ << std::endl;
      std::cout << "Metadata core: " << metadata_score_ << std::endl;
      std::cout << "Corpus score: " << corpus_score_ << std::endl;
      std::cout << "Overall score: " << GetOverallScore() << std::endl;
      
      if (word != "") {
        if (book_->map_words_pages().count(word) > 0) {
          std::cout << "Direct match: true" << std::endl;
        }
        else if (book_->corpus_fuzzy_matches().count(word) > 0) {
          std::cout << "Fuzzy match (corpus): " 
            << book_->corpus_fuzzy_matches()[word].GetBestMatch() << std::endl;
        }
        else if (book_->metadata_fuzzy_matches().count(word) > 0) {
          std::cout << "Fuzzy match (metadata): " 
            << book_->metadata_fuzzy_matches()[word].GetBestMatch() << std::endl;
        }
        else {
          std::cout << "Something went wrong: no match could be tracked. "
            "(Or direct match in title/ author)" << std::endl;
        }
      }
      std::cout << std::endl;
    }

  private:
    bool found_in_metadata_;
    bool found_in_corpus_;
    double metadata_score_; 
    double corpus_score_; 
    Book* book_;
};

#endif
