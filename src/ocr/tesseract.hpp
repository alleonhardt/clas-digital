#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>

class OcrCreator
{
    public:
	std::string CreateOcrFromImage(const unsigned char *data, unsigned int size, std::string language);
};
