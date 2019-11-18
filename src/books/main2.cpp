#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

int main()
{
    for (auto& p : fs::recursive_directory_iterator("../../web/books")) {
        std::string name = p.path().filename();
        std::string end = name.substr(name.length()-4, name.length());
        if(name != "ocr.txt" && end == ".txt")
            std::cout << p.path().filename() << std::endl; 
    }
}
