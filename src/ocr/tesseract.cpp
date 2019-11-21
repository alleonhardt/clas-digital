#include "ocr/tesseract.hpp"


TesseractInterface::TesseractInterface(const char *language)
{
    _api = new tesseract::TessBaseAPI();
    if(_api->Init("trainingdata/",language))
    {
	_api = nullptr;
    }
}

TesseractInterface::~TesseractInterface()
{
    if(_api)
    {
	_api->End();
	delete _api;
	_api = nullptr;
    }
}

std::string TesseractInterface::recognizeOCRTxt(const unsigned char *data, unsigned int size)
{
    if(_api)
    {
	char *outTxt;
	Pix *image = pixReadMem(data,size);
	_api->SetImage(image);
	outTxt = _api->GetUTF8Text();
	std::string outstr = outTxt;
	delete [] outTxt;
	pixDestroy(&image);
	return outstr;
    }
    else
	return "";
}



std::string OcrCreator::CreateOcrFromImage(const unsigned char *data, unsigned int size, std::string language)
{
    TesseractInterface *inter;
    try
    {
	inter = _tessInter.at(language);
    }
    catch(...)
    {
	inter = _tessInter.at("deu");
    }
    return inter->recognizeOCRTxt(data,size);
}
