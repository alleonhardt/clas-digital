#include "ocr/tesseract.hpp"


std::string OcrCreator::CreateOcrFromImage(const unsigned char *data, unsigned int size, std::string language)
{
    try
    {

	char *outText;

	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	// Initialize tesseract-ocr with English, without specifying tessdata path
	if (api->Init("/usr/share/tessdata/", language.c_str())) {
	    fprintf(stderr, "Could not initialize tesseract.\n");
	    throw 0;
	}

	// Open input image with leptonica library
	Pix *image = pixReadMem(data,size);
	
	Pix *image2 = pixConvertTo1(image,0x22);
	api->SetImage(image2);
	// Get OCR result
	outText = api->GetUTF8Text();
	// Destroy used object and release memory
	api->End();
	std::string ret = outText;
	delete [] outText;
	pixDestroy(&image);
	pixDestroy(&image2);
	return ret;
    }
    catch(...)
    {
	return "ERROR";
    }
}
