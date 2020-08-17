#include "pdf_extractor.h"

namespace fs = std::filesystem;
 
PdfExtractor::PdfExtractor() {
  cur_path_ = "";
  cur_path_to_pdf_ = "";
  num_pages_ = 0;
}

/**
 * Conterts given pdf to ocr and jpgs.
 * @param[in] path (path to directory of pdf)
 * @return success identification.
*/
bool PdfExtractor::Extract(std::string path) {
  cur_path_ = path;

  if (!fs::exists(cur_path_)) {
      std::cout << "Directory doesn't exist!\n";
      return false;
  }

  //Find pdf in given directory.
  if (FindPdf() == false)
    return false;

  //Convert pdf to text.
  if (ConvertToText() == false)
    return false;

  //Convert single pages to one ocr
  ConvertPagesToOcr();

  //Convert pdf to jpds
  ConvertPagesToJpgs();

  return true;
}

/**
 * Finds pdf in given directory.
 * @return success identification.
*/
bool PdfExtractor::FindPdf() {

  //iterate over given directory and find pdf
  for (const auto& p : fs::directory_iterator(cur_path_)) {
    if (p.path().extension() == ".pdf") {
      cur_path_to_pdf_ = p.path();
      return true;
    }
  }

  std::cout << "no pdf found!\n";
  return false;
}

/**
 * Convert pdf to ocr.
 * @return success identification.
*/
bool PdfExtractor::ConvertToText() {

  //Convert one page after the oter. Authomatically breaky, when page to high.
  for (size_t i=1; i<1000000; i++) {

    //Construct comand to extract nth page.
    std::string command = "pdftotext -layout -f " + std::to_string(i) + " -l " 
                          + std::to_string(i) + " " + cur_path_to_pdf_ + " "
                          + cur_path_ + "/" + std::to_string(i-1) + ".txt";

    //Execute command + break loop if couldn't be executed (last page reached)
    if (system(command.c_str()) != 0) {
        num_pages_ = i;
        break;
    }
  }

  return true;
}

/**
 * Convert pdf to ocr.
 * @return success identification.
*/
void PdfExtractor::ConvertPagesToOcr() {
  std::string ocr = "";
 
  //Iterate over given directory, find all pages and add to map to sort.
  std::map<int, std::string> sorted_pages;
  for (const auto& p : fs::directory_iterator(cur_path_)) {
    //Add page to map if its a txt-file and contains a digit.
    std::string name = p.path().stem();
    if (p.path().extension() == ".txt" && std::isdigit(name.front()) == true) 
      sorted_pages[std::stoi(name)] = p.path();
  }

  //Read file and add to ocr.
  for (auto page : sorted_pages) {
    //Read content of file
    std::ifstream read(page.second);
    std::string cur_page( (std::istreambuf_iterator<char>(read) ),
                           std::istreambuf_iterator<char>()     );
    read.close();

    //Add page-mark and delete end-of-file-character.
    cur_page.insert(0, "----- " + std::to_string(page.first) + " / " 
                        + std::to_string(num_pages_) + " -----\n\n");
    cur_page.pop_back();
    
    //Add to ocr and delete 
    ocr += cur_page + "\n\n";
    std::remove(page.second.c_str());
  }

  //Write ocr to disc.
  std::ofstream write(cur_path_ + "/ocr.txt");
  write << ocr;
  write.close();
}

/**
 * Convert pdf to jpgs
*/
void PdfExtractor::ConvertPagesToJpgs() {
  
  //Convert pdf to pngs using shell command.
  std::cout << "Converting pdf to png.\n";
  std::string command = "pdftoppm " + cur_path_to_pdf_ + " " 
                        + cur_path_ + "/page -png";
  system(command.c_str());

  //Convert all pngs to jpgs using shell command.
  std::cout << "Converting png to jpg.\n";
  command = "mogrify -format jpg " + cur_path_ + "/*.png";
  system(command.c_str());
  
  //Create directory for pages
  fs::create_directories(cur_path_ + "/pages");

  //Delete all .png files and rename jpg files (delete "-" bevor pagenumber)
  for (const auto& p : fs::directory_iterator(cur_path_)) {
    std::cout << p.path() << std::endl;
    if (p.path().extension() == ".png")
      std::remove(p.path().c_str());
    else if (p.path().extension() == ".jpg")
    {
      std::string filename = p.path().stem();
      size_t pos = filename.find("-");
      if (pos == std::string::npos)
        continue;
      std::string num = std::to_string(std::stoi(filename.substr(pos+1))-1);
      std::string new_path = p.path().parent_path();
      new_path += "/pages/page_"+num+".jpg";
      std::rename(p.path().c_str(), new_path.c_str());
    }
  }
}
