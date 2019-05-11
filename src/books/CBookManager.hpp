#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <dirent.h>
#include "CBook.hpp"


class CBookManager
{
private:

    //Map of all books
    std::map<std::string, CBook> m_mapBooks;

public:

   // **** getter **** //

   /**
   * @return map of all book
   */
   std::map<std::string, CBook>& getMapOfBooks();

   /**
   * @brief load all books.
   * @return boolean for successful of not
   */
   bool initialize(); 

}; 



