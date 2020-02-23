#include "books/CBookManager.hpp"
#include "json.hpp"
#include <filesystem>
#include <inja/inja.hpp>

class StaticWebpageCreator
{
	private:
		std::string m_path;
		std::string m_bookID;
		nlohmann::json m_info;
		CBook *m_book;
	public:
		StaticWebpageCreator(CBook *book)
		{
			m_path = book->getPath();
			m_bookID = book->getKey();
			m_info = book->getMetadata().getMetadata();
			m_book = book;
		}

		bool createWebpage()
		{
			std::string webpath = "/books/"+m_bookID+"/pages";
			std::error_code ec;
			
			//Remove the old folder structure
			//std::filesystem::remove_all(m_path+"/view",ec);
			//std::filesystem::remove_all(m_path+"/meta",ec);
			if(std::filesystem::exists(m_path+"/readerInfo.json"))
			{
				//Full blown webpage
				std::string content="<!DOCTYPE html><html><head><title>";
				content+=m_book->getMetadata().getShow2();
				content+="</title><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">";
				content+="<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css\" integrity=\"sha384-WskhaSGFgHYWDcbwN70/dfYBj47jz9qbsMId/iRN3ewGhXQFZCSftd1LZCfmhktB\" crossorigin=\"anonymous\">";
				content+="<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">";
				content+="<link rel=\"canonical\" href=\"https://www.clas-digital.uni-frankfurt.de/books/";
				content+=m_book->getKey();
				content+="/pages\" />\n";
				content+="<link rel=\"up\" href=\"https://www.clas-digital.uni-frankfurt.de/books/";
				content+=m_book->getKey();
				content+="\" />\n";
				content+="<link rel=\"top\" href=\"https://www.clas-digital.uni-frankfurt.de/\"/>\n";
				content+="<link rel=\"shortcut icon\" href=\"/static/fav/hand-top-right-16+32+48.ico\"/>";
				content+="<link rel=\"stylesheet\" href=\"/GetBooks.css\">";
				content+="<script>let gGlobalBookId=\"";
				content+=m_book->getKey();
				content+="\";</script><script src=\"/general.js\"></script><script src=\"/GetBooks.js\"></script></head><body><nav class='searchbox'>\n<a class=\"about\" href=\"../\" rel=\"index up\">About this book</a>\n<center><input id='srchbox' style='width:25%;border-radius: 20px;padding-left: 0.5rem;padding-right: 0.5rem;margin-left: 1rem;min-width: 10rem;margin-top: 0.5rem;text-align: center;' type='text' placeholder=\"search this book\"><i class=\"fa fa-search searchstyler\" onclick='doCompleteNewSearch();return true;'></i></input><img id='fullbut' onclick='tooglefullscreen(this);' class='fullscreen' title='Toogle fullscreen mode' style='float: right; margin:0.5rem' src=\"/static/GetBooks/fullscreen-24px.svg\"/><img id='tooglebut' onclick=\"read_mode(this);\" class='fullscreen' style='float: right;margin:0.5rem;' title=\"Toggle image reader and split reader mode\" src=\"/static/GetBooks/chrome_reader_mode-24px.svg\"/><div style='position: relative;width:90%;height: 1px;'><div id='fuzzysuggestions' style='visibility:hidden;overflow-y:scroll;z-index: 5;position: absolute;top:0;left: 0;right:0;margin-left: auto;margin-right:auto;max-height:80vh;background:white;border: 1px solid black;'></div></div></center><center><span id=\"bibliography\" class=\"bibliostyle\">";
				content+=m_info["bib"];
				content+="</span></center><div class='linknav'><div class='lastbutcont'><img class='lastbut' onclick='SelectLastHit();' src='/next.svg'/></div><div id=\"fullsearchhitlist\"></div><div class='nextbutcont'><img class='nextbut' onclick='SelectNextHit();' src='/last.svg'/></div></div></nav></body></html>";
				
				std::string pathcopy = m_path;
				pathcopy+="/pages";
				std::filesystem::create_directory(pathcopy,ec);
				
				//MOVE all jps from web/books/BOOKID/*.jpg to web/books/BOOKID/pages/*.jpg
			
				for(auto &it : std::filesystem::directory_iterator(m_path))
				{
					auto pos = it.path().string().find(".jpg");

					if(pos!=std::string::npos && pos == it.path().string().length()-4)
					{
						std::filesystem::rename(it.path(),m_path+"/pages/"+it.path().filename().string(),ec);
						if(ec)
						{
							std::cout<<"Could not rename "<<it.path()<<std::endl;
							break;
						}
					}
				}

				
				pathcopy+="/index.html";

				std::ofstream ofs(pathcopy.c_str(), std::ios::out);
				ofs<<content;
				ofs.close();

			}
			

			{
				auto itemType =m_info["data"].value("itemType","");
				auto title=m_info["data"].value("title","");
				auto isbn = m_info["data"].value("isbn","");
				
				//Metadata page only
				std::string content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\"/>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">\n<link rel=\"shortcut icon\" href=\"/static/fav/hand-top-right-16+32+48.ico\"/><title>";
				content+=m_book->getMetadata().getShow2();
				content+="</title>\n";
				content+="<link rel=\"canonical\" href=\"https://www.clas-digital.uni-frankfurt.de/books/";
				content+=m_book->getKey();
                                content+="\"/>\n";
                                content+="</head>\n<body style=\"padding: 5rem;\"><h1>\n";
				content+=m_info["bib"];
				content+="</h1><nav id='booklcontentlink'>\n<ul>\n";
				if(m_book->getHasFiles()) {
				content+="<li><a href=\"";
				content+=webpath;
				content+="\" rel=\"search\">Pages in this book</a></li>\n";
				}
				content+="<li><a href=\"/books/\">Other books in catalogue</a></li>\n";
				content+="</ul>\n</nav>\n";
				content+="<p id=\"itemType\"><b>Item Type:</b> ";
			       	content+=itemType;
				content+="</p><hr><p id=\"title\"><b>Title:</b> ";
			       	content+=title;
				content+="</p><hr><p id='author'><b>Author:</b> ";
				content+=m_book->getAuthor();
				content+="</p><hr><p id=\"place\"><b>Place:</b> ";
				content+=m_info["data"].value("place","");
				content+="</p><hr><p id=\"date\"><b>Date:</b> ";
				content+=m_info["data"].value("date","");
				content+="</p><hr>";
				if(isbn!="")
				{
					content+="<p id=\"isbn\"><b>ISBN:</b> ";
					content+=isbn;
					content+="</p><hr>";
				}
				content+="</body></html>";
				std::string pathcopy = m_path;
				pathcopy+="/index.html";

				std::ofstream ofs(pathcopy.c_str(), std::ios::out);
				ofs<<content;
				ofs.close();
			}
			return true;
		}
};

