#pragma once

#include <string>

#include "base_data.h"
#include "catch2/catch.hpp"
#include "search_object.h"
#include "search_options.h"


namespace util {
  std::list<ResultObject> CheckResultsForQuery(std::string query, 
      SearchOptions search_options, BaseData* base_data);
  bool CheckAuthors(std::list<ResultObject>& results, std::string author);
  bool CheckSorting(std::list<ResultObject>& results, int sort_style);
}
