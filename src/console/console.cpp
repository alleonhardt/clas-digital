#include "console.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

namespace alx
{
	console::color console::red_black;
	console::color console::white_black;
	console::color console::green_black;
	console::color console::yellow_black;
	console::color console::blue_black;
	console::color console::black_red;
	volatile bool resize_window = false;

	void do_resize(int dummy)
	{
		(void)dummy;
		resize_window = true;
	}

	console::console()
	{
		initscr();
		cbreak();
		raw();
		noecho();
		start_color();
		init_pair(1,COLOR_YELLOW,COLOR_BLACK);
		init_pair(2,COLOR_WHITE,COLOR_BLACK);
		init_pair(3,COLOR_RED,COLOR_BLACK);
		init_pair(4,COLOR_GREEN,COLOR_BLACK);
		init_pair(5,COLOR_BLUE,COLOR_BLACK);
		init_pair(6,COLOR_BLACK,COLOR_RED);

		console::red_black.x = COLOR_PAIR(3)|A_BOLD;
		console::green_black.x = COLOR_PAIR(4)|A_BOLD;
		console::yellow_black.x = COLOR_PAIR(1)|A_BOLD;
		console::white_black.x = COLOR_PAIR(2)|A_BOLD;
		console::blue_black.x = COLOR_PAIR(5)|A_BOLD;
		console::black_red.x = COLOR_PAIR(6);

		int x, y;
		getmaxyx(stdscr,y,x);
		_stdout = newwin(y-2,x,0,0);
		_cmd = newwin(2,x,y-2,0);
		_scrollback = newpad(2000,x);
		keypad(_cmd,TRUE);
		wattrset(_cmd,COLOR_PAIR(3)|A_BOLD);
		wattrset(_stdout,COLOR_PAIR(2)|A_BOLD);
		wattrset(_scrollback,COLOR_PAIR(2)|A_BOLD);
		whline(_cmd,0,x);
		scrollok(_stdout,TRUE);
		scrollok(_scrollback,TRUE);
		wattrset(_cmd,COLOR_PAIR(4)|A_BOLD);
		mvwaddstr(_cmd,1,0,"$> ");
		wattrset(_cmd,COLOR_PAIR(1)|A_BOLD);
		wrefresh(_cmd);
		signal(SIGWINCH, do_resize);
		_scrolling = false;
	}

	void console::SetColor(color x)
	{
		std::lock_guard lck(_outputLock);
		wattrset(_stdout,x.x);
		wattrset(_scrollback,x.x);
	}

	std::string console::getCommand()
	{
		std::string command="";
		int ch;
		auto it = _cmdBuffer.end();

		int currline = 0;
		WINDOW *scrollwin;
		int dimx,dimy;
		getmaxyx(stdscr,dimy,dimx);
		scrollwin = newpad(1000,dimx);

		for(;;)
		{
			ch = wgetch(_cmd);
			std::lock_guard lck(_outputLock);
			
			if(ch==KEY_F(12))
				continue;
			else if(ch==13||ch==10)
				break;
			else if(ch==KEY_PPAGE)
			{
				if(!_scrolling)
				{
					int x,y;
					_scrolling = true;
					getyx(_scrollback,y,x);
					currline = y-dimy+3;
					copywin(_scrollback,scrollwin,0,0,0,0,999,dimx-1,0);
				}
				currline = currline-1;
				if(currline<0)
					currline = 0;
				prefresh(scrollwin,currline,0,0,0,dimy-3,dimx-1);
				continue;
			}
			else if(ch==KEY_NPAGE)
			{
				if(_scrolling)
				{
					currline+=1;
					if(currline>(999-(dimy-3)))
						currline = 999-(dimy-3);
					prefresh(scrollwin,currline,0,0,0,dimy-3,dimx-1);
				}
				continue;
			}

			if(_scrolling)
			{
				_scrolling = false;
				redrawwin(_stdout);
				wrefresh(_stdout);
			}

			if(resize_window)
			{
				resize_window = false;
				int x, y;
				struct winsize size;
				ioctl(STDOUT_FILENO,TIOCGWINSZ,&size);
				y = size.ws_row;
				x = size.ws_col;
				dimx = x;
				dimy = y;
				resizeterm(y,x);
				wresize(_cmd,2,x);
				wresize(_scrollback,1000,dimx);
				wresize(scrollwin,1000,dimx);
				mvwin(_cmd,y-2,0);
				wresize(_stdout,y-2,x);
				
				wclear(_cmd);
				wattrset(_cmd,COLOR_PAIR(3)|A_BOLD);
				wmove(_cmd,0,0);
				whline(_cmd,0,x);
				wattrset(_cmd,COLOR_PAIR(4)|A_BOLD);
				mvwaddstr(_cmd,1,0,"$> ");
				wattrset(_cmd,COLOR_PAIR(1)|A_BOLD);

				command="";
				wrefresh(_stdout);
				wrefresh(_cmd);
				continue;
			}
			else if(ch==KEY_UP)
			{
				if(it==_cmdBuffer.begin()||_cmdBuffer.size()==0)
					continue;
				it--;
				wmove(_cmd,1,3);
				wclrtoeol(_cmd);
				command = it->c_str();
				mvwaddstr(_cmd,1,3,command.c_str());
			}
			else if(ch==KEY_DOWN)
			{
				wmove(_cmd,1,3);
				wclrtoeol(_cmd);
				wmove(_cmd,1,3);
				command="";
				if(it==_cmdBuffer.end()||_cmdBuffer.size()==0)
					continue;
				it++;
				if(it==_cmdBuffer.end())
					continue;
				command = it->c_str();
				waddstr(_cmd,command.c_str());
			}
			else if(ch==KEY_BACKSPACE||ch == KEY_DC || ch == 127)
			{
				if(command=="")
					continue;

				command.pop_back();
				int x,y;
				getyx(_cmd,y,x);
				wmove(_cmd,y,x-1);
				wdelch(_cmd);
			}
			else if(ch>255)
				continue;
			else 
			{
				waddch(_cmd,ch);
				char singlec = (char)ch;
				command+=singlec;
			}
			wrefresh(_cmd);
		}
		if(_cmdBuffer.size() == 0 || _cmdBuffer.back() != command)
			_cmdBuffer.push_back(command);
		std::lock_guard lck(_outputLock);
		wmove(_cmd, 1, 0);
		wclrtoeol(_cmd);
		wattrset(_cmd,COLOR_PAIR(4)|A_BOLD);
		mvwaddstr(_cmd,1,0,"$> ");
		wattrset(_cmd,COLOR_PAIR(1)|A_BOLD);
		wrefresh(_cmd);
		return std::move(command);
	}

	void console::_write(std::string s)
	{
		waddstr(_stdout,s.c_str());
		waddstr(_scrollback,s.c_str());
	}

	void console::flush()
	{
		if(!_scrolling)
			wrefresh(_stdout);
	}


	void console::_write(const char *x)
	{
		waddstr(_stdout,x);
		waddstr(_scrollback,x);
	}

	void console::_write(int x)
	{
		waddstr(_stdout,std::to_string(x).c_str());
		waddstr(_scrollback,std::to_string(x).c_str());
	}

	void console::_write(color x)
	{
		wattrset(_stdout,x.x);
		wattrset(_scrollback,x.x);
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
		_write(std::move(strs));
		flush();
		return *this;
	}
}
