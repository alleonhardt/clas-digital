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
    std::map<std::string, CBook*> m_mapBooks;

public:

   /**
   *@brief load all books.
   */
   void initialize(); 

}; 



