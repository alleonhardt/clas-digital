#include <iostream>
#include "CBookManager.hpp"

int main()
{
    CBookManager manager;

    bool check = manager.initialize();

    if(check == false)
    {
        std::cout << "Failed to initialize.\n";
        return 0;
    }

    std::map<std::string, CBook> mapBooks = manager.getMapOfBooks();

    unsigned int counter = 0;
    for(auto it=mapBooks.begin(); it!= mapBooks.end(); it++)
    {
        std::cout << it->second.getMetadata().getShow() << "\n";
        counter++;
    }
    std::cout << "Books found: " << counter << "\n";
}
     



