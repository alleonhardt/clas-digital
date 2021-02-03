/*
* @author fux
*/

#ifndef PDFEXTRACTOR_PDFEXTRACTOR_H_
#define PDFEXTRACTOR_PDFEXTRACTOR_H_

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

class PdfExtractor {
  public:
    PdfExtractor();

    /**
     * Conterts given pdf to ocr and jpgs.
     * @param[in] path (path to directory of pdf)
     * @return success identification.
    */
    bool Extract(std::string path);

  private:

    /**
     * Finds pdf in given directory.
     * @return success identification.
    */
    bool FindPdf();

    /**
     * Convert pdf to ocr.
     * @return success identification.
    */
    bool ConvertToText();

    /**
     * Convert pages to ocr.
    */
    void ConvertPagesToOcr();

    /**
     * Convert pdf to jpgs
    */
    void ConvertPagesToJpgs();

    // *** member variables *** //
    
    std::string cur_path_;  ///< Path to given directory
    std::string cur_path_to_pdf_;  ///< path to current pdf
    int num_pages_;
};

#endif
