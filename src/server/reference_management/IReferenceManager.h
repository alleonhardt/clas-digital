#ifndef CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H
#define CLASDIGITAL_SRC_SERVER_REFERENCE_MANAGEMENT_IREFERENCE_MANAGER_H

#include <nlohmann/json.hpp>

/**
 * @brief Manages the reference information, for the moment only manages the
 * data from zotero but could be extended in a later version
 */
class IReferenceManager
{
  public:
   
    /**
     * @brief Updates the corpus from a remote or local source, depending on the
     * details given to the UpdateCorpus field
     *
     * @param details The parameters to give to the update function e. g.
     * credentials, url, cache options whatever, look into the documentation of
     * the concrete implementation
     */
    virtual bool UpdateCorpus() = 0;

    /**
     * @brief Returns the reference management of the texts
     *
     * @return All references to all managed items
     */
    virtual nlohmann::json &references() = 0;
};



#endif
