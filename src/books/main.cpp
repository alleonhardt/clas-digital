#include <iostream>
#include <list>
#include "json.hpp"
#include "CBookManager.hpp"
#include "func.hpp"


int main()
{
    CBookManager manager;

    std::string str1;
    std::string str2;

    std::ifstream read("zotero.json");
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

    for(;;)
    {
        std::string sInput;
        std::string sFuzzy;

        std::cout << "> ";
        getline(std::cin, sInput);
        func::convertToLower(sInput);

        if(sInput == "q")
            return 0;

        if(sInput == "show")
        {
            std::map<std::string, CBook> mapBooks = manager.getMapOfBooks();
            for(auto it=mapBooks.begin(); it!=mapBooks.end(); it++)
                std::cout << it->first << ": " << it->second.getMetadata().getShow() << "\n";
             std::cout << "\n";
             continue;
        }

        else if (sInput == "get")
        {
            std::string sKey;
            std::cout << "> ";
            getline(std::cin, sKey);
            std::ofstream write(sKey + ".json");
            write << manager.getMapOfBooks().at(sKey).getMetadata().getMetadata();
            write.close();
            continue;
        }

        std::cout << "Fuzzyness: ";
        getline(std::cin, sFuzzy);
        int fuzzy = std::stoi(sFuzzy);

        
        CSearchOptions* searchOpts = new CSearchOptions(sInput, fuzzy, {}, false, true, "", 0 , 2019);

        std::cout << "Searching for " << sInput << "... \n";

        std::map<std::string, CBook*>* searchResults = manager.search(searchOpts);
        unsigned int counter = 0;
        for(auto it=searchResults->begin(); it!=searchResults->end(); it++)
        {
            std::cout << it->first << ": " << it->second->getMetadata().getShow() << "\n";
            std::cout << "-- Pages: ";

            //Pages for full search
            if(fuzzy == 0)
            {
                std::list<int>* listPages = it->second->getPagesFull(sInput);
                for(auto jt=listPages->begin(); jt!=listPages->end(); jt++)
                    std::cout << (*jt) << ", ";
                delete listPages;
                std::cout << "\n";
            }

            //Pages for contains search
            else if(fuzzy == 1)
            {
                std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesContains(sInput);
                for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                {
                    std::cout << jt->first << " (";
                    for(size_t i=0; i<jt->second.size(); i++)
                        std::cout << jt->second[i] << ", ";
                    std::cout << "), ";
                }
                delete mapPages;
                std::cout << std::endl;
            }

            //Pages for fuzzy search
            else if(fuzzy == 2)
            {
                std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesFuzzy(sInput);
                for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                {
                    std::cout << jt->first << " (";
                    for(size_t i=0; i<jt->second.size(); i++)
                        std::cout << jt->second[i] << ", ";
                    std::cout << "), ";
                }
                delete mapPages;
                std::cout << std::endl;
            }
            counter++;
        }
        std::cout << "Results found: " << counter << "\n";

   }
}
     



