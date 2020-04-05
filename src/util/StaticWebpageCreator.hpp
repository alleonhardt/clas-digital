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

		void CreatePagesPage()
		{
			nlohmann::json js;
			js["key"] = m_book->getKey();
			js["title"] = m_book->getMetadata().getShow2();
			js["bib"] = m_info["bib"];
			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/pages_template.html");
			std::string result = env.render(temp, js);
			std::ofstream ofs("web/books/"+m_book->getKey()+"/pages/index.html",std::ios::out);
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
				CreatePagesPage();
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
                std::vector<nlohmann::json> vBooks;
				for(auto y : x.second.second)
				{
					nlohmann::json js_k;
					js_k["key"] = y->getMetadata().getMetadata()["data"]["key"];
					js_k["title"] = y->getMetadata().getShow2();
					js_k["bib"] = y->getMetadata().getMetadata()["bib"];
					vBooks.push_back(std::move(js_k));
				}

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);
                js["books"] = vBooks;
				js["year"] = x.first;

				std::string result = env2.render(temp2, js);
				std::ofstream ofs("web/catalogue/years/"+x.first+"/index.html",std::ios::out);
				ofs<<result;
				ofs.close();
			}
		}

		void CreateCatalogueBooks(CBookManager &mng)
		{
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

            std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

			nlohmann::json books;
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
                std::vector<nlohmann::json> vBooks;
                for(auto &jt : manager.getMapofAuthors()[lastName]) {
                    vBooks.push_back({ 
                        {"id", jt.first},
                        { "title", manager.getMapOfBooks()[jt.first]->getMetadata().getShow2()},
                        { "bib", manager.getMapOfBooks()[jt.first]->getMetadata().getMetadata("bib")}
                                          });
                }

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

                js["books"] = vBooks;
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
                std::vector<nlohmann::json> vBooks;
                for(auto &jt : manager.getMapOfBooks()) {
                    std::vector<std::string> collections = jt.second->getMetadata().getCollections();

                    if(collections.size() == 0 || func::in(key, collections) == false)
                        continue;

                    vBooks.push_back({ 
                        {"id", jt.first},
                        { "title", jt.second->getMetadata().getShow2()},
                        { "bib", jt.second->getMetadata().getMetadata("bib")} });
                }

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

                js["books"] = vBooks;
                std::string result = env.render(temp, js);
                std::ofstream ofs("web/catalogue/collections/"+key+"/index.html",std::ios::out);
                ofs<<result;
                ofs.close();
            }
        }

        static bool mySort(nlohmann::json i, nlohmann::json j) {
            std::string str1 = i["title"];
            std::string str2 = j["title"];
            return func::returnToLower(str1)<func::returnToLower(str2);
        }
};
