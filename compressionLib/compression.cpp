#include "compression.h"
#include <iomanip>
#include <cstring>

RawImageData::~RawImageData() {
    delete[] data;
}

// Write operations
void BitPacker::flush() {
    if (bitIndex > 0) {
        _byteBuffer.push_back(writeByte);
        writeByte = 0;
        bitIndex = 0;
    }
}

void BitPacker::pushBit(bool bit) {
    writeByte |= (static_cast<uint8_t>(bit) << (7 - bitIndex));
    ++bitIndex;
    if (bitIndex == 8) {
        _byteBuffer.push_back(writeByte);
        writeByte = 0;
        bitIndex = 0;
    }
}

void BitPacker::pushBits(const std::vector<bool>& bits) {
    for (bool bit : bits) {
        pushBit(bit);
    }
}

void BitPacker::pushByte(uint8_t byte) {
    for (int i = 7; i >= 0; --i) {
        pushBit((byte >> i) & 1);
    }
}

std::vector<uint8_t> BitPacker::getBuffer() const {
    return _byteBuffer;
}

// Read operations
void BitPacker::setBuffer(std::vector<uint8_t>&& data) {
    _byteBuffer = std::move(data);
    readIndex = 0;
    bitIndex = 0;
}

bool BitPacker::eof() const {
    return readIndex >= _byteBuffer.size();
}

bool BitPacker::readBit() {
    if (eof()) {
        throw std::runtime_error("Attempted to read beyond end of buffer");
    }
    bool bit = (_byteBuffer[readIndex] >> (7 - bitIndex)) & 1;
    ++bitIndex;
    if (bitIndex == 8) {
        bitIndex = 0;
        ++readIndex;
    }
    return bit;
}

uint8_t BitPacker::readBits(int n) {
    if (n > 8 || n < 0) {
        throw std::invalid_argument("Cannot read more than 8 bits");
    }
    uint8_t value = 0;
    for (int i = 0; i < n; ++i) {
        if (eof()) {
            throw std::runtime_error("Unexpected end of buffer while reading bits");
        }
        value <<= 1;
        value |= readBit() ? 1 : 0;
    }
    return value;
}

void BitPacker::printBinBuffer() const {
    std::cout << "Buffer in bin format: " << std::endl;
    for (auto byte : _byteBuffer) {
        std::cout << std::bitset<8>(byte);
    }
    std::cout << std::endl;
}

void BitPacker::printHexBuffer() const {
    std::cout << "Buffer in hex format: " << std::endl;
    for (size_t k = 0; k < _byteBuffer.size(); ++k) {
        std::cout << std::hex << static_cast<int>(_byteBuffer[k]) << " ";
    }
    std::cout << std::endl;
}

void ImageHandler::writeToFile(const std::string& filename,
                 const std::vector<bool>& emptyRows,
                 const std::vector<uint8_t>& compressedData,
                 uint32_t width, uint32_t height) {

    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Can't open file to write!" << std::endl;
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
    size_t bitCount = emptyRows.size();
    size_t byteCount = (bitCount + 7) / 8; // 8 bits = 1 byte
    std::vector<uint8_t> rowIndex(byteCount, 0);

    for (size_t i = 0; i < bitCount; ++i) {
        if (emptyRows[i]) {
            rowIndex[i / 8] |= (1 << (7 - (i % 8))); // set bit
        }
    }

    file.write(reinterpret_cast<const char*>(rowIndex.data()), rowIndex.size());

    // 5. Write size of compressed data (4 bytes)
    uint32_t compressedSize = static_cast<uint32_t>(compressedData.size());
    file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));

    // 6. Write encoded image data (vector<uint8_t>)
    file.write(reinterpret_cast<const char*>(compressedData.data()), compressedData.size());

    file.close();
    std::cout << "Successfully created barch file" << std::endl;
}