class StaticCatalogueCreator
{
	public:
		void CreateCatalogue()
		{
			nlohmann::json js;
			for(auto &it : std::filesystem::directory_iterator("web/catalogue"))
			{
				if(it.is_directory())
				{
					js["pages"].push_back(it.path().filename().string());
				}
			}
			js["world"] = "hallo";
			std::cout<<js.dump()<<std::endl;
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}

        void CreateCatalogueAuthors(CBookManager& manager)
		{
			nlohmann::json js;
            std::map<std::string, nlohmann::json> mapAuthors;
            for(auto &it : manager.getMapOfBooks()) {
                std::string lastName = it.second->getMetadata().getAuthor();
                std::string firstName = it.second->getMetadata().getMetadata("firstName", "data", "creators", 0);
                mapAuthors[lastName + "_" + firstName] = {
                            {"id", lastName + "_" + firstName},
                            {"show", lastName + ", " + firstName},
                            {"num", manager.getMapofAuthors()[func::returnToLower(lastName)].size()} };
            }
            for(auto &it : mapAuthors)
                js["authors"].push_back(it.second);
            

			std::cout<<js.dump()<<std::endl;
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/authors/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/authors/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}
    
        void CreateCatalogueCollections(nlohmann::json pillars)
        {
			nlohmann::json js;
            for(auto &it : pillars)
                js["pillars"].push_back(it);
            
			std::cout<<js.dump()<<std::endl;
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/collections/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/collections/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}
};
