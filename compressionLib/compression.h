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
#include <cstdint>

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

struct RawImageData {
    int width = 0;
    int height = 0;
    unsigned char * data = nullptr; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.

    ~RawImageData();
    
    // Disable copying to prevent double-free
    RawImageData() = default;
    RawImageData(const RawImageData&) = delete;
    RawImageData& operator=(const RawImageData&) = delete;
    
    // Allow move
    RawImageData(RawImageData&& other) noexcept : width(other.width), height(other.height), data(other.data) {
        other.data = nullptr;
        other.width = 0;
        other.height = 0;
    }
    RawImageData& operator=(RawImageData&& other) noexcept {
        if (this != &other) {
            delete[] data;
            width = other.width;
            height = other.height;
            data = other.data;
            other.data = nullptr;
            other.width = 0;
            other.height = 0;
        }
        return *this;
    }
};

class BitPacker {
public:
    BitPacker() = default;
    BitPacker(const BitPacker&) = delete;
    BitPacker& operator=(const BitPacker&) = delete;

    // Write operations
    void flush();
    void pushBit(bool bit);
    void pushBits(const std::vector<bool>& bits);
    void pushByte(uint8_t byte) ;
    std::vector<uint8_t> getBuffer() const;

    // Read operations
    void setBuffer(std::vector<uint8_t>&& data);
    bool eof() const;
    bool readBit();
    uint8_t readBits(int n);
    void printBinBuffer() const;
    void printHexBuffer() const;

private:
    std::vector<uint8_t> _byteBuffer;
    mutable size_t readIndex = 0;
    mutable int bitIndex = 0;
    uint8_t writeByte = 0;
};

class ImageHandler {
public:
    ImageHandler() = default;

    void compressImage(const std::string& inputFilename, const std::string& outputFilename);
    void restoreImage(const std::string& compressedFilename, const std::string& restoredFilename);
    void createBMPTest(const std::string& filename, const RawImageData &image);
    void writeToFile(const std::string& filename,
                     const std::vector<bool>& emptyRows,
                     const std::vector<uint8_t>& compressedData,
                     uint32_t width, uint32_t height);

private:
    void _decodePixels(BitPacker& bitPacker, unsigned char* imgData,
                      const std::vector<bool>& emptyRows,
                      int width, int height, size_t& outSize);
    RawImageData _readBMP(const std::string &filename);
    RawImageData _readBarch(const std::string &filename);
    void _writeBMP(const std::string& filename, const RawImageData &image);
};

#pragma pack(pop)
#endif // COMP_H
