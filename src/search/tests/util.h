#pragma once

#include <string>

#include "base_data.h"
#include "catch2/catch.hpp"
#include "search_object.h"
#include "search_options.h"


namespace util {
  void CheckResultsForQuery(std::string query, SearchOptions search_options, BaseData* base_data);
}
