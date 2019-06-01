/**
 * @file src/console/console.hpp
 * Basic console file defines interfaces for outputting to the terminal and reading user input from the terminal
 */
#pragma once
#include <ncurses.h>
#include <iostream>
#include <mutex>
#include <list>

namespace alx
{
	/**
	 * @brief Basic console class, creates a USER interface in the terminal based on  the ncurses library
	 */
	class console
	{
		private:
			/**
			 * @brief Defines the basic color structure just to ensure type safety while printing
			 */
			struct color
			{
				int x;	///< The color code to set the terminal to.
			};

			WINDOW *_stdout; ///<The stdout window all output is directed to, this is the bigger window seen in the user interface
			WINDOW *_cmd;	///<The command window this holds the window seperators and also the basic command line only console input will print in this window
			std::mutex _outputLock;	///<Synchronisation of the screen to ensure conflict free printing
			std::list<std::string> _cmdBuffer;	///<A list with the last commands used so the user can scroll back and forth in it


		private:
			/**
			 * @brief Basic constructor disables the basic cout and cin and brings the terminal in the alternative screen mode, also initialises color codes
			 */
			console();
		
			/**
			 * @brief Writes a multiple data sets directly after each other to the screen, the whole operation will execute atomically
			 *	This function does not do any locking!!! Thats why it is private, always invoke the normal write function which handles the locking
			 * @param s The current param to write to the screen
			 * @param args The whole rest of the arguments
			 */
			template<typename ...Args,typename T>
			void _write(T s,Args... args)
			{
				_write(s);		//Write the current param to screen
				_write(args...);	//Recursively call ourself with the rest of the params, let the compiler decide which function to call
			}

			/**
			 * @brief The recursion anchor for _write tries to convert the argument to string to print it with a easy print string function
			 * @param argument The object to print
			 */
			template<typename T>
			void _write(T argument)
			{
				_write(std::to_string(argument));
			}

			/**
			 * @brief Output a string to the output window, this function does not lock the output lock this has to be done before calling this function
			 * @param s The string to print
			 */
			void _write(std::string s);

			/**
			 * @brief This function changes the output color of the terminal to the given color
			 * @param x The color to which the terminal output should change
			 */
			void _write(color x);

			/**
			 * @brief This function prints an integer on the screen
			 * @param x The integer to print
			 */
			void _write(int x);

			/**
			 * @brief This function prints a simple c string on the screen
			 * @param x The 0-terminated string to print
			 */
			void _write(const char *x);

		public:
			static color red_black; 	///<The color foreground red, background black in the terminal
			static color white_black;	///<The color foreground white, background black in the terminal, this is the default color
			static color green_black;	///<The color foreground green, background black in the terminal
			static color yellow_black;	///<The color foreground yellow, background black in the terminal
			static color blue_black;	///<The color foreground blue, background black in the terminal
			static color black_red; 	///<The color foreground black, background red used for error messages!


			/**
			 * @brief The destructor this resets the console to the original state and deletes all allocated objects
			 */
			~console();

			/**
			 * @brief This function atomically prints multiple arguments on screen and resets the color of the terminal the back to white_black
			 * @param args The arguments to print on screen
			 */
			template<typename ...Args>
			void write(Args... args)
			{ 
				std::lock_guard lck(_outputLock);
				_write(args...);
				flush();
				_write(white_black);
			}

			/**
			 * @brief This function is the same as write, but includes a new line at the end of the argument printing
			 *
			 * @param args The arguments to print on screen
			 */
			template<typename ...Args>
			void writeln(Args... args)
			{
				write(args...,"\n");
			}

			/**
			 * @brief Reads a string from the user input and returns it on enter
			 * @return The string the user entered
			 */
			std::string getCommand();

			/**
			 * @brief Sets the color to the specified color given to the function
			 */
			void SetColor(color x);

			/**
			 * @brief This function manages the Singleton instance of the console as there can always only be one console at one time.
			 * @return A reference to the only instance of the console
			 */
			static console &GetConsole();
			
			/**
			 * @brief Mimicks the basic cout operator only used for basic printing
			 * @param strs The string to print on screen
			 * @return A reference to itself for function chaining e. g.
			 * @code
			 * alx::cout<<"Hallo "<<"Welt"<<"\n";
			 * @endcode
			 */
			console &operator<<(std::string strs);
			
			/**
			 * @brief Mimicks the basic cout operator only used for basic printing
			 * @param x The string to print on screen
			 * @return A reference to itself for function chaining e. g.
			 * @code
			 * alx::cout<<"Hallo "<<"Welt"<<10;
			 * @endcode
			 */
			console &operator<<(int x);

			/**
			 * @brief Flushes all pending changes to the console write will do this automatically for you.
			 */
			void flush();
	};

	static inline auto &cout = console::GetConsole(); ///<Always have a global reference in the header file named cout so one can easily use the console
	static constexpr const char endl[] = "\n";		  ///<Mimick endl for easier portability
}
