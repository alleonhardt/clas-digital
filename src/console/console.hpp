#pragma once
#include <ncurses.h>
#include <iostream>
#include <mutex>

namespace alx
{
#ifndef COMPILE_UNITTEST
	
	struct color
	{
		int x;
	};

	class console
	{
		private:
			console();
			WINDOW *_stdout;
			WINDOW *_cmd;
			std::mutex _outputLock;
		public:
			~console();

			std::string getCommand();
			static console &GetConsole();
			console &operator<<(std::string strs);
			console &operator<<(int x);
	};

	static inline auto &cout = console::GetConsole();
#else
	static inline auto &cout = std::cout;
#endif
	static constexpr const char endl[] = "\n";
}
