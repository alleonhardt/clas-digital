#include "corpus_manager.h"
#include "reference_management/IReferenceManager.h"
#include "metadata_handler.h"
#include "filehandler/util.h"
#include <inja/inja.hpp>

using namespace clas_digital;

std::string get_machine_time() {
  time_t now;
  std::time(&now);
  char buf[sizeof("YYYY-mm-ddTHH:MM:ssZ")];
  std::strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&now));
  return std::string(buf);
}

bool mySort(nlohmann::json i, nlohmann::json j) {
  std::string check = "";
  if(i.count("title") > 0) check = "title";
  else if(i.count("show") > 0) check = "show";
  else return true;

  std::string str1 = i[check];
  std::string str2 = j[check];
  return func::returnToLower(str1)<func::returnToLower(str2);
}

/**
* @param[in, out] str string to be modified
*/
std::string returnToLower(std::string &str)
{
    std::locale loc1("de_DE.UTF8");
    std::string str2;
    for(unsigned int i=0; i<str.length(); i++)
        str2 += tolower(str[i], loc1);

    return str2;
}


std::string get_human_readable_time() {
  time_t now;
  std::time(&now);
  char buf_human[sizeof("YYYY-mm-dd HH:MM:ss")];
  strftime(buf_human, sizeof(buf_human), "%F %T", gmtime(&now));
  return std::string(buf_human);
}


void create_topnav_json(nlohmann::json &topnav) {
  nlohmann::json m_topnav = {{"information", ""}, {"search", ""}, {"catalogue", ""}, {"admin", ""}, {"upload", ""}};
  topnav["topnav"] = m_topnav;
  topnav["topnav"]["time"] = get_machine_time();
  topnav["topnav"]["time_human"] = get_human_readable_time();
}

void createSearchPage(IReferenceManager::ptr_cont_t &pillars) {
  inja::Environment env;
  nlohmann::json j;

  create_topnav_json(j);
  j["topnav"]["search"] = "class='dropdown-banner active'";
  j["pillars"] = nlohmann::json::array();
  for(auto &it : *pillars) {
    j["pillars"].push_back(it.second->json()["data"]);
  }

  inja::Template temp = env.parse_template("templates/search_template.html");
  std::string result = env.render(temp, j);  
  atomic_write_file("web/index.html", result);
}

void createInformationPage()
{
  inja::Environment env;
  nlohmann::json j;
  create_topnav_json(j);
  j["topnav"]["information"] = "class='dropdown-banner active'";

  inja::Template temp = env.parse_template("templates/information_template.html");
  std::string result = env.render(temp, j);  
  atomic_write_file("web/information/index.de.html", result);
}

void createAdminPage() {
  inja::Environment env;
  nlohmann::json j;
  create_topnav_json(j);
  j["topnav"]["admin"] = "classifiedContent active";

  inja::Template temp = env.parse_template("templates/administration_template.html");
  std::string result = env.render(temp, j);  
  atomic_write_file("web/private/admin/Administration.html", result);
}


void createUploadPage()
{
  inja::Environment env;
  nlohmann::json j;
  create_topnav_json(j);
  j["topnav"]["upload"] = "classifiedContent active";

  inja::Template temp = env.parse_template("templates/uploadBook_template.html");
  std::string result = env.render(temp, j);  
  atomic_write_file("web/private/write/UploadBook.html", result);
}

