#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <string_view>
#include <map>

class TesseractInterface {
    private:
	tesseract::TessBaseAPI *_api;
    public:
	TesseractInterface(const char *language);
	~TesseractInterface();
	std::string recognizeOCRTxt(const unsigned char *data, unsigned int size);
};

class OcrCreator
{
    private:
	std::map<std::string,TesseractInterface*> _tessInter;

    public:
	OcrCreator()
	{
	    _tessInter["deu_frak"] = new TesseractInterface("deu_frak");
	    _tessInter["deu"] = new TesseractInterface("deu");
	    _tessInter["eng"] = new TesseractInterface("eng");
	}
	
	~OcrCreator()
	{
	    for(auto &it : _tessInter)
		delete it.second;
	}

	std::string CreateOcrFromImage(const unsigned char *data, unsigned int size, std::string language);
};


