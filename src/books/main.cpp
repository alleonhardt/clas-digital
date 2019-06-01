#include <iostream>
#include <list>
#include "json.hpp"
#include "CBookManager.hpp"
#include "func.hpp"
#include "src/console/console.hpp"

int main()
{
    CBookManager manager;

    std::string str1;
    std::string str2;

    std::ifstream read("bin/zotero.json");
    nlohmann::json jItems;
    read >> jItems;
    read.close();
    alx::cout.write ("updating zotero.\n");
    manager.updateZotero(jItems);
    bool check = manager.initialize();

    if(check == false)
    {
        alx::cout.write ("Failed to initialize.\n");
        return 0;
    }

    for(;;)
    {
        alx::cout.write ("> ");
        std::string sInput = alx::cout.getCommand();
        func::convertToLower(sInput);

        if(sInput == "q")
            return 0;

        if(sInput == "show")
        {
            std::map<std::string, CBook> mapBooks = manager.getMapOfBooks();
            for(auto it=mapBooks.begin(); it!=mapBooks.end(); it++)
                alx::cout.write (it->first, ": ", it->second.getMetadata().getShow(), "\n");
             alx::cout.write("\n");
             continue;
        }

        else if (sInput == "get")
        {
            alx::cout.write ("> ");
            std::string sKey = alx::cout.getCommand();
            std::ofstream write(sKey + ".json");
            write << manager.getMapOfBooks().at(sKey).getMetadata().getMetadata();
            write.close();
            continue;
        }

        alx::cout.write ("Fuzzyness: ");
        std::string sFuzzy = alx::cout.getCommand();
        int fuzzy = std::stoi(sFuzzy);

        
        CSearchOptions* searchOpts = new CSearchOptions(sInput, fuzzy, {}, false, true, "", 0 , 2019);
        alx::cout.write("Second word: ");
        std::string sSecondWord = alx::cout.getCommand();
        if(sSecondWord.length() > 0)
            searchOpts->setSecondWord(sSecondWord);

        alx::cout.write ("Searching for ", sInput, "... \n");

        std::map<std::string, CBook*>* searchResults = manager.search(searchOpts);
        unsigned int counter = 0;
        for(auto it=searchResults->begin(); it!=searchResults->end(); it++)
        {
            alx::cout.write (it->first, ": ", it->second->getMetadata().getShow(), "\n");
            alx::cout.write ("-- Pages: ");

            //Pages for full search
            if(fuzzy == 0)
            {
                std::list<int>* listPages = it->second->getPagesFull(sInput);
                for(auto jt=listPages->begin(); jt!=listPages->end(); jt++)
                    alx::cout.write ((*jt), ", ");
                delete listPages;
                alx::cout.write ("\n");

                //If second word exists, print pages, where second word has been found
                if(searchOpts->getSecondWord().length() > 0)
                {
                    alx::cout.write("-- Pages: ");
                    std::list<int>* listPages = it->second->getPagesFull(searchOpts->getSecondWord());
                    for(auto jt=listPages->begin(); jt!=listPages->end(); jt++)
                        alx::cout.write ((*jt), ", ");
                    delete listPages;
                    alx::cout.write ("\n");
                }
            }

            //Pages for contains search
            else if(fuzzy == 1)
            {
                std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesContains(sInput);
                for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                {
                    alx::cout.write (jt->first, " (");
                    for(size_t i=0; i<jt->second.size(); i++)
                        alx::cout.write (jt->second[i], ", ");
                    alx::cout.write ("), ");
                }
                delete mapPages;
                alx::cout.write ("\n");

                //If second word exists, print pages, where second word has been found
                if(searchOpts->getSecondWord().length() > 0)
                {

                    std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesContains(searchOpts->getSecondWord());
                    for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                    {
                        alx::cout.write (jt->first, " (");
                        for(size_t i=0; i<jt->second.size(); i++)
                            alx::cout.write (jt->second[i], ", ");
                        alx::cout.write ("), ");
                    }
                    delete mapPages;
                    alx::cout.write ("\n");
                }
            }

            //Pages for fuzzy search
            else if(fuzzy == 2)
            {
                std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesFuzzy(sInput);
                for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                {
                    alx::cout.write (jt->first, " (");
                    for(size_t i=0; i<jt->second.size(); i++)
                        alx::cout.write (jt->second[i], ", ");
                    alx::cout.write ("), ");
                }
                delete mapPages;
                alx::cout.write ("\n");

                //If second word exists, print pages, where second word has been found
                if(searchOpts->getSecondWord().length() > 0)
                {
                    alx::cout.write("-- Pages: ");
                    std::map<int, std::vector<std::string>>* mapPages = it->second->getPagesFuzzy(searchOpts->getSecondWord());
                    for(auto jt=mapPages->begin(); jt!=mapPages->end(); jt++)
                    {
                        alx::cout.write (jt->first, " (");
                        for(size_t i=0; i<jt->second.size(); i++)
                            alx::cout.write (jt->second[i], ", ");
                        alx::cout.write ("), ");
                    }
                    delete mapPages;
                    alx::cout.write ("\n");
                }
            }
            counter++;
        }
        alx::cout.write("Results found: ", (int)counter, "\n");

   }
}
     



