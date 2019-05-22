#include "src/console/console.hpp"

namespace alx
{
	console::console()
	{
		initscr();
		printw("Give it a try");
		refresh();
	}

	console::~console()
	{
		endwin();
	}
}
