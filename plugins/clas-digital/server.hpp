#ifndef CLAS_DIGITAL_PLUGIN_SERVER_H
#define CLAS_DIGITAL_PLUGIN_SERVER_H

#include "filehandler/filehandler.hpp"
#include "plugins/EventManager.hpp"
#include "reference_management/IReferenceManager.h"
#include "server/server.hpp"


class PluginServer
{
  public:
    PluginServer(clas_digital::CLASServer *server);


    /**
     * @brief Initialise all variables and redirects
     */
    void Initialise();
    
    void UpdateMetadataPage(clas_digital::IReference *ref);
    void UpdateDisplayPage(clas_digital::IReference *ref);

  private:
    clas_digital::CLASServer *server_;
    std::shared_ptr<clas_digital::IFileHandler> file_handler_;
    httplib::Server *http_server_;
    std::shared_ptr<clas_digital::EventManager> event_manager_;
    std::shared_ptr<clas_digital::IReferenceManager> reference_manager_;

    
    clas_digital::IReferenceManager::ptr_cont_t book_metadata_;
    clas_digital::IReferenceManager::ptr_cont_t all_collections_;
};


#endif