void createMetadataPage(IReference *item, IReferenceManager::ptr_cont_t &collections)
{
  std::cout<<"Processing: "<<item->GetKey()<<std::endl;
  auto m_info = item->json();
  MetadataHandler meta(m_info);


  auto itemType =m_info["data"].value("itemType","");
  auto title=m_info["data"].value("title","");
  auto isbn = m_info["data"].value("isbn","");
  nlohmann::json info;
  info["show"] = item->GetShow2();
  info["bib"] = m_info["bib"].get<std::string>();
  info["bib_own"] = ""; 
  info["hasContent"] = item->HasOcr();
  info["itemType"] = itemType;
  info["title"] = title;
  info["authors"] = nlohmann::json::array();
  for(auto author : meta.GetAuthorsKeys())
  {
    nlohmann::json j;
    j["key"] = author["key"];
    j["name"] = author["fullname"];
    j["type"] = author["creatorType"];
    j["isAuthor"] = meta.IsAuthorEditor(j["type"]);
    info["authors"].push_back(j);
  } 
  info["hasISBN"] = isbn != "";
  info["isbn"] = isbn;
  info["place"] = m_info["data"].value("place","");
  info["date"] = "";
  if(item->GetDate() != -1)
    info["date"] = item->GetDate(); 
  info["hasDate"] = item->GetDate() != -1;
  info["key"] = item->GetKey();

  info["collections"] = nlohmann::json::array();
  for(const auto& it : meta.GetCollections()) 
  {
    nlohmann::json j;
    j["key"] = it;
    j["name"] = "Untracked collection.";
    bool found = false;
    for(const auto& jt : *collections) {
      if(jt.second->GetKey() == it)
      {
        found = true;
        j["name"] = jt.second->GetName();
      }
    }
    if(found != false)
      info["collections"].push_back(j);
  }

  //Parse navbar
  create_topnav_json(info);
  info["topnav"]["catalogue"] = "class='dropdown-banner active'";

  inja::Environment env;
  inja::Template temp = env.parse_template("templates/metadata_template.html");
  std::string result = env.render(temp, info);

  atomic_write_file("web/books/"+item->GetKey()+"/index.html",result);
}

void escapeHTML(std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    data.swap(buffer);
}

void createPagesPage(IReference *item)
{
  nlohmann::json js;

  //Parse navbar
  create_topnav_json(js);
  js["topnav"]["catalogue"] = "class='dropdown-banner active'";
  
  auto m_info = item->json();
  js["key"] = item->GetKey();
  js["title"] = item->GetShow2();
  js["bib"] = m_info["bib"].get<std::string>();
  std::string bib_esc = m_info["bib"].get<std::string>();
  escapeHTML(bib_esc);
  js["bib_esc"] = bib_esc;
  inja::Environment env;
  inja::Template temp = env.parse_template("templates/pages_template.html");
  std::string result = env.render(temp, js);
  atomic_write_file("web/books/"+item->GetKey()+"/pages/index.html",result);
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
  create_topnav_json(js);
  js["topnav"]["catalogue"] = "class='dropdown-banner active'";


  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/template.html");
  std::string result = env.render(temp, js);
  atomic_write_file("web/catalogue/index.html",result);
}


void CreateCatalogueYears(IReferenceManager::ptr_cont_t &map)
{
  nlohmann::json rend;

  //Parse navbar
  create_topnav_json(rend);
  rend["topnav"]["catalogue"] = "class='dropdown-banner active'";

  std::map<std::string,std::pair<int,std::list<IReference*>>> _map;
  for(auto &book : *map)
  {
    std::string name = std::to_string(book.second->GetDate());
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
    create_topnav_json(js);
    js["topnav"]["catalogue"] = "class='dropdown-banner active'";

    std::vector<nlohmann::json> vBooks;
    for(auto y : x.second.second)
    {
      nlohmann::json js_k;
      js_k["key"] = y->GetKey();
      js_k["title"] = y->GetShow2();
      js_k["bib"] = y->GetBibliography();
      vBooks.push_back(std::move(js_k));
    }

    std::sort(vBooks.begin(), vBooks.end(), mySort);
    js["books"] = vBooks;
    js["year"] = x.first;

    std::string result = env2.render(temp2, js);
    atomic_write_file("web/catalogue/years/"+x.first+"/index.html",result);
  }
}


void CreateCatalogueBooks(IReferenceManager::ptr_cont_t &mapbooks)
{
  std::vector<nlohmann::json> vBooks;
  for(auto &it : *mapbooks)
  {
    nlohmann::json entry;
    entry["key"] = it.second->GetKey();
    entry["title"] = it.second->GetShow2();
    entry["bib"] = it.second->GetBibliography();
    entry["has_ocr"] = it.second->HasOcr();

    vBooks.push_back(std::move(entry));
  }

  std::sort(vBooks.begin(), vBooks.end(), mySort);

  nlohmann::json books;
  books["books"] = vBooks;

  //Parse navbar
  create_topnav_json(books);
  books["topnav"]["catalogue"] = "class='dropdown-banner active'";

  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/books/template.html");
  std::string result = env.render(temp, books);
  atomic_write_file("web/catalogue/books/index.html",result);
}

