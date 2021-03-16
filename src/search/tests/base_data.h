#ifndef SRC_SEARCH_TESTS_BASEDATA_H_
#define SRC_SEARCH_TESTS_BASEDATA_H_

#include "book_manager/book_manager.h"
#include "gramma.h"

class BaseData {
  public:

    // getter:
    static BaseData *instance(std::string books, std::string metadata) {
      if (!instance_)
        instance_ = new BaseData(books, metadata);
      return instance_; 
    }

    static BaseData *instance2(std::string books, std::string metadata) {
      if (!instance2_)
        instance2_ = new BaseData(books, metadata);
      return instance2_; 
    }

    Dict& dict() { return dict_; }

    BookManager& book_manager() { return book_manager_; }

    std::string book_key() { return book_key_; }

  private:
    static BaseData* instance_;
    static BaseData* instance2_;
    Dict dict_;
    BookManager book_manager_;
    const std::string book_key_;

    BaseData(std::string books, std::string metadata) 
      : dict_("web/dict.json"), book_manager_({"src/search/tests/" + books}, &dict_), book_key_("UEHEJINT") {
    
      // Give book access to dictionary.
      
      Book::set_dict(&dict_);
      // Initialize book-manager.
      nlohmann::json j_metadata = func::LoadJsonFromDisc("src/search/tests/" + metadata + ".json");
      book_manager_.CreaeItemsFromMetadata(j_metadata);
      book_manager_.Initialize(true);
    }
};

#endif
