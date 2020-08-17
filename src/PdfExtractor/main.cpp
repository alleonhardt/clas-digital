/**
* @author fux
*/

#include <iostream>
#include <string>

#include "pdf_extractor.h"

int main()
{
  std::cout << "WELCOME TO PDF EXTRACTOR\n";
  PdfExtractor extractor;

  for (;;) {
    std::cout << "Enter path to pdf: \n> ";
    std::string input;
    getline(std::cin, input);
    if (input == "q")
        break;
    std::cout << "Extracting pdf at path: " << input << std::endl;
    if (extractor.Extract(input) == true)
      std::cout << "done.\n";
    else
      std::cout << "failed.\n";
  }
  
  std::cout << "Thanks for using our application.\n";
}