void CreateCatalogueAuthors(IReferenceManager::ptr_cont_t &cont, std::map<std::string,std::vector<std::string>> &uniqueAuthors)
{
  nlohmann::json js;

  //Parse navbar
  create_topnav_json(js);
  js["topnav"]["catalogue"] = "class='dropdown-banner active'";

  //Using map to erase dublicates
  std::map<std::string, nlohmann::json> mapAuthors;
  for(auto &it : *cont) 
  {
    for(auto author : it.second->GetAuthorKeys())
    {
      if(!it.second->IsAuthorEditor(author["creator_type"]))
        continue;

      mapAuthors[author["key"]] = {
        {"id", author["key"]},
        {"show", author["fullname"]},
        {"num", uniqueAuthors[author["key"]].size()} 
      };
    }
  }


  //Convert to vector, sort and add to json
  std::vector<nlohmann::json> vAuthors;
  for(auto &it : mapAuthors)
    vAuthors.push_back(it.second);
  std::sort(vAuthors.begin(), vAuthors.end(), mySort);
  js["authors"] = vAuthors;


  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/authors/template.html");
  std::string result = env.render(temp, js);
  atomic_write_file("web/catalogue/authors/index.html",result);
}

void CreateCatalogueAuthor(IReferenceManager::ptr_cont_t &books,std::map<std::string,std::vector<std::string>> &uniqueAuthors)
{
  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/authors/template_author.html");

  for(const auto &it : *books) {

    for(auto author : it.second->GetAuthorKeys())
    {
      std::string key = author["key"];

      if(author["lastname"].size() == 0)
        continue;

      if(!it.second->IsAuthorEditor(author["creator_type"]))
        continue;

      //Create directory
      std::error_code ec;
      std::filesystem::create_directory("web/catalogue/authors/"+key+"/", ec);

      nlohmann::json js;
      js["author"] = {{"name", author["fullname"]}, {"id", key}};

      //Parse navbar
      create_topnav_json(js);
      js["topnav"]["catalogue"] = "class='dropdown-banner active'";

      //Create json with all books
      std::vector<nlohmann::json> vBooks;
      if(uniqueAuthors.count(key) == 0)
        continue;

        for(auto &jt : uniqueAuthors[key]) {
        vBooks.push_back({ 
            {"id", jt},
            { "title", (*books)[jt]->GetShow2()},
            { "bib", (*books)[jt]->GetBibliography()},
            { "has_ocr", (*books)[jt]->HasOcr()}
            });
      }

      std::sort(vBooks.begin(), vBooks.end(), mySort);

      js["books"] = vBooks;
      std::string result = env.render(temp, js);
      atomic_write_file("web/catalogue/authors/"+key+"/index.html",result);
    }
  }
}

void CreateCatalogueCollections(IReferenceManager::ptr_cont_t &collections)
{
  nlohmann::json js;

  //Parse navbar
  create_topnav_json(js);
  js["topnav"]["catalogue"] = "class='dropdown-banner active'";

  for(auto &it : *collections) {
    std::cout<<it.second->json()<<std::endl;
    js["pillars"].push_back(it.second->json()["data"]);
  }

  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/collections/template.html");
  std::string result = env.render(temp, js);
  atomic_write_file("web/catalogue/collections/index.html",result);
}

