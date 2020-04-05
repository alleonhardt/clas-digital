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
		
		void CreateMetadataPage()
		{
            		std::cout << "CREATE WEBPAGE STATIC!!!"<<std::endl;
			auto itemType =m_info["data"].value("itemType","");
			auto title=m_info["data"].value("title","");
			auto isbn = m_info["data"].value("isbn","");
			nlohmann::json info;
			info["bib"] = m_info["bib"];
			info["hasContent"] = m_book->hasContent();
			info["itemType"] = itemType;
			info["title"] = title;
			info["authors"] = nlohmann::json::array();
                	for(const auto &it : m_book->getMetadata().getAuthors())
				info["authors"].push_back(it);
			info["hasISBN"] = isbn != "";
			info["isbn"] = isbn;
			info["place"] = m_info["data"].value("place","");
			info["date"] = m_info["data"].value("date","");
			info["key"] = m_book->getKey();


			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/metadata_template.html");
			std::string result = env.render(temp, info);
			std::ofstream ofs("web/books/"+m_book->getKey()+"/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
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
				content+="<link rel=\"stylesheet\" href=\"/static/pages.css\">";
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
				CreateMetadataPage();
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
			js["pages"].push_back("books");
			js["pages"].push_back("authors");
			js["pages"].push_back("collections");
			js["pages"].push_back("years");
			js["world"] = "hallo";
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}

		void CreateCatlogueYears(CBookManager &mng)
		{
			nlohmann::json rend;
			std::map<std::string,std::pair<int,std::list<CBook*>>> _map;
			for(auto &book : mng.getMapOfBooks())
			{
				std::string name = std::to_string(book.second->getDate());
				if(name=="-1")
					name="unknown";

				if(_map.count(name) > 0 )
					_map[name].first++;
				else
					_map[name].first=1;
				_map[name].second.push_back(book.second);
			}
			for(auto &x : _map)
			{
				nlohmann::json ks;
				ks["year"] = x.first;
				ks["count"] = x.second.first;
				rend["years"].push_back(ks);
			}
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/years/template.html");
			std::string result = env.render(temp, rend);
			std::ofstream ofs("web/catalogue/years/index.html",std::ios::out);
			ofs<<result;
			ofs.close();

			inja::Environment env2;
			inja::Template temp2 = env2.parse_template("web/catalogue/years/template_year.html");
			for(auto &x : _map)
			{
				std::error_code ec;
				std::filesystem::create_directory("web/catalogue/years/"+x.first,ec);
				nlohmann::json js;
				for(auto y : x.second.second)
				{
					nlohmann::json js_k;
					js_k["key"] = y->getMetadata().getMetadata()["data"]["key"];
					js_k["title"] = y->getMetadata().getShow2();
					js_k["bib"] = y->getMetadata().getMetadata()["bib"];
					js["books"].push_back(std::move(js_k));
				}
				js["year"] = x.first;

				std::string result = env2.render(temp2, js);
				std::ofstream ofs("web/catalogue/years/"+x.first+"/index.html",std::ios::out);
				ofs<<result;
				ofs.close();
			}
		}

		void CreateCatalogueBooks(CBookManager &mng)
		{
			nlohmann::json books;
            std::vector<nlohmann::json> vBooks;
			for(auto &it : mng.getMapOfBooks())
			{
				nlohmann::json entry;
				entry["key"] = it.second->getMetadata().getMetadata()["data"]["key"];
				entry["title"] = it.second->getMetadata().getShow2();
				entry["bib"] = it.second->getMetadata().getMetadata()["bib"];
				entry["has_ocr"] = it.second->hasOcr();
            
				vBooks.push_back(std::move(entry));
			}

            std::sort(vBooks.begin(), vBooks.end(), [](nlohmann::json i, nlohmann::json j){ return i["title"]<j["title"];});

            books["books"] = vBooks;
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/books/template.html");
			std::string result = env.render(temp, books);
			std::ofstream ofs("web/catalogue/books/index.html",std::ios::out);
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
		auto origName = lastName;
		auto origFam = firstName;
		func::convertToLower(lastName);
		func::convertToLower(firstName);
                mapAuthors[lastName + "_" + firstName] = {
                            {"id", firstName + "-" + lastName},
                            {"show", origName + ", " + origFam},
                            {"num", manager.getMapofAuthors()[func::returnToLower(lastName)].size()} };
            }
            	for(auto &it : mapAuthors)
	    	{
			if(it.first == "_")
				continue;
                	js["authors"].push_back(it.second);
		}
            

			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/authors/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/authors/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}

        void CreateCatalogueAuthor(CBookManager& manager)
        {
            inja::Environment env;
            inja::Template temp = env.parse_template("web/catalogue/authors/template_author.html");

            for(auto &it : manager.getMapOfBooks()) {
                std::string lastName = it.second->getMetadata().getAuthor();
                std::string firstName = it.second->getMetadata().getMetadata("firstName", "data", "creators", 0);

                std::string name = lastName + ", " + firstName;
		        func::convertToLower(lastName);
		        func::convertToLower(firstName);
                std::string key = firstName + "-" + lastName;

                if(lastName.size() == 0)
                    continue;

                //Create directory
                std::error_code ec;
                std::filesystem::create_directory("web/catalogue/authors/"+key+"/", ec);

                nlohmann::json js;
                js["author"] = {{"name", name}, {"id", key}};

                //Create json with all books
                for(auto &jt : manager.getMapofAuthors()[lastName]) {
                    js["books"].push_back({ 
                        {"id", jt.first},
                        { "show", manager.getMapOfBooks()[jt.first]->getMetadata().getMetadata("bib")}
                                          });
                }

                std::string result = env.render(temp, js);
                std::ofstream ofs("web/catalogue/authors/"+key+"/index.html",std::ios::out);
                ofs<<result;
                ofs.close();
            }
        }
    
        void CreateCatalogueCollections(nlohmann::json pillars)
        {
			nlohmann::json js;
            for(auto &it : pillars)
                js["pillars"].push_back(it);
            
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/collections/template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/catalogue/collections/index.html",std::ios::out);
			ofs<<result;
			ofs.close();
		}

        void CreateCatalogueCollection(CBookManager& manager, nlohmann::json pillars)
        {
            inja::Environment env;
            inja::Template temp = env.parse_template("web/catalogue/collections/template_collection.html");
            for(auto& it : pillars)
            {
                nlohmann::json js;
                js["pillar"] = it;
                std::string key = it["key"];

                //Create directory
                std::error_code ec;
                std::filesystem::create_directory("web/catalogue/collections/"+key+"/", ec);

                //Create books for this collection
                for(auto &jt : manager.getMapOfBooks()) {
                    std::vector<std::string> collections = jt.second->getMetadata().getCollections();

                    if(collections.size() == 0 || func::in(key, collections) == false)
                        continue;

                    js["books"].push_back({ 
                        {"id", jt.first},
                        { "show", jt.second->getMetadata().getShow2()} });
                }

                std::string result = env.render(temp, js);
                std::ofstream ofs("web/catalogue/collections/"+key+"/index.html",std::ios::out);
                ofs<<result;
                ofs.close();
            }
        }
};
