#include "server.hpp"
#include "httplib.h"
#include "plugins/EventManager.hpp"
#include "reference_management/IReferenceManager.h"
#include "server/server.hpp"

using namespace clas_digital;

PluginServer::PluginServer(clas_digital::CLASServer *server)
{
  server_ = server;
  http_server_ = &server_->GetHTTPServer();
  event_manager_ = server_->GetEventManager();

  //Fill the remaining interfaces
  event_manager_->RegisterForEvent(EventManager::Events::ON_AFTER_INITIALISE, nullptr,
      [this](CLASServer*,void*)
      {
        this->file_handler_ = server_->GetFileHandler();
        this->reference_manager_ = server_->GetReferenceManager();

        this->Initialise();
        return debug::Error(EventManager::RET_OK);
      }
      );

}


void PluginServer::Initialise()
{
  event_manager_->RegisterForEvent(EventManager::Events::ON_UPDATE_REFERENCE, nullptr,
      [this](CLASServer *srv, void *data)
      {
        IReference *ref = (IReference*)data; 
        UpdateMetadataPage(ref);
        UpdateDisplayPage(ref);
        return debug::Error(EventManager::RET_OK);
      });

  //Initialise Zotero and force fetch all zotero data!
    reference_manager_->GetAllCollections(all_collections_,IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);
    reference_manager_->GetAllItems(book_metadata_,IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);
    reference_manager_->GetAllCollections(all_collections_,IReferenceManager::CacheOptions::CACHE_FORCE_FETCH);

    //Add file redirect!
    file_handler_->AddAlias({"/search","/"},"../web/index.html");
}

void PluginServer::UpdateMetadataPage(clas_digital::IReference *ref)
{
			std::cout<<"Processing: "<<ref->GetKey()<<std::endl;
}

void PluginServer::UpdateDisplayPage(clas_digital::IReference *ref)
{

}
