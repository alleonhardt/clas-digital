#pragma once
#include <iostream>
#define DBG_INF_MSG_EXIT(x,y) std::cerr<<x<<std::endl;std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;exit(EXIT_FAILURE);
#define DBG_INF_MSG(x) std::cerr<<x<<std::endl;std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;
#define DBG_INF() std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;
#include "src/console/console.hpp"
#define DBG_HEADER alx::console::red_black,"\nError in file: ",__FILE__,",\nin Line: ",__LINE__,",\nin Function: ",__PRETTY_FUNCTION__,",\nError string: ",alx::console::black_red

namespace debug
{
	static inline volatile bool gGlobalShutdown = false;
	/**
	 * @brief This structure prints everything in order given to the constructor of the class and an endline at the end of all prints
	 */
	struct print
	{
		/**
		 * @brief Overloaded constructor prints all arguments in order to stdout
		 *
		 * @param t1 The argument to print now
		 * @param args The arguments to print next
		 */
		template<typename ...Args,typename T>print(T t1, Args... args)
		{
			alx::cout.writeln(t1,args...);
		}
	};

	/**
	 * @brief This structure does not do anything with its constructor arguments it also does not print them
	 */
	struct empty
	{
		/**
		 * @brief This function does not do anything at all
		 */
  		template<typename ...Args>empty(Args... /*args*/)
  		{}
	};
}

//Dont use the DBG_MSG by default
#define DBG_MSG debug::empty

#ifdef _DEBUG_
#include "src/util/debug_file.hpp"
#endif
