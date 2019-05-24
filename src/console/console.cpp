#include "console.hpp"
#include <unistd.h>

namespace alx
{
#ifndef COMPILE_UNITTEST
	console::console()
	{
		initscr();
		cbreak();
		raw();
		noecho();
		keypad(stdscr,TRUE);
		start_color();
		init_pair(1,COLOR_YELLOW,COLOR_BLACK);
		init_pair(2,COLOR_WHITE,COLOR_BLACK);
		init_pair(3,COLOR_RED,COLOR_BLACK);
		init_pair(4,COLOR_GREEN,COLOR_BLACK);

		int x, y;
		getmaxyx(stdscr,y,x);
		_stdout = newwin(y-2,x,0,0);
		_cmd = newwin(2,x,y-2,0);
		wattrset(_cmd,COLOR_PAIR(3)|A_BOLD);
		wattrset(_stdout,COLOR_PAIR(2)|A_BOLD);
		whline(_cmd,0,x);
		scrollok(_stdout,TRUE);
		//curs_set(0);
		wattrset(_cmd,COLOR_PAIR(4)|A_BOLD);
		mvwaddstr(_cmd,1,0,"$> ");
		wattrset(_cmd,COLOR_PAIR(1)|A_BOLD);
		wrefresh(_cmd);
	}

	std::string console::getCommand()
	{
		std::string command="";
		char ch;
		for(;;)
		{
			ch = wgetch(_cmd);
			std::lock_guard lck(_outputLock);
			if(ch==13||ch==10)
				break;
			else if(ch==KEY_BACKSPACE||ch == KEY_DC || ch == 127)
			{
				if(command!="")
					command.pop_back();

				int x,y;
				getyx(_cmd,y,x);
				wmove(_cmd,y,x-1);
				wdelch(_cmd);
			}
			else 
			{
				waddch(_cmd,ch);
				command+=ch;
			}
			wrefresh(_cmd);
		}
		std::lock_guard lck(_outputLock);
		wmove(_cmd, 1, 0);
		wclrtoeol(_cmd);
		wattrset(_cmd,COLOR_PAIR(4)|A_BOLD);
		mvwaddstr(_cmd,1,0,"$> ");
		wattrset(_cmd,COLOR_PAIR(1)|A_BOLD);
		wrefresh(_cmd);
		return std::move(command);
	}

	console::~console()
	{
		endwin();
	}

	console &console::GetConsole()
	{
		static console c;
		return c;
	}


	console &console::operator<<(int x)
	{
		return (*this)<<std::to_string(x);
	}
	console &console::operator<<(std::string strs)
	{
		std::lock_guard lck(_outputLock);
		waddstr(_stdout,strs.c_str());
		wrefresh(_stdout);
		return *this;
	}
#endif
}