RawImageData ImageHandler::_readBMP(const std::string &filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file");
    }

    RawImageData img{};
    BitmapFileHeader fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));

    if (fileHeader.bfType != 0x4D42) {
        throw std::runtime_error("Not a BMP file");
    }

    // Read DIB header size
    uint32_t dibSize;
    file.read(reinterpret_cast<char*>(&dibSize), sizeof(dibSize));

    // Go back to read proper header
    file.seekg(-4, std::ios::cur);

    if (dibSize == 124) { // BITMAPV5HEADER
        BitmapV5Header dibHeader;
        file.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

        std::cout << "Width: " << dibHeader.biWidth << "\n";
        std::cout << "Height: " << dibHeader.biHeight << "\n";
        std::cout << "BitCount: " << dibHeader.biBitCount << "\n";

        img.width = static_cast<int>(dibHeader.biWidth);
        img.height = static_cast<int>(dibHeader.biHeight);
    } else {
        throw std::runtime_error("Unsupported BMP header type");
    }

    // Allocate memory for image data (without padding)
    img.data = new unsigned char[static_cast<size_t>(img.width) * static_cast<size_t>(img.height)];

    // Calculate padding for BMP rows
    int padding = (4 - (img.width % 4)) % 4;

    // Seek to pixel data
    file.seekg(fileHeader.bfOffBits);

    // Read image data (bottom-up, but we'll store it top-down for our purposes)
    std::vector<unsigned char> row(img.width + padding);
    for (int j = img.height - 1; j >= 0; --j) {
        file.read(reinterpret_cast<char*>(row.data()), img.width + padding);

        // Copy only the pixel data (without padding)
        std::memcpy(img.data + static_cast<size_t>(j) * static_cast<size_t>(img.width), row.data(), img.width);
    }
    file.close();

    return img;
}

void ImageHandler::_decodePixels(BitPacker& bitPacker, unsigned char* imgData,
                                const std::vector<bool>& emptyRows,
                                int width, int height, size_t& outSize) {
    std::cout << "_decodePixels" << std::endl;
    std::vector<unsigned char> temp;
    int totalPixels = width * height;
    int decodedPixels = 0;

    // Счетчик для отладки
    int blockCount = 0;
    int mixedBlocks = 0;
    int whiteBlocks = 0;
    int blackBlocks = 0;
    int nonEmptyRows = 0;

    for (int row = 0; row < height; ++row) {
        if (emptyRows[row]) {
            // Empty row - fill with white pixels
            std::cout << "Row " << row << " is empty, filling with white pixels" << std::endl;
            temp.insert(temp.end(), width, 0xFF);
            decodedPixels += width;
        } else {
            nonEmptyRows++;
            std::cout << "Row " << row << " is non-empty, decoding..." << std::endl;
            // Non-empty row - decode compressed data
            int rowPixels = 0;
            while (rowPixels < width) {
                int pixelsToWrite = std::min(4, width - rowPixels);

                // Проверяем, что мы не читаем за границы буфера
                if (bitPacker.eof()) {
                    std::cerr << "Unexpected EOF in non-empty row " << row
                              << " at pixel " << rowPixels
                              << " (rowPixels: " << rowPixels
                              << ", width: " << width << ")" << std::endl;
                    std::cerr << "Non-empty rows decoded so far: " << nonEmptyRows << std::endl;
                    std::cerr << "Total decoded pixels: " << decodedPixels << std::endl;
                    throw std::runtime_error("Unexpected EOF during decompression");
                }

                bool firstBit = bitPacker.readBit();
                blockCount++;

                if (!firstBit) {
                    // 0 → 4 white
                    temp.insert(temp.end(), pixelsToWrite, 0xFF);
                    decodedPixels += pixelsToWrite;
                    rowPixels += pixelsToWrite;
                    whiteBlocks++;
                } else {
                    bool secondBit = bitPacker.readBit();
                    if (!secondBit) {
                        // 10 → 4 black
                        temp.insert(temp.end(), pixelsToWrite, 0x00);
                        decodedPixels += pixelsToWrite;
                        rowPixels += pixelsToWrite;
                        blackBlocks++;
                    } else {
                        // 11 → 4 mixed
                        mixedBlocks++;
                        for (int i = 0; i < pixelsToWrite; ++i) {
                            // Дополнительная проверка безопасности
                            if (bitPacker.eof()) {
                                std::cerr << "Unexpected EOF in mixed block at non-empty row "
                                          << row << ", col " << rowPixels << std::endl;
                                throw std::runtime_error("Unexpected EOF while reading mixed block");
                            }

                            uint8_t color = bitPacker.readBits(8);
                            temp.push_back(color);
                        }
                        decodedPixels += pixelsToWrite;
                        rowPixels += pixelsToWrite;
                    }
                }
            }
        }
    }

    std::cout << "Blocks: " << blockCount
              << " (White: " << whiteBlocks
              << ", Black: " << blackBlocks
              << ", Mixed: " << mixedBlocks << ")" << std::endl;
    std::cout << "Non-empty rows: " << nonEmptyRows << std::endl;
    std::cout << "Decoded pixels: " << decodedPixels
              << " / " << totalPixels << std::endl;

    outSize = static_cast<size_t>(totalPixels);
    size_t copySize = std::min(temp.size(), static_cast<size_t>(outSize));

    if (copySize < outSize) {
        std::cerr << "Warning: decoded " << copySize
                  << " pixels, expected " << outSize << std::endl;
    }

    std::memcpy(imgData, temp.data(), copySize);
}

