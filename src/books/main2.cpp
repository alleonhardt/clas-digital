#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    for (auto& p : fs::directory_iterator("web/books")) {

        if(p.path().filename() == "whatever.txt")
            continue;

        fs::create_directory(p.path()/"intern");

        for(auto& p2 : fs::directory_iterator(p.path())) {
            std::string name = p2.path().filename();
            std::string end = name.substr(name.length()-4, name.length());
            if(name != "ocr.txt" && end == ".txt") {

                std::string to = "intern/"+name;
                if(name == "pages_new.txt")
                    to="intern/pages.txt";
                    
                fs::rename(p2, p.path()/to);
                std::cout << p2.path() << std::endl; 
            }
        }
    }
}
