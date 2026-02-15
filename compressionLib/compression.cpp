#include "compression.h"
#include <iomanip>

Compression::Compression()
{
}

RawImageData::~RawImageData() {
    delete[] data;
}

void Compression::_writeEncodedPixelSequence(const RawImageData& image, std::ofstream& file, int count, unsigned char color, bool isSameColor, int rowStart, int blockNumber) {
}


void Compression::_encodeNonEmptyRow(const RawImageData& image, std::ofstream& file, int rowIndex, int rowStart) {
}

void Compression::writeToFile(const std::string& filename,
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
    size_t byteCount = ceil(bitCount / 8);// every 8bit = 2 bytes
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

RawImageData Compression::_readBMP(const std::string &filename) {
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

    createBMPTest("outputBMP.bmp", img);
    return img;
}

void Compression::compressImage(const std::string& inputFilename, const std::string &outputFilename) {
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

    // non emptyrows
    std::vector<uint8_t> encodedData;
    BitPacker packer;

    int j = 65; // for now encode only 65th row (non emty)
    //for (int j = 0; j < height; ++j) {
        //if (emptyRows[j]) continue; // don`t encode empty rows

        std::cout << "Row 65 pixels" << std::endl;
        for (int i = 0; i < width; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')<< static_cast<int>(img.data[j * width + i]) << " ";
        }
        std::cout<<std::endl;

        for (int i = 0; i < width; i += 4) {
            uint8_t block[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // default
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
                std::cout<< "White block " << std::endl;
                packer.pushBit(0); // 4 white
                packer.printBits();
            } else if (allBlack) {
                std::cout<< "Black block " << std::endl;
                packer.pushBits({1, 0}); // 4 black
                packer.printBits();
            } else {
                std::cout<< "Mixed block " << std::endl;
                packer.pushBits({1, 1}); // mixed identifier
                packer.printBits();
                for (int k = 0; k < blockSize; ++k) {
                    std::cout << std::hex <<  static_cast<int>(block[k]) << std::endl;
                    packer.pushByte(block[k]);
                }
                packer.printBits();
            }
        }
    //}
    std::cout << "Row 65 encoded" << std::endl;

    std::cout << std::dec << "packer.getBuffer().size() " << packer.getBuffer().size() << std::endl;
    for (size_t i = 0; i < packer.getBuffer().size(); ++i) {
        std::cout << std::hex <<  static_cast<int>(packer.getBuffer()[i]) << " ";
    }
    std::cout<<std::endl;

    // Write to barch
    writeToFile(outputFilename, emptyRows, packer.getBuffer(), width, height);
}

void Compression::_decodeNonEmptyRow(std::ifstream& file, RawImageData &image, int rowIndex) {
    /*int index = rowIndex * image.width;

    std::cout<<"_decodeNonEmptyRow"<<std::endl;
    int i = 0;


    while ((i < image.width) && ((image.width - i) >= 4)) {
        // process all full blocks
        unsigned char code;
        file >> code;
        if (code == '0') {
            //std::cout << "code == 0 " << std::endl;
            // 4 white pixels
            for (int j = 0; j < 4; ++j) {
                image.data[index++] = 0xFF; // white pixel
                std::cout << "255 ";
                i++;
            }
        } else if (code == '1') {
            file >> code;
            //std::cout << " i =  " << i<<" ";
            //std::cout << "code == 1 " << std::endl;
            if (code == '0') {
                //std::cout << " next code == 0 " << std::endl;
                // 4 black pixels
                for (int j = 0; j < 4; ++j) {
                    image.data[index++] = 0x00; // black pixel
                    //std::cout << "0 ";
                    i++;
                }
            } else if (code == '1') {
                // Sequence of other colors
                //std::cout << " next code == 1 " << std::endl;
                unsigned char color;
                for (int j = 0; j < 4; ++j) {
                    file.read(reinterpret_cast<char*>(&color), sizeof(unsigned char));
                    //std::cout<<index;
                    image.data[index++] = color;
                    std::cout << color<< "* ";
                    i++;
                }
            }
        } else {
            std::cout<<"Error code: "<<(int)code<<std::endl; //127 code
            file >> code;
                        continue;
            //std::cout<<" Next code: "<<(int)code<<std::endl;
        }
    }

    if ((image.width - i) < 4) {
        unsigned char color;
        while (i < image.width) {
            file.read(reinterpret_cast<char*>(&color), sizeof(unsigned char));
            image.data[index++] = color;
            std::cout << color<< " ";
            i++;
        }
    }
    std::cout<<std::endl;
    std::cout<<index<<std::endl;
    std::cout<<std::endl;

    std::cout <<" rowdatastruct from barch"<<std::endl;
    for (int i = 0; i < image.width; ++i) {
        unsigned char color = image.data[rowIndex*image.width + i];
        std::cout<<(int)color << " ";
        std::cout<< static_cast<int>(image.data[rowIndex+i])<<" ";
    }
    std::cout <<std::endl;*/
}

void Compression::restoreImage(const std::string& filename) {
    /*std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for reading." << std::endl;
        return;
    }

    // Check identifier
    char identifier[2];
    file >> identifier[0];
    file >> identifier[1];

    if (identifier[0] != 'B' && identifier[1] != 'A') {
        std::cerr << "Invalid file format." << std::endl;
        return;
    }

    std::cout << std::endl;

    RawImageData image;
    // Read width and height
    file.read(reinterpret_cast<char*>(&image.width), sizeof(int));
    file.read(reinterpret_cast<char*>(&image.height), sizeof(int));
    std::cout << "width: " << image.width << std::endl;
    std::cout << "height: " << image.height << std::endl;

    // Allocate memory for image data
    image.data = new unsigned char[image.width * image.height];
    std::vector<int> emptyRows;
    std::vector<int> nonEmptyRows;

    // Read empty and non-empty row indexes
    int emptyRowCount, nonEmptyRowCount;
    file.read(reinterpret_cast<char*>(&emptyRowCount), sizeof(int));
    std::cout << "emptyRowCount: " << emptyRowCount << std::endl;

    for (int i = 0; i < emptyRowCount; ++i) {
        int rowIndex;
        file.read(reinterpret_cast<char*>(&rowIndex), sizeof(int));
        //write to vector EmptyRowCount
        emptyRows.push_back(rowIndex);
        // Assuming all pixels in empty rows are 0xFF (white)
        for (int pixelPos = 0; pixelPos < image.width; ++pixelPos) {
            //std::cout<<"rowIndex: "<<rowIndex<<std::endl;
            image.data[rowIndex * image.width + pixelPos] = 0xFF;
        }
    }
    //std::cout<<std::endl;
    //std::cout<<"emptyRows.size(): "<<emptyRows.size()<<std::endl;

    file.read(reinterpret_cast<char*>(&nonEmptyRowCount), sizeof(int));
    std::cout << "nonEmptyRowCount " << nonEmptyRowCount << std::endl;
    for (int i = 0; i < nonEmptyRowCount; ++i) { //nonEmptyRowCount
        int rowIndex;
        file.read(reinterpret_cast<char*>(&rowIndex), sizeof(int));
        //write to vector nonEmptyRowCount
        nonEmptyRows.push_back(rowIndex);
        //std::cout<<"rowIndex: "<<rowIndex<<std::endl;
        //_decodeNonEmptyRow(file, image, rowIndex);
        //c++;
        //if (c>=6) {
        //    break;
        //}
    }

    int c=0;
    for (const auto& row : nonEmptyRows) {
        //int rowIndex;
        //file.read(reinterpret_cast<char*>(&rowIndex), sizeof(int));
        //std::cout<<"row: "<<row<<std::endl;
        _decodeNonEmptyRow(file, image, row);
        c++;
        //if (c>=6) {
        //            break;
        //        }

    }

    file.close();

    // Create grayscale BMP image
    //_writeBMP("restored_image.bmp", image);

    // Free allocated memory
    //delete[] image.data;*/
}

void Compression::createBMPTest(const std::string &filename, const RawImageData &img)
{
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

void Compression::_writeBMP(const std::string& filename, const RawImageData& image) {
    /*std::ofstream file(filename, std::ios::binary);
    std::cout << "here3 " << std::endl;
    if (!file) {
        std::cerr << "Error opening file for writing BMP." << std::endl;
        return;
    }

    // BMP file header
    const int BMP_FILE_HEADER_SIZE = 14;
    const int BMP_INFO_HEADER_SIZE = 40;

    int file_size = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE + image.width * image.height;
    int pixel_data_offset = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE;

    // BMP file header
    file.put('B');
    file.put('M');
    file.write(reinterpret_cast<const char*>(&file_size), 4);
    file.write(reinterpret_cast<const char*>(&pixel_data_offset), 4);
    file.write(reinterpret_cast<const char*>(&BMP_INFO_HEADER_SIZE), 4);
    file.write(reinterpret_cast<const char*>(&image.width), 4);
    file.write(reinterpret_cast<const char*>(&image.height), 4);
    file.write(reinterpret_cast<const char*>(&image.width), 4); // biHeight (same as height for upside-down BMP)
    file.write("\x01\x00\x08\x00", 4); // biPlanes, biBitCount
    file.write("\x00\x00\x00\x00", 4); // biCompression, biSizeImage
    file.write("\x00\x00\x00\x00", 4); // biXPelsPerMeter, biYPelsPerMeter
    file.write("\x00\x00\x00\x00\x00\x00\x00\x00", 8); // biClrUsed, biClrImportant

    // BMP pixel data (grayscale)
    for (int i = 0; i < image.height; ++i) {
        for (int j = 0; j < image.width; ++j) {
            unsigned char pixel = image.data[i * image.width + j];
            file.write(reinterpret_cast<const char*>(&pixel), 1);
        }
    }

    file.close();*/
}

