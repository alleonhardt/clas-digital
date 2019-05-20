#include <iostream>
#include <list>
#include "json.hpp"
#include "CBookManager.hpp"
#include "func.hpp"


int main()
{
    CBookManager manager;

    std::ifstream read("Items.json");
    nlohmann::json jItems;
    read >> jItems;
    read.close();
    std::cout << "updating zotero.\n";
    manager.updateZotero(jItems);
    bool check = manager.initialize();

    if(check == false)
    {
        std::cout << "Failed to initialize.\n";
        return 0;
    }

    std::string sInput;
    std::cout << "> ";
    getline(std::cin, sInput);
    func::convertToLower(sInput);

    std::cout << "Searching for " << sInput << "... \n";

    std::map<std::string, CBook*>* searchResults = manager.search(sInput, true, false);
    
    unsigned int counter = 0;
    for(auto it=searchResults->begin(); it!=searchResults->end(); it++)
    {
        std::cout << it->first << ": " << it->second->getMetadata().getShow() << "\n";
        std::cout << "Pages: ";
        std::list<int>* listPages = it->second->getPages(sInput);
        for(auto jt=listPages->begin(); jt!=listPages->end(); jt++)
            std::cout << (*jt) << ", ";
        std::cout << "\n";
        counter++;
    }
    std::cout << "Results found: " << counter << "\n";
}
     



