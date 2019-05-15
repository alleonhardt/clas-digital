#pragma once
#include <iostream>

#define DBG_INF_MSG_EXIT(x,y) std::cerr<<x<<std::endl;std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;exit(EXIT_FAILURE);
#define DBG_INF_MSG(x) std::cerr<<x<<std::endl;std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;
#define DBG_INF() std::cerr<<"Error in FILE: "<<__FILE__<<"\nIn Line: "<<__LINE__<<"\nIn Function: "<<__PRETTY_FUNCTION__<<std::endl;

namespace debug
{
	struct print
	{
		template<typename ...Args,typename T>print(T t1, Args... args)
		{
			std::cout<<t1;
			print(args...);
		}
		template<typename T>print(T t1)
		{
			std::cout<<t1<<std::endl;
		}
	};

	struct empty
	{
  		template<typename ...Args>empty(Args... args __attribute__((unused)))
  		{}
	};
}
