#ifndef COMP_H
#define COMP_H

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>

#pragma pack(push, 1)
struct BMPHeader {
    char header[2];
    unsigned int fileSize;
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int dataOffset;
    unsigned int headerSize;
    int width;
    int height;
    unsigned short planes;
    unsigned short bitsPerPixel;
    unsigned int compression;
    unsigned int imageSize;
    int xPixelsPerMeter;
    int yPixelsPerMeter;
    unsigned int colorsInColorTable;
    unsigned int importantColorCount;
};
#pragma pack(pop)

struct RawImageData {
    int width; // image width in pixels
    int height; // image height in pixels
    unsigned char *data; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.

    // Destructor to free allocated memory
    ~RawImageData();
};

class Compression
{

public:
    Compression();
    //~Compression();

    void compressImage(const std::string& filename);
    //void restoreImage(const std::string& filename);

private:
    RawImageData _readBMP(const std::string &filename);
    void _encodeNonEmptyRow(const RawImageData& image, std::ofstream& file, int rowIndex, int rowStart);
    void _writeEncodedPixelSequence(const RawImageData& image, std::ofstream& file, int count, unsigned char color, bool isSameColor, int rowStart, int blockNumber);
    //void _decodeNonEmptyRow(std::ifstream& file, RawImageData& image, int rowIndex);
    //void _saveGrayscaleBMP(const std::string& filename, const RawImageData& image);
};

#endif // COMP_H
