#include <iostream>
#include <list>
#include "CBookManager.hpp"
#include "CFunctions.hpp"


int main()
{
    CFunctions function;

    CBookManager manager;
    bool check = manager.initialize();

    if(check == false)
    {
        std::cout << "Failed to initialize.\n";
        return 0;
    }

    std::string sInput;
    std::cout << "> ";
    getline(std::cin, sInput);
    function.convertToLower(sInput);

    std::cout << "Searching for " << sInput << "... \n";

    std::map<std::string, CBook*>* searchResults = manager.search(sInput, false, false);
    
    unsigned int counter = 0;
    for(auto it=searchResults->begin(); it!=searchResults->end(); it++)
    {
        std::cout << it->first << ": " << it->second->getMetadata().getShow() << "\n";
        counter++;
    }
    std::cout << "Results found: " << counter << "\n";
}
     