void CreateCatalogueCollection(IReferenceManager::ptr_cont_t &items, IReferenceManager::ptr_cont_t &collections)
{
  inja::Environment env;
  inja::Template temp = env.parse_template("web/catalogue/collections/template_collection.html");
  for(auto& it : *collections)
  {
    nlohmann::json js;

    //Parse navbar
    create_topnav_json(js);
    js["topnav"]["catalogue"] = "class='dropdown-banner active'";

    js["pillar"] = it.second->json()["data"];
    std::string key = it.second->GetKey();

    //Create directory
    std::error_code ec;
    std::filesystem::create_directory("web/catalogue/collections/"+key+"/", ec);

    //Create books for this collection
    std::vector<nlohmann::json> vBooks;
    for(auto &jt : *items) {
      std::vector<std::string> collections = jt.second->GetCollections();

      if(collections.size() == 0 || func::in(key, collections) == false)
        continue;

      vBooks.push_back({ 
          {"id", jt.first},
          { "title", jt.second->GetShow2()},
          { "bib", jt.second->GetBibliography()},
          {"has_ocr",jt.second->HasOcr()}
          });
    }

    std::sort(vBooks.begin(), vBooks.end(), mySort);

    js["books"] = vBooks;
    std::string result = env.render(temp, js);
    atomic_write_file("web/catalogue/collections/"+key+"/index.html",result);
  }
}






bool createWebpage(IReference *item,IReferenceManager::ptr_cont_t &collections,clas_digital::IFileHandler *handler)
{
  std::string webpath = "/books/"+item->GetKey()+"/pages";
  std::error_code ec;

  std::filesystem::create_directory("web/books/"+item->GetKey(), ec);
  std::filesystem::create_directory("web/books/"+item->GetKey()+"/pages", ec);

  if(item->HasContent())
  {
    createPagesPage(item);
  }

  {
    createMetadataPage(item,collections);
  }

  return true;
}



bool CorpusManager::UpdateZotero(clas_digital::IReferenceManager *manager, clas_digital::IFileHandler *handler) {
  if(!manager)
    return false;

  auto res2 = manager->GetAllCollections(collection_references_);
  auto res = manager->GetAllItems(item_references_);
  res2 = manager->GetAllCollections(collection_references_);
  //manager->SaveToFile();

  createSearchPage(collection_references_);
  std::cout<<"After search"<<std::endl;
  createInformationPage();
  std::cout<<"After information"<<std::endl;
  createAdminPage();
  std::cout<<"After Admin"<<std::endl;
  createUploadPage();
  std::cout<<"After Upload"<<std::endl;

  std::cout<<"Value of pointer: "<<item_references_.get()<<std::endl;
  std::cout<<"Value of pointer: "<<collection_references_.get()<<std::endl;

  for(auto &i : *item_references_) {
    createWebpage(i.second,collection_references_,handler);
  }
  
  for(auto &it : *item_references_)
  {
    for(auto &author : it.second->GetAuthorKeys())
    {
      if(!it.second->IsAuthorEditor(author["creator_type"]))
        continue;
      m_mapUniqueAuthors[author["key"]].push_back(it.first);
    }
  }

  CreateCatalogue();
  CreateCatalogueCollections(collection_references_);
  CreateCatalogueCollection(item_references_,collection_references_);
  std::cout<<"Created catalogue collection index"<<std::endl;
  CreateCatalogueAuthors(item_references_,m_mapUniqueAuthors);
  CreateCatalogueAuthor(item_references_,m_mapUniqueAuthors);
  std::cout<<"Created catalogue author index"<<std::endl;
  CreateCatalogueYears(item_references_);
  std::cout<<"Created catalogue years index"<<std::endl;
  CreateCatalogueBooks(item_references_);
  std::cout<<"Created catalogue books index"<<std::endl;

  std::cout<<"Created full catalogue and book index"<<std::endl;
  auto no_error = res == clas_digital::IReferenceManager::Error::OK
    && res2 == clas_digital::IReferenceManager::Error::OK;

  return no_error;
}

IReference *CorpusManager::book(std::string ref) {
  return item_references_->at(ref);
}


void CorpusManager::WriteCollectionPage(IReference *ref) {

}

void CorpusManager::WriteMetadataPage(IReference *ref) {
}

void CorpusManager::BookIndex(IReference* ref) {
}

IReferenceManager::ptr_cont_t &CorpusManager::item_references() {
  return item_references_;
}
