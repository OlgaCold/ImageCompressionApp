#include "compression.h"
#include <iomanip>
#include <cstring>

ImageHandler::ImageHandler(/*Compressor& cp*/) /*: bitPacker(cp)*/ {}

RawImageData::~RawImageData() {
    delete[] data;
}

void ImageHandler::writeToFile(const std::string& filename,
                 const std::vector<bool>& emptyRows,
                 const std::vector<uint8_t>& compressedData,
                 uint32_t width, uint32_t height) {

    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Can`t open file to write!" << std::endl;
        return;
    }

    // 1. Identifier ("BA" - 2 bytes)
    char format[] = {'B', 'A'};
    file.write(format, sizeof(format));

    // 2. Width (4 bytes)
    file.write(reinterpret_cast<const char*>(&width), sizeof(width));

    // 3. Height (4 bytes)
    file.write(reinterpret_cast<const char*>(&height), sizeof(height));

    // 4. Empty rows index (bool values)

    // Pack vector to bytes
    size_t bitCount = emptyRows.size();
    std::cout<<std::dec<< "bitCount "<< bitCount <<std::endl;
    size_t byteCount = (bitCount + 7) / 8;// every 8bit = 2 bytes
    std::cout<<"byteCount "<< byteCount <<std::endl;
    std::vector<uint8_t> rowIndex(byteCount, 0);

    for (size_t i = 0; i < bitCount; ++i) {
        if (emptyRows[i]) {
            rowIndex[i / 8] |= (1 << (7 - (i % 8))); // set bit
        }
    }

    file.write(reinterpret_cast<const char*>(rowIndex.data()), rowIndex.size());

    // 5. Write encoded image data (vector uint8_t)
    file.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());

    // Close file
    file.close();
    std::cout << "Successfully created barch file" << std::endl;
}

RawImageData ImageHandler::_readBMP(const std::string &filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file");
    }

    RawImageData img;
    BitmapFileHeader fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

    if (fileHeader.bfType != 0x4D42) {
        throw std::runtime_error("Not a BMP file");
    }

    // read first 4 bytes - DIB
    uint32_t dibSize;
    file.read(reinterpret_cast<char*>(&dibSize), sizeof(dibSize));

    // return 4 bites back
    file.seekg(-4, std::ios::cur);

    if (dibSize == 124) {
        BitmapV5Header dibHeader;
        file.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

        std::cout << "Width: " << dibHeader.biWidth << "\n";
        std::cout << "Height: " << dibHeader.biHeight << "\n";
        std::cout << "BitCount: " << dibHeader.biBitCount << "\n";
        std::cout << "dibHeader.biHeight: " << dibHeader.biHeight << "\n";
        img.width = dibHeader.biWidth;
        img.height = dibHeader.biHeight;
    }
    else {
        throw std::runtime_error("Unsupported BMP header type");
    }

    img.data = new unsigned char[img.width * img.height];

    int padding = (4 - (img.width % 4)) % 4;
    std::cout << "padding: " << padding << "\n";
    int rowSize = img.width + padding;
    std::cout << "rowSize: " << rowSize << "\n";

    file.seekg(fileHeader.bfOffBits);
    std::vector<unsigned char> row(rowSize);
    for (int j = img.height - 1; j >= 0; --j) {
        file.read(reinterpret_cast<char*>(row.data()), rowSize);

        for (int i = 0; i < img.width; ++i) {
            img.data[j * img.width + i] = row[i];
        }
    }
    file.close();

    return img;
}

void ImageHandler::_decodePixels(BitPacker& bitPacker, unsigned char* imgData, std::vector<bool> emptyRows, size_t& outSize) {
    std::cout << "_decodePixels" << std::endl;
    std::vector<unsigned char> temp;

    int totalPixels = 825*1200;
    int decodedPixels = 0;

    for (size_t row = 0; row < 1200/*emptyRows.size()*/; ++row) { //2 non empty rows
        std::cout << std::dec<<  "vector capacity " << temp.capacity() << std::endl;
        std::cout << std::dec<<  "vector size " << temp.size() << std::endl;
        if (emptyRows[row]) {
            std::cout << "row " << row << std::endl;
            temp.insert(temp.end(), 825/*width*/, 0xFF);
            decodedPixels += 825;
            std::cout << std::dec <<"decodedPixels Empt " << decodedPixels << std::endl;
        } else {
            std::cout << "row " << row << std::endl;
            int rowPixels = 0;
            while (rowPixels < 825) {
                int pixelsToWrite = std::min(4, 825 - rowPixels);
                bool firstBit = bitPacker.readBit();
                if (!firstBit) {
                    // 0 → 4 white
                    temp.insert(temp.end(), pixelsToWrite, 0xFF);
                    decodedPixels += pixelsToWrite;
                    rowPixels += pixelsToWrite;
                    std::cout<<std::dec<<"+ 4 white "<< " ";
                    std::cout << "rowPixels " << rowPixels << std::endl;
                } else {
                    bool secondBit = bitPacker.readBit();
                    if (!secondBit) {
                        // 10 → 4 black
                        temp.insert(temp.end(), pixelsToWrite, 0x00);
                        decodedPixels += pixelsToWrite;
                        rowPixels += pixelsToWrite;
                        std::cout<<std::dec<<"+ 4 black "<< " ";
                        std::cout << "rowPixels " << rowPixels << std::endl;
                    } else {
                        // 11 → 4 mixed
                        for (int i = 0; i < pixelsToWrite; ++i) {
                            uint8_t color = bitPacker.readBits(8);
                            temp.push_back(color);
                        }
                        decodedPixels += pixelsToWrite;
                        rowPixels += pixelsToWrite;
                        std::cout<<std::dec<<"+ 4 mixed "<< " ";
                        std::cout << "rowPixels " << rowPixels << std::endl;
                    }
                }
            }
            //std::cout<<std::dec<<"- 3 additional "<< " ";
            /*for (int i = 0; i<3; ++i) {
                temp.pop_back();
                decodedPixels -= 1;
                rowPixels -= 1;
            }*/
            /*for (int i = 0; i<24; ++i) { //to read to the end of prev block
                bitPacker.readBit();
            }*/
            std::cout << "decodedPixels All " << decodedPixels << std::endl;
            std::cout << "rowPixels " << rowPixels << std::endl;
        }
    }
    //std::cout<<std::dec<<"decodedPixels All "<< decodedPixels<<std::endl;

    //std::cout << "Row 65 pixels vector restored" << std::endl;
    //for (size_t i = 0; i < temp.size(); ++i) {
    //    std::cout << std::hex << std::setw(2) << std::setfill('0')<< static_cast<int>(temp[i]) << " ";
    //}
    //std::cout<<std::endl;

    outSize = totalPixels;
    std::cout<< std::dec << "outSize " << outSize << std::endl;
    size_t copySize = std::min(temp.size(), static_cast<size_t>(outSize));
    std::memcpy(imgData, temp.data(), copySize);

    /*std::cout << "Row 65 pixels char restored" << std::endl;
    for (size_t i = 0; i < outSize; ++i) {
        std::cout << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(imgData[i])
                  << " ";
    }*/

    //std::cout << std::dec << std::endl;

    return;// data;
}

RawImageData ImageHandler::_readBarch(const std::string &compressedFilename) {
    std::ifstream file(compressedFilename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file");
    }

    RawImageData img;

    // Read format
    char format[2];
    file.read(format, 2);

    if (format[0] != 'B' || format[1] != 'A') {
        throw std::runtime_error("Invalid file format");
    }
    std::cout<< "format " << format[0] << format[1] << std::endl;

    // Read width
    file.read(reinterpret_cast<char*>(&img.width), sizeof(img.width));
    std::cout<< std::dec << "img.width " << img.width << std::endl;

    // Read height
    file.read(reinterpret_cast<char*>(&img.height), sizeof(img.height));
    std::cout<< std::dec << "img.height " << img.height << std::endl;

    // EmptyRows size = height
    size_t bitCount = img.height;
    size_t byteCount = (bitCount + 7) / 8;
    std::cout<< std::dec << "byteCount " << byteCount << std::endl;

    std::vector<uint8_t> rowBytes(byteCount);
    file.read(reinterpret_cast<char*>(rowBytes.data()), byteCount);

    // Bits to vector<bool>
    std::vector<bool> emptyRows;
    emptyRows.resize(bitCount);

    for (size_t i = 0; i < bitCount; ++i) {
        uint8_t byte = rowBytes[i / 8];
        bool bit = byte & (1 << (7 - (i % 8)));
        emptyRows[i] = bit;
    }

    std::cout << "emptyRows "<< std::endl;
    for (size_t i = 0; i < emptyRows.size(); ++i) {
        std::cout << emptyRows[i] << " ";
    }
    std::cout << std::endl;

    // size of image data
    std::streampos currentPos = file.tellg();
    file.seekg(0, std::ios::end);
    std::streampos endPose = file.tellg();

    std::size_t dataSize = static_cast<std::size_t>(endPose - currentPos);
    std::cout << "dataSize " << dataSize << std::endl;

    // return to current
    file.seekg(currentPos);

    img.data = new unsigned char[img.width*img.height];
    std::cout<<"img.width*img.height " << img.width*img.height << std::endl;
    BitPacker dataFromBarch;
    std::vector<uint8_t> codedBufferFromBarch(dataSize);

    // Read data image
    file.read(reinterpret_cast<char*>(codedBufferFromBarch.data()), dataSize);

    dataFromBarch.setBuffer(std::move(codedBufferFromBarch));

    dataFromBarch.printBinBuffer();
    dataFromBarch.printHexBuffer();

    size_t outSize = 0;
    _decodePixels(dataFromBarch, img.data, emptyRows, outSize);

    createBMPTest("restored_image.bmp", img, outSize);

    return img;
}

void ImageHandler::compressImage(const std::string& inputFilename, const std::string &outputFilename) {
    RawImageData img = _readBMP(inputFilename);
    int width = img.width;
    int height = img.height;

    // empty rows
    std::vector<bool> emptyRows(height, false);
    std::cout << "Empty index" << std::endl;
    for (int j = 0; j < height; ++j) {
        bool empty = true;
        for (int i = 0; i < width; ++i) {
            if (img.data[j * width + i] != 0xFF) {
                empty = false;
                break;
            }
        }
        emptyRows[j] = empty;
    }

    // non empty rows
    BitPacker bitPacker;

    // for now encode only 65th row (first non emty)
    for (int j = 0; j < height; ++j) {
        if (emptyRows[j]) continue; // don`t encode empty rows

        std::cout << std::dec << "Row pixels of " << j << std::endl;
        for (int i = 0; i < width; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')<< static_cast<int>(img.data[j * width + i]) << " ";
        }
        std::cout<<std::endl;

        for (int i = 0; i < width; i += 4) {
            uint8_t block[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // default
            int blockSize = std::min(4, width - i);
            //std::cout<< "blockSize " << blockSize << std::endl;
            for (int k = 0; k < blockSize; ++k) {
                block[k] = img.data[j * width + i + k];
            }

            bool allWhite = true, allBlack = true;
            for (int k = 0; k < blockSize; ++k) {
                if (block[k] != 0xFF) allWhite = false;
                if (block[k] != 0x00) allBlack = false;
            }

            if (allWhite) {
                //std::cout<< "White block " << std::endl;
                bitPacker.pushBit(0); // 4 white
                //bitPacker.printBinBuffer();
            } else if (allBlack) {
                //std::cout<< "Black block " << std::endl;
                bitPacker.pushBits({1, 0}); // 4 black
                //bitPacker.printBinBuffer();
            } else {
                //std::cout<< "Mixed block " << std::endl;
                bitPacker.pushBits({1, 1}); // mixed identifier
                //bitPacker.printBinBuffer();
                for (int k = 0; k < blockSize; ++k) {
                    //std::cout << std::hex <<  static_cast<int>(block[k]) << std::endl;
                    bitPacker.pushByte(block[k]);
                }
                //bitPacker.printBinBuffer();
            }
            // add padding for block <  4
            /*if (blockSize < 4) {
                std::cout<< "Added padding, block size was " << blockSize << std::endl;
                for (int k = blockSize; k < 4; ++k) {
                    bitPacker.pushByte(0xFF);
                }
            }*/
        }
    }
    bitPacker.flush();
    std::cout << "All rows encoded" << std::endl;
    //std::cout << "Row 65 encoded" << std::endl;

    std::cout << std::dec << "bitPacker.getBuffer().size() " << bitPacker.getBuffer().size() << std::endl;
    for (size_t i = 0; i < bitPacker.getBuffer().size(); ++i) {
        std::cout << std::hex <<  static_cast<int>(bitPacker.getBuffer()[i]) << " ";
    }
    std::cout<<std::endl;

    // Write to barch
    writeToFile(outputFilename, emptyRows, bitPacker.getBuffer(), width, height);
}

