#include "CBookManager.hpp"


/**
* @return map of all book
*/
std::map<std::string, CBook>& CBookManager::getMapOfBooks() {
    return m_mapBooks;
}

/**
*@brief load all books.
*/
bool CBookManager::initialize()
{
    //Load directory of all books 
    DIR *dir_allItems;
    struct dirent* e_allItems;
    dir_allItems = opendir("web/books");

    //Check whether files where found
    if(!dir_allItems)
    {
        std::cerr<< "No books found.\n";
        return false;
    }

    //Go though all books and create book
    while((e_allItems = readdir(dir_allItems)) != NULL)
    {
        //Ignore files that aren't books
        if(e_allItems->d_name[0] == '.')
            continue;

        //Create book
        std::string sPath = "web/books/";
        sPath.append(e_allItems->d_name); 
        CBook book (sPath);

        //Add book to map of all books
        m_mapBooks[book.getKey()] = book;
    }

    return true;
}

