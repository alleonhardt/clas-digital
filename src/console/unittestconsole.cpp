#include "console.hpp"
#include <unistd.h>

namespace alx
{
	console::color console::red_black;
	console::color console::white_black;
	console::color console::green_black;
	console::color console::yellow_black;
	console::color console::blue_black;

	console::console()
	{
	}

	void console::SetColor(color)
	{
	}

	void console::_write(const char *x)
	{
		std::cout<<x;
	}

	std::string console::getCommand()
	{
		return "";
	}

	void console::_write(std::string s)
	{
		std::cout<<s;
	}

	void console::_write(int x)
	{
		std::cout<<x;
	}

	void console::_write(color)
	{
	}

	console::~console()
	{
	}

	console &console::GetConsole()
	{
		static console c;
		return c;
	}

	void console::flush()
	{
		std::cout.flush();
	}


	console &console::operator<<(int x)
	{
		return (*this)<<std::to_string(x);
	}
	console &console::operator<<(std::string strs)
	{
		std::cout<<strs;
		return *this;
	}
}
