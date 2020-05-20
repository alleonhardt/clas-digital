#include "books/CBookManager.hpp"
#include "json.hpp"
#include <filesystem>
#include <inja/inja.hpp>
#include <cstdio>
#include <chrono>
#include <ctime>

static void atomic_write_file(std::string filename, const std::string &data)
{
	std::cout<<"Filename : "<<filename<<std::endl;
	std::ofstream ofs("main.tmp",std::ios::out);
	ofs<<data;
	ofs.close();
	std::error_code ec;
	std::filesystem::rename("main.tmp",filename.c_str(),ec);
	if( ec )
	{
		std::cout<<"Error code, rename : "<<ec<<std::endl;
		throw 0;
	}
}

class StaticWebpageCreator
{
	private:
		std::string m_path;
		std::string m_bookID;
		nlohmann::json m_info;
        nlohmann::json m_topnav;
		CBook *m_book;
	public:
        StaticWebpageCreator()
        {
            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
            auto time=std::chrono::system_clock::now();
            std::time_t time_time = std::chrono::system_clock::to_time_t(time);
            m_topnav["time"] = std::ctime(&time_time);    
        }

		StaticWebpageCreator(CBook *book)
		{
			m_path = book->getPath();
			m_bookID = book->getKey();
			m_info = book->getMetadata().getMetadata();
			m_book = book;

            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
            auto time=std::chrono::system_clock::now();
            std::time_t time_time = std::chrono::system_clock::to_time_t(time);
            m_topnav["time"] = std::ctime(&time_time);   
		}

		template<typename T>
		std::string general_applier(const std::string &esc, char character, T t1)
		{
			std::string newstr;
			for(unsigned int i = 0; i < esc.length(); i++)
				if( esc[i] == character )
				{
					newstr+=t1(character);
				}
				else
					newstr+=esc[i];
			return newstr;
		}
		std::string escaper(const std::string &esc,char character)
		{
			return general_applier(esc,character,[](char ch){return std::string("\\")+ch;});
		}

		std::string cutter(const std::string &esc, char character)
		{
			return general_applier(esc,character,[](char ){return "";});
		}

		std::string replacer(const std::string &esc, char character,std::string seq)
		{
			return general_applier(esc,character,[&seq](char){return seq;});
		}
		
		std::string escape_characters(std::string str)
		{
			return replacer(replacer(replacer(replacer(str,'<',"&lt;"),'>',"&gt;"),'"',"&quot;"),'\'',"&apos;");
		}

        void createSearchPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["search"] = "class='dropdown-banner active'";

