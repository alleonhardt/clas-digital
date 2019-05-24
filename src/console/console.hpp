#pragma once
#include <ncurses.h>
#include <iostream>
#include <mutex>
#include <list>

namespace alx
{
	class console
	{
		private:
			struct color
			{
				int x;
			};

			console();
			WINDOW *_stdout;
			WINDOW *_cmd;
			std::mutex _outputLock;
			std::list<std::string> _cmdBuffer;
		
			template<typename ...Args,typename T>
			void _write(T s,Args... args)
			{
				_write(s);
				_write(args...);
			}

			void _write(std::string s);
			void _write(color x);
			void _write(int x);
			void _write(const char *x);

		public:
			static color red_black;
			static color white_black;
			static color green_black;
			static color yellow_black;
			static color blue_black;


			~console();

			template<typename ...Args>
			void write(Args... args)
			{ 
				std::lock_guard lck(_outputLock);
				_write(args...);
				flush();
				_write(white_black);
			}

			std::string getCommand();
			void SetColor(color x);
			static console &GetConsole();
			console &operator<<(std::string strs);
			console &operator<<(int x);
			void flush();
	};

	static inline auto &cout = console::GetConsole();
	static constexpr const char endl[] = "\n";
}
