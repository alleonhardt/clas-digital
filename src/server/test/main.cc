#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>
#include <termcolor/termcolor.hpp>
#include "version.hpp"

int main( int argc, char* argv[] ) {

  version::print_version();
  Catch::Session session; // There must be exactly one instance

  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) // Indicates a command line error
    return returnCode;
 


  constexpr int iterations = 10;
  for(int i = 0; i < iterations; i++) {
    std::cout<<termcolor::blue<<"[CATCH 2]: -- Iteration "<<i+1<<"/"<<iterations<<std::endl;
    int result = session.run();
    if(result != 0)
      return result;
  }

  // global clean-up...

  return 0;
}

