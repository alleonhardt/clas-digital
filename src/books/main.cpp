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

    /*
    for(;;)
    {
        std::cout << "> ";
        getline(std::cin, str1);
        std::cout << "> ";
        getline(std::cin, str2);
        size_t ld = fuzzy::levenshteinDistance(str1.c_str(), str2.c_str()); 
        int rd = fuzzy::recursiveLD(str1.c_str(), str1.length(), str2.c_str(), str2.length());
        std::cout << "LD: " << ld << "\n";
        double score = static_cast<double>(ld)/std::max(str1.length(), str2.length());
        double score2 = static_cast<double>(rd)/std::max(str1.length(), str2.length());
        std::cout << "Score: " << score << "\n";
        std::cout << "Score: " << score2 << "\n";
        if(str1 == "q")
            break;
    }
    */

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
            /*
            std::cout << "-- Pages: ";
            std::list<int>* listPages = it->second->getPages(sInput, fuzzy);
            for(auto jt=listPages->begin(); jt!=listPages->end(); jt++)
                std::cout << (*jt) << ", ";
            std::cout << "\n";
            */
            counter++;
        }
        std::cout << "Results found: " << counter << "\n";

   }
}
     