            inja::Template temp = env.parse_template("web/search_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/Search.html", result);
        }

        void createInformationPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["information"] = "class='dropdown-banner active'";

            inja::Template temp = env.parse_template("web/information/information_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/information/index.de.html", result);
        }

        void createAdminPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["admin"] = "classifiedContent active";

            inja::Template temp = env.parse_template("web/private/admin/administration_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/private/admin/Administration.html", result);
        }

        void createUploadPage()
        {
            inja::Environment env;
            nlohmann::json j;
            j["topnav"]=m_topnav;
            j["topnav"]["upload"] = "classifiedContent active";

            inja::Template temp = env.parse_template("web/private/write/uploadBook_template.html");
            std::string result = env.render(temp, j);  
            atomic_write_file("web/private/write/UploadBook.html", result);
        }

		void CreateMetadataPage()
		{
			std::cout<<"Processing: "<<m_book->getKey()<<std::endl;
			auto itemType =m_info["data"].value("itemType","");
			auto title=m_info["data"].value("title","");
			auto isbn = m_info["data"].value("isbn","");
			nlohmann::json info;
            		info["show"] = m_book->getMetadata().getShow2();
			info["bib"] = m_info["bib"].get<std::string>();
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

            //Parse navbar
            info["topnav"] = m_topnav;
            info["topnav"]["catalogue"] = "class='dropdown-banner active";

			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/metadata_template.html");
			std::string result = env.render(temp, info);

			atomic_write_file("web/books/"+m_book->getKey()+"/index.html",result);
		}

		void CreatePagesPage()
		{
			nlohmann::json js;
			js["key"] = m_book->getKey();
			js["title"] = m_book->getMetadata().getShow2();
			js["bib"] = m_info["bib"].get<std::string>();
			js["bib_esc"] = cutter(replacer(m_info["bib"].get<std::string>(),'"',"&quot;"),'\n');
			inja::Environment env;
			inja::Template temp = env.parse_template("web/books/pages_template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/books/"+m_book->getKey()+"/pages/index.html",result);
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
    private: 
        nlohmann::json m_topnav;

	public:
        StaticCatalogueCreator()
        {
            m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
            auto time=std::chrono::system_clock::now();
            std::time_t time_time = std::chrono::system_clock::to_time_t(time);
            m_topnav["time"] = std::ctime(&time_time);    
        }

		void CreateCatalogue()
		{
			nlohmann::json js;
			js["pages"].push_back("books");
			js["pages"].push_back("authors");
			js["pages"].push_back("collections");
			js["pages"].push_back("years");
			js["world"] = "hallo";

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active";


			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/catalogue/index.html",result);
		}

		void CreateCatlogueYears(CBookManager &mng)
		{
			nlohmann::json rend;

            //Parse navbar
            rend["topnav"] = m_topnav;
            rend["topnav"]["catalogue"] = "class='dropdown-banner active";

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
			atomic_write_file("web/catalogue/years/index.html",result);

			inja::Environment env2;
			inja::Template temp2 = env2.parse_template("web/catalogue/years/template_year.html");
			for(auto &x : _map)
			{
				std::error_code ec;
				std::filesystem::create_directory("web/catalogue/years/"+x.first,ec);
				nlohmann::json js;

                //Parse navbar
                js["topnav"] = m_topnav;
                js["topnav"]["catalogue"] = "class='dropdown-banner active";

                std::vector<nlohmann::json> vBooks;
				for(auto y : x.second.second)
				{
					nlohmann::json js_k;
					js_k["key"] = y->getMetadata().getMetadata("key", "data");
					js_k["title"] = y->getMetadata().getShow2();
					js_k["bib"] = y->getMetadata().getMetadata("bib");
					vBooks.push_back(std::move(js_k));
				}

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);
                js["books"] = vBooks;
				js["year"] = x.first;

				std::string result = env2.render(temp2, js);
				atomic_write_file("web/catalogue/years/"+x.first+"/index.html",result);
			}
		}

		void CreateCatalogueBooks(CBookManager &mng)
		{
            std::vector<nlohmann::json> vBooks;
			for(auto &it : mng.getMapOfBooks())
			{
				nlohmann::json entry;
				entry["key"] = it.second->getMetadata().getMetadata("key", "data");
				entry["title"] = it.second->getMetadata().getShow2();
				entry["bib"] = it.second->getMetadata().getMetadata("bib");
				entry["has_ocr"] = it.second->hasOcr();
            
				vBooks.push_back(std::move(entry));
			}

            std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

			nlohmann::json books;
            books["books"] = vBooks;

            //Parse navbar
            books["topnav"] = m_topnav;
            books["topnav"]["catalogue"] = "class='dropdown-banner active";

			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/books/template.html");
			std::string result = env.render(temp, books);
			atomic_write_file("web/catalogue/books/index.html",result);
		}

        void CreateCatalogueAuthors(CBookManager& manager)
		{
			nlohmann::json js;

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active";

            std::map<std::string, nlohmann::json> mapAuthors;
            for(auto &it : manager.getMapOfBooks()) 
            {
                std::string lastName = it.second->getMetadata().getAuthor();
                std::string firstName = it.second->getMetadata().getMetadata("firstName", "data", "creators", 0);
                auto origName = lastName;
                auto origFam = firstName;
                func::convertToLower(lastName);
                func::convertToLower(firstName);
                std::string key = firstName + "-" + lastName;
                std::replace(key.begin(), key.end(), ' ', '-');
                std::replace(key.begin(), key.end(), '/', ',');
        
                mapAuthors[lastName + "_" + firstName] = {
                            {"id", key},
                            {"show", origName + ", " + origFam},
                            {"num", manager.getMapofUniqueAuthors()[key].size()} };
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
			atomic_write_file("web/catalogue/authors/index.html",result);
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
                std::replace(key.begin(), key.end(), ' ', '-');
                std::replace(key.begin(), key.end(), '/', ',');

                if(lastName.size() == 0)
                    continue;

                //Create directory
                std::error_code ec;
                std::filesystem::create_directory("web/catalogue/authors/"+key+"/", ec);

                nlohmann::json js;
                js["author"] = {{"name", name}, {"id", key}};

                //Parse navbar
                js["topnav"] = m_topnav;
                js["topnav"]["catalogue"] = "class='dropdown-banner active";

                //Create json with all books
                std::vector<nlohmann::json> vBooks;
                if(manager.getMapofUniqueAuthors().count(key) == 0)
                {
                    std::cout << key << " NOTFOUND!!\n";
                    continue;
                }
                for(auto &jt : manager.getMapofUniqueAuthors()[key]) {
                    vBooks.push_back({ 
                        {"id", jt},
                        { "title", manager.getMapOfBooks()[jt]->getMetadata().getShow2()},
                        { "bib", manager.getMapOfBooks()[jt]->getMetadata().getMetadata("bib")}
                                          });
                }

                std::sort(vBooks.begin(), vBooks.end(), &StaticCatalogueCreator::mySort);

                js["books"] = vBooks;
                std::string result = env.render(temp, js);
                atomic_write_file("web/catalogue/authors/"+key+"/index.html",result);
            }
        }
    
        void CreateCatalogueCollections(nlohmann::json pillars)
        {
			nlohmann::json js;

            //Parse navbar
            js["topnav"] = m_topnav;
            js["topnav"]["catalogue"] = "class='dropdown-banner active";

            for(auto &it : pillars)
                js["pillars"].push_back(it);
            
			inja::Environment env;
			inja::Template temp = env.parse_template("web/catalogue/collections/template.html");
			std::string result = env.render(temp, js);
			atomic_write_file("web/catalogue/collections/index.html",result);
		}

        void CreateCatalogueCollection(CBookManager& manager, nlohmann::json pillars)
        {
            inja::Environment env;
            inja::Template temp = env.parse_template("web/catalogue/collections/template_collection.html");
            for(auto& it : pillars)
            {
                nlohmann::json js;

                //Parse navbar
                js["topnav"] = m_topnav;
                js["topnav"]["catalogue"] = "class='dropdown-banner active";

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
                atomic_write_file("web/catalogue/collections/"+key+"/index.html",result);
            }
        }

        static bool mySort(nlohmann::json i, nlohmann::json j) {
            std::string str1 = i["title"];
            std::string str2 = j["title"];
            return func::returnToLower(str1)<func::returnToLower(str2);
        }
};