RawImageData ImageHandler::_readBarch(const std::string &compressedFilename) {
    std::ifstream file(compressedFilename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file");
    }

    RawImageData img{};

    // Read format
    char format[2];
    file.read(format, 2);
    if (format[0] != 'B' || format[1] != 'A') {
        throw std::runtime_error("Invalid file format");
    }

    // Read width and height
    file.read(reinterpret_cast<char*>(&img.width), sizeof(img.width));
    file.read(reinterpret_cast<char*>(&img.height), sizeof(img.height));

    // EmptyRows size = height
    size_t bitCount = static_cast<size_t>(img.height);
    size_t byteCount = (bitCount + 7) / 8;

    std::vector<uint8_t> rowBytes(byteCount);
    file.read(reinterpret_cast<char*>(rowBytes.data()), byteCount);

    // Convert bytes to vector<bool>
    std::vector<bool> emptyRows;
    emptyRows.resize(bitCount);

    for (size_t i = 0; i < bitCount; ++i) {
        uint8_t byte = rowBytes[i / 8];
        bool bit = byte & (1 << (7 - (i % 8)));
        emptyRows[i] = bit;
    }

    // Count non-empty rows for debugging
    int nonEmptyRows = 0;
    for (bool empty : emptyRows) {
        if (!empty) nonEmptyRows++;
    }
    std::cout << "Number of non-empty rows: " << nonEmptyRows << std::endl;

    // Read compressed data size
    uint32_t compressedSize;
    file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
    if (!file) {
        throw std::runtime_error("Failed to read compressed size");
    }

    std::cout << "Image dimensions: " << img.width << "x" << img.height << std::endl;
    std::cout << "Compressed data size: " << compressedSize << " bytes" << std::endl;

    // Allocate memory for image data
    img.data = new unsigned char[static_cast<size_t>(img.width) * static_cast<size_t>(img.height)];

    // Read compressed data
    BitPacker bitPacker;
    std::vector<uint8_t> codedBuffer(compressedSize);
    file.read(reinterpret_cast<char*>(codedBuffer.data()), compressedSize);
    if (!file) {
        throw std::runtime_error("Failed to read compressed data");
    }
    bitPacker.setBuffer(std::move(codedBuffer));

    size_t outSize = 0;
    _decodePixels(bitPacker, img.data, emptyRows, img.width, img.height, outSize);

    file.close();
    return img;
}

