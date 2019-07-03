#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::experimental::filesystem;

int main()
{
	std::string pathFrom = "../Menard2/install/root/books";
	if(fs::exists(pathFrom) && fs::is_directory(pathFrom))
	{
		for(const auto &dirEntry : fs::directory_iterator(pathFrom))
		{
			std::string newPath = dirEntry.path();
			newPath+="/ocr.txt";
			if(fs::exists(newPath)&&!fs::is_directory(newPath))
			{
				std::string to = "web/books/";
				to+=dirEntry.path().filename();
				std::string command = "cp -r ";
				command+=dirEntry.path();
				command+=" ";
				command+=to;
				std::cout<<command<<std::endl;
				system(command.c_str());
			}
		}
	}
}