void ImageHandler::restoreImage(const std::string& compressedFilename, const std::string &restoredFilename) {
    std::cout<< "restoreImage" << std::endl;

    RawImageData img = _readBarch(compressedFilename);

    //createBMPTest(restoredFilename, img, 825);

    return;
}

void ImageHandler::createBMPTest(const std::string &filename, const RawImageData &img, size_t outSize)
{
    std::cout<<"createBMPTest"<< std::endl;
    int width = img.width;
    std::cout<<"width "<< width << std::endl;
    int height = img.height;
    std::cout<<"height "<< height << std::endl;
    int rowSize = ((width + 3) / 4) * 4;
    std::cout<<"rowSize "<< rowSize << std::endl;
    int padding = rowSize - width;
    std::cout<<"padding "<< padding << std::endl;

    std::cout<<"outSize "<< outSize << std::endl;

    // File header
    BitmapFileHeader fileHeader{
        0x4D42, // bfType
        0,      // bfSize
        0,      // bfReserved1
        0,      // bfReserved2
        0       // bfOffBits
    };
    fileHeader.bfOffBits = 14 + 124 + 256*4; // 14 + header + palette
    fileHeader.bfSize = fileHeader.bfOffBits + rowSize * height;

    // DIB header
    BitmapV5Header dibHeader{
        124,      // biSize
        width,    // biWidth
        height,   // biHeight
        1,        // biPlanes
        8,        // biBitCount
        0,        // biImageHandler
        0,        // biSizeImage
        2835,     // biXPelsPerMeter
        2835,     // biYPelsPerMeter
        256,      // biClrUsed
        256,      // biClrImportant
        0, 0, 0, 0,             // masks
        0x73524742,              // bV5CSType
        {0},                     // bV5Endpoints
        0, 0, 0, 0, 0, 0, 0      // gamma, intent, profile, reserved
    };
    dibHeader.biWidth = width;
    dibHeader.biHeight = height;
    dibHeader.biSizeImage = rowSize * height;

    // Open file
    std::ofstream file(filename, std::ios::out | std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot create BMP file");
        }

    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    // Write grayscale palette
    for (int i=0; i<256; ++i) {
        unsigned char color[4] = { (unsigned char)i, (unsigned char)i, (unsigned char)i, 0 };
        file.write(reinterpret_cast<char*>(color), 4);
    }

    // Write pixel data (bottom-up)
    std::vector<unsigned char> pad(padding, 0);
    for (int j=height-1; j>=0; --j) {
        file.write(reinterpret_cast<char*>(img.data + j*width), width);
        if (padding) file.write(reinterpret_cast<char*>(pad.data()), padding);
    }
    file.close();
}

//void ImageHandler::_writeBMP(const std::string& filename, const RawImageData& image) {

//}