void ImageHandler::compressImage(const std::string& inputFilename, const std::string &outputFilename) {
    RawImageData img = _readBMP(inputFilename);
    int width = img.width;
    int height = img.height;

    // Determine empty rows
    std::vector<bool> emptyRows(height, false);
    int nonEmptyRows = 0;
    for (int j = 0; j < height; ++j) {
        bool empty = true;
        for (int i = 0; i < width; ++i) {
            if (img.data[j * width + i] != 0xFF) {
                empty = false;
                break;
            }
        }
        emptyRows[j] = empty;
        if (!empty) nonEmptyRows++;
    }

    std::cout << "Number of non-empty rows: " << nonEmptyRows << std::endl;

    // Compress non-empty rows
    BitPacker bitPacker;

    for (int j = 0; j < height; ++j) {
        if (emptyRows[j]) continue; // Skip empty rows

        for (int i = 0; i < width; i += 4) {
            uint8_t block[4] = {0xFF, 0xFF, 0xFF, 0xFF};
            int blockSize = std::min(4, width - i);

            for (int k = 0; k < blockSize; ++k) {
                block[k] = img.data[j * width + i + k];
            }

            bool allWhite = true, allBlack = true;
            for (int k = 0; k < blockSize; ++k) {
                if (block[k] != 0xFF) allWhite = false;
                if (block[k] != 0x00) allBlack = false;
            }

            if (allWhite) {
                bitPacker.pushBit(0); // 4 white
            } else if (allBlack) {
                bitPacker.pushBits({1, 0}); // 4 black
            } else {
                bitPacker.pushBits({1, 1}); // mixed identifier
                for (int k = 0; k < blockSize; ++k) {
                    bitPacker.pushByte(block[k]);
                }
            }
        }
    }

    bitPacker.flush();

    // Write to barch file
    writeToFile(outputFilename, emptyRows, bitPacker.getBuffer(), width, height);
}

void ImageHandler::restoreImage(const std::string& compressedFilename, const std::string &restoredFilename) {
    std::cout << "restoreImage" << std::endl;

    try {
        RawImageData img = _readBarch(compressedFilename);
        createBMPTest(restoredFilename, img);
    } catch (const std::exception& e) {
        std::cerr << "Error during restore: " << e.what() << std::endl;
        throw;
    }
}

void ImageHandler::createBMPTest(const std::string &filename, const RawImageData &img) {
    std::cout << "createBMPTest" << std::endl;
    int width = img.width;
    int height = img.height;
    int rowSize = ((width + 3) / 4) * 4;
    int padding = rowSize - width;

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
        0,        // biCompression
        (uint32_t)rowSize * height, // biSizeImage
        2835,     // biXPelsPerMeter
        2835,     // biYPelsPerMeter
        256,      // biClrUsed
        256,      // biClrImportant
        0, 0, 0, 0, // masks
        0x73524742,  // bV5CSType
        {0},         // bV5Endpoints
        0, 0, 0, 0, 0, 0, 0 // gamma, intent, profile, reserved
    };

    // Open file
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot create BMP file");
    }

    // Write headers
    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    // Write grayscale palette
    for (int i = 0; i < 256; ++i) {
        unsigned char color[4] = { static_cast<unsigned char>(i),
                                  static_cast<unsigned char>(i),
                                  static_cast<unsigned char>(i), 0 };
        file.write(reinterpret_cast<char*>(color), 4);
    }

    // Write pixel data (bottom-up)
    std::vector<unsigned char> pad(padding, 0);
    for (int j = height - 1; j >= 0; --j) {
        file.write(reinterpret_cast<const char*>(img.data + j * width), width);
        if (padding > 0) {
            file.write(reinterpret_cast<const char*>(pad.data()), padding);
        }
    }
    file.close();
}

void ImageHandler::_writeBMP(const std::string& filename, const RawImageData &image) {
    createBMPTest(filename, image);
}
