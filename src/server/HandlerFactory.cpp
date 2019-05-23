#include "src/server/HandlerFactory.hpp"

void HandlerFactory::parseCommands(std::string cmd)
{
	if(cmd=="?")
	{
		alx::cout.write(alx::console::yellow_black,std::string("Command ?: Supported commands -> 'shutdown'(Exits the programm),'show users'(Shows the current user table)\n"));
	}
	else if(cmd=="show users")
	{
		nlohmann::json js= nlohmann::json::parse(UserHandler::GetUserTable().toJSON());
		std::string xs = "\nShow users v0.1: \n";
		for(auto &entry : js)
		{
			xs+=entry["email"].get<std::string>();
			xs+=" with access: ";
			xs+=std::to_string(entry["access"].get<int>());
			xs+="\n";
		}
		xs+="\n";
		alx::cout.write(alx::console::yellow_black,std::move(xs));
	}
	else
	{
		alx::cout.write(alx::console::yellow_black,std::string("Unknown command(show list of commands with '?'): "),cmd,std::string("\n"));
	}
}
