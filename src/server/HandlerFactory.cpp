#include "src/server/HandlerFactory.hpp"

void HandlerFactory::parseCommands(std::string cmd)
{
	if(cmd=="?")
	{
		alx::cout<<"Command ?: Supported commands -> 'quit'(Exits the programm),'show users'(Shows the current user table)"<<alx::endl;
	}
	else if(cmd=="show users")
	{
		nlohmann::json js= nlohmann::json::parse(UserHandler::GetUserTable().toJSON());
		alx::cout<<"\nShow users v0.0: "<<alx::endl;
		for(auto &entry : js)
		{
			alx::cout<<entry["email"].get<std::string>()<<" with access: "<<entry["access"].get<int>()<<alx::endl;
		}
		alx::cout<<alx::endl;
	}
	else
	{
		alx::cout<<"Unknown command(show list of commands with '?'): "<<cmd<<alx::endl;
	}
}
