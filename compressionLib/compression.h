#ifndef COMP_H
#define COMP_H

#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <cmath>
#include <bitset>

#pragma pack(push, 1)

struct BitmapFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BitmapV5Header {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
    uint32_t bV5RedMask;
    uint32_t bV5GreenMask;
    uint32_t bV5BlueMask;
    uint32_t bV5AlphaMask;
    uint32_t bV5CSType;
    uint32_t bV5Endpoints[9];
    uint32_t bV5GammaRed;
    uint32_t bV5GammaGreen;
    uint32_t bV5GammaBlue;
    uint32_t bV5Intent;
    uint32_t bV5ProfileData;
    uint32_t bV5ProfileSize;
    uint32_t bV5Reserved;
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

    void compressImage(const std::string& inputFilename, const std::string& outputFilename);
    void restoreImage(const std::string& filename);
    void createBMPTest(const std::string& filename, const RawImageData &image);
    void writeToFile(const std::string& filename,
                     const std::vector<bool>& emptyRows,
                     const std::vector<uint8_t>& compressedData,
                     uint32_t width, uint32_t height);

private:
    RawImageData _readBMP(const std::string &filename);
    void _writeBMP(const std::string& filename, const RawImageData &image);
    void _encodeNonEmptyRow(const RawImageData& image, std::ofstream& file, int rowIndex, int rowStart);
    void _writeEncodedPixelSequence(const RawImageData& image, std::ofstream& file, int count, unsigned char color, bool isSameColor, int rowStart, int blockNumber);
    void _decodeNonEmptyRow(std::ifstream& file, RawImageData& image, int rowIndex);
};

class BitPacker {
public:
    void pushBit(bool bit) {
        if (bitIndex == 8) {
            std::cout<<"Full byte, add new"<<std::endl;
            byteBuffer.push_back(currentByte);
            currentByte = 0;
            bitIndex = 0;
        }

        currentByte |= (bit << (7 - bitIndex));
        std::cout << "currentByte: " << std::bitset<8>(currentByte) << std::endl;
        ++bitIndex;
    }

    void pushBits(const std::vector<bool>& bits) {
        for (bool bit : bits) {
            pushBit(bit);
        }
    }

    void pushByte(uint8_t byte) {
        std::cout << "pushByte: " << std::bitset<8>(byte) << std::endl;
        for (int i = 7; i >= 0; --i) {
            std::cout << "bit: " << std::bitset<8>((byte >> i) & 1) << std::endl;
            pushBit((byte >> i) & 1);
        }
    }

    void printBits() {
        for (auto byte : byteBuffer) {
            std::cout << std::bitset<8>(byte);
        }

        if (bitIndex > 0) {
            for (int i = 7; i >= bitIndex; --i) {
                std::cout << "0";  //padding
            }
            for (int i = bitIndex - 1; i >= 0; --i) {
                std::cout << ((currentByte >> i) & 1);
            }
        }

        std::cout << std::endl;
    }

    std::vector<uint8_t> getBuffer() const {
        return byteBuffer;
    }

private:
    std::vector<uint8_t> byteBuffer;
    uint8_t currentByte = 0;
    int bitIndex = 0;
};

#endif // COMP_H
