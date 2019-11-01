#include <iostream>
#include <list>
#include <ctime>
#include "json.hpp"
#include "CBookManager.hpp"
#include "CBook.hpp"
#include "func.hpp"

int main()
{
    /*
    std::ifstream read("web/books/XUA36SSS/ocr.txt");
    std::string str((std::istreambuf_iterator<char>(read)), std::istreambuf_iterator<char>());

    std::ofstream write("test.txt");
    for(auto it : func::extractWordsFromString(str))
        write << it.first << "\n";

    */
    CBookManager manager;

    std::ifstream read("bin/zotero.json");
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
        std::cout << "> ";
        std::string sInput;
        getline(std::cin, sInput);
        func::convertToLower(sInput);
        sInput = func::convertStr(sInput);

        if(sInput == "q")
            return 0;


        /*
        struct timespec start, finish;
        double elapsed;
        clock_gettime(CLOCK_MONOTONIC, &start);
        std::list<std::string>* l_sugg = manager.getSuggestions_acc(sInput);
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        std::cout << "Results in: " << elapsed << std::endl;
        for(auto it=l_sugg->begin(); it!=l_sugg->end();it++)
            std::cout << (*it) << std::endl;
        delete l_sugg;
        std::cout << std::endl;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        l_sugg = manager.getSuggestions_acc2(sInput, false, true);
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
        std::cout << "Results in: " << elapsed << std::endl;
        for(auto it=l_sugg->begin(); it!=l_sugg->end();it++)
            std::cout << (*it) << std::endl;
        delete l_sugg;
        */

        std::cout << "Fuzzyness: ";
        std::string sFuzzy;
        getline(std::cin, sFuzzy);
        bool fuzzy;
        if(sFuzzy == "true")
            fuzzy = true;
        else
            fuzzy = false;

        CSearchOptions* searchOpts = new CSearchOptions(sInput, fuzzy, {"RFWJC42V", "XCFFDRQC", "RBB8DW5B", "WIXP3DS3"}, false, true, "", 0 , 2019, 1, 2);

        sInput = searchOpts->getSearchedWord();

        std::cout << "\nSearching for " << sInput << "... \n";

        std::list<std::string>* searchResults = manager.search(searchOpts);
        unsigned int counter = 0;
        for(auto it=searchResults->begin(); it!=searchResults->end(); it++)
        {
            CBook* book = manager.getMapOfBooks()[(*it)];
            std::cout << "\033[1;32m" << book->getKey() << ": " << book->getShow() << "\n";
            std::cout << "Preview: " << std::endl;
            std::cout << "\033[1;31m" << book->getPreview(sInput)<< "\n";

            // *** Print Pages *** //
            std::cout << "\033[1;33m Pages: ";

            std::map<int, std::vector<std::string>>* mapPages = book->getPages(sInput, fuzzy);
            for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
            {
                std::cout << jt->first;

                //If there are no found words of this page (f.e. full-search) -> continue)
                if(jt->second.size() == 0)
                {
                    std::cout << ", ";
                    continue;
                }

                //Print words on this page
                std::cout << " (";
                for(size_t i=0; i<jt->second.size(); i++)
                    std::cout << jt->second[i] << ", ";
                std::cout << "\b\b), ";
            }

            //Delete results
            delete mapPages;
            std::cout << "\b\b\n\033[0m";

            counter++;
        }
        std::cout << "Results found: " << (int)counter << "\n";
   }
}
     

