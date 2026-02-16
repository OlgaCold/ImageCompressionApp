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

struct RawImageData {
    int width; // image width in pixels
    int height; // image height in pixels
    unsigned char *data; // Pointer to image data. data[j * width + i] is color of pixel in row j and column i.

    // Destructor to free allocated memory
    ~RawImageData();
};

class BitPacker {
public:

    bool eof() const { return currentByte >= _byteBuffer.size(); }

    void flush() { //write
        if (bitIndex > 0) {
            // Додаємо паддінг (заповнюємо залишкові біти нулями)
            _byteBuffer.push_back(currentByte); // записуємо остаточний байт
            currentByte = 0;
            bitIndex = 0;
        }
    }

    void flushRead() {
        if (bitIndex > 0) {
            // Додаємо паддінг (заповнюємо залишкові біти нулями)
            _byteBuffer.push_back(currentByte); // записуємо остаточний байт
            currentByte = 0;
            bitIndex = 0;
        }
    }

    void pushBit(bool bit) {
        if (bitIndex == 8) {
            //std::cout<<"Full byte, add new"<<std::endl;
            _byteBuffer.push_back(currentByte);
            currentByte = 0;
            bitIndex = 0;
        }

        currentByte |= (bit << (7 - bitIndex));
        //std::cout << "currentByte: " << std::bitset<8>(currentByte) << std::endl;
        ++bitIndex;
    }

    void pushBits(const std::vector<bool>& bits) {
        for (bool bit : bits) {
            pushBit(bit);
        }
    }

    void pushByte(uint8_t byte) {
        //std::cout << "pushByte: " << std::bitset<8>(byte) << std::endl;
        for (int i = 7; i >= 0; --i) {
            //std::cout << "bit: " << std::bitset<8>((byte >> i) & 1) << std::endl;
            pushBit((byte >> i) & 1);
        }
        //printBinBuffer();
    }

    void printBinBuffer() {
        std::cout << "Buffer in bin format: " << std::endl;
        for (auto byte : _byteBuffer) {
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
    void printHexBuffer() {
        std::cout << "Buffer in hex format: " << std::endl;
        for (size_t k = 0; k < _byteBuffer.size() ; ++k) {
            std::cout << std::hex << static_cast<int>(_byteBuffer[k]) << " ";
        }
        std::cout << std::endl;
    }

    std::vector<uint8_t> getBuffer() const {
        return _byteBuffer;
    }

    void setBuffer(std::vector<uint8_t>&& data) {
        _byteBuffer = std::move(data);
        currentByte = 0;
        bitIndex = 0;
    }

    bool readBit() {
        if (currentByte >= _byteBuffer.size()) {
            throw std::runtime_error("End of buffer");
        }
        //std::cout << std::bitset<8>(_byteBuffer[currentByte]) << " *"<< std::endl;
        bool bit = (_byteBuffer[currentByte] >> (7 - bitIndex)) & 1;
        bitIndex++;
        if (bitIndex == 8) {
            bitIndex = 0;
            currentByte++;
        }
        std::cout << bit << " ";
        return bit;
    }

    // читає n бітів і повертає у uint8_t (max 8 біт)
    uint8_t readBits(int n) {
        std::cout << "readBits" << std::endl;
        uint8_t value = 0;
        for (int i = 0; i < n; ++i) {
            value <<= 1;
            value |= readBit() ? 1 : 0;
        }
        return value;
    }

private:
    std::vector<uint8_t> _byteBuffer;
    uint8_t currentByte = 0;
    int bitIndex = 0;
};

class ImageHandler
{

public:
    ImageHandler(/*BitPacker& compressor*/);
    //~ImageHandler();

    void compressImage(const std::string& inputFilename, const std::string& outputFilename);
    void restoreImage(const std::string& compressedFilename, const std::string& restoredFilename);
    void createBMPTest(const std::string& filename, const RawImageData &image, size_t outSize);
    void writeToFile(const std::string& filename,
                     const std::vector<bool>& emptyRows,
                     const std::vector<uint8_t>& compressedData,
                     uint32_t width, uint32_t height);


private:
    //BitPacker& compressor;
    void _decodePixels(BitPacker& bitPacker, unsigned char* imgData, std::vector<bool> emptyRows, size_t& outSize);
    RawImageData _readBMP(const std::string &filename);
    RawImageData _readBarch(const std::string &filename);
    void _writeBMP(const std::string& filename, const RawImageData &image);
};

#pragma pack(pop)
#endif // COMP_H
