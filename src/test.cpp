#include "src/books/CBookManager.hpp"
#include "src/books/func.hpp"
#include "src/server/HandlerFactory.hpp"
#include "src/zotero/zotero.hpp"
#include "src/console/console.hpp"
#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
