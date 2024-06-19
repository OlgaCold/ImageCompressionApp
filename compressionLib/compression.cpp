#include "compression.h"

Compression::Compression()
{
}

RawImageData::~RawImageData() {
    delete[] data;
}

void Compression::_writeEncodedPixelSequence(const RawImageData& image, std::ofstream& file, int count, unsigned char color, bool isSameColor, int rowStart, int blockNumber) {
    //std::cout<<"_writeEncodedPixelSequence"<<std::endl;
    //std::cout<<" count: "<<count<<" color: "<<(int)color<<std::endl;
    //std::cout<<" isSameColor: "<<isSameColor<<";";
    if (isSameColor) {
        if (color == 0xFF) { // 4 white pixels
            file << 0;
            std::cout<<"0 ";
        } else if (color == 0x00) { // 4 black pixels
            file << 10;
            std::cout<<"10 ";
        }
        return;
    }

    // another color sequence
    file << 11;
    std::cout<<"11 ";
    //std::cout<<" count "<<count;
    for (int i = 0; i < count; ++i) {
        //std::cout<<" rowStart + blockNumber*4 + count: "<<rowStart + blockNumber*4 + i;
        //std::cout<<" image.data[rowStart + blockNumber*4 + i]: "<<(int)image.data[rowStart + blockNumber*4 + i]<< " ";
        file.write(reinterpret_cast<const char*>(&image.data[rowStart + blockNumber*4 + i]), sizeof(unsigned char));
        std::cout<<(int)image.data[rowStart + blockNumber*4 + i]<<" ";
    }
}


void Compression::_encodeNonEmptyRow(const RawImageData& image, std::ofstream& file, int rowIndex, int rowStart) {
    std::cout<<"_encodeNonEmptyRow"<<std::endl;
    int count = 0;
    int blockNumber = 0;
    bool isSameColor = true;
    unsigned char color = image.data[rowStart];
    std::cout<<"rowStart: "<<rowStart<<std::endl;


    for (int i = 0; i < image.width; ++i) {
        //std::cout<<" i: "<<i<< " ";
        //std::cout<<" image.data[rowStart + i]: "<<(int)image.data[rowStart + i]<<" color: "<<(int)color;
        if (image.data[rowStart + i] != color) {
            isSameColor = false;
        }
        if (count == 4 || (image.width - i) < 4) { // encode block size or rest of pixels (1 pixel - 1 color)
            _writeEncodedPixelSequence(image, file, count, color, isSameColor, rowStart, blockNumber);
            count = 1;
            blockNumber++; //from 0
            color = image.data[rowStart + i];
            isSameColor = true;
            continue;
        }
        count++;
    }
    std::cout <<std::endl;
}

RawImageData Compression::_readBMP(const std::string &filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file");
    }

    BMPHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeof(header));
    std::cout<<"header size: "<<sizeof(header)<<std::endl;;

    if (header.bitsPerPixel != 8 || header.compression != 0) {
        throw std::runtime_error("Unsupported BMP format. Only 8-bit grayscale BMPs without compression are supported.");
    }

    RawImageData imageData;
    imageData.width = header.width;
    imageData.height = header.height;

    int dataSize = header.width * header.height;
    imageData.data = new unsigned char[dataSize];

    file.seekg(header.dataOffset, std::ios::beg);
    file.read(reinterpret_cast<char *>(imageData.data), dataSize);
    file.seekg(header.dataOffset + dataSize, std::ios::beg);

    file.close();


    /*for (int j = 0; j < imageData.height; j++) {
        int rowStart = j * imageData.width;
        for (int i = 0; i < imageData.width; i++) {
            std::cout<<(int)imageData.data[rowStart + i]<<" ";
        }
        std::cout<<std::endl;
    }*/

    return imageData;
}

void Compression::compressImage(const std::string& filename) {
    std::ofstream file("compressed_image.barch", std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    // Identifier
    file << "BA";
    RawImageData image = _readBMP(filename);

    // Width and height
    file.write(reinterpret_cast<const char*>(&image.width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&image.height), sizeof(int));
    std::cout<<image.width<<" "<<image.height<<std::endl;

    // Encode rows
    std::vector<int> emptyRows;
    std::vector<int> nonEmptyRows;
    //int ind = 0;
    for (int j = 0; j < image.height; ++j) {
        int rowStart = j * image.width;
        bool isEmpty = true;
        for (int i = 0; i < image.width; ++i) {
            if ((int)image.data[rowStart + i] != 0xFF) {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty) {
            std::cout<<std::endl;
            emptyRows.push_back(j);
        } else {
            nonEmptyRows.push_back(j);
        }
    }

    // Write indexes of empty and non-empty rows
    int emptyRowCount = emptyRows.size();
    std::cout<<"emptyRowCount: "<<emptyRowCount<<std::endl;
    file.write(reinterpret_cast<const char*>(&emptyRowCount), sizeof(int));
    for (int row : emptyRows) {
        file.write(reinterpret_cast<const char*>(&row), sizeof(int));
    }

    int nonEmptyRowCount = nonEmptyRows.size();
    std::cout<<"nonEmptyRowCount: "<<nonEmptyRowCount<<std::endl;
    file.write(reinterpret_cast<const char*>(&nonEmptyRowCount), sizeof(int));
    for (int row : nonEmptyRows) {
        file.write(reinterpret_cast<const char*>(&row), sizeof(int));
    }

    // write non empty rows data
    for (const auto& row : nonEmptyRows) {

        for (int i = 0; i < image.width; ++i) {
            unsigned char color = image.data[row*image.width + i];
            std::cout<<(int)color << " ";
        }
        // Encode non-empty row
        _encodeNonEmptyRow(image, file, row, row*image.width);
    }

    file.close();
}

/*void Compression::_decodeNonEmptyRow(std::ifstream& file, RawImageData& image, int rowIndex) {
    int index = rowIndex * image.width;
    //std::cout<<image.width<<std::endl;
    int i = 0;
    while (i < image.width) {
        //std::cout << "i " << i<<std::endl;

        unsigned char code;
        file >> code;
        if (code == 0) {
            //std::cout << "code == 0 " << std::endl;
            // 4 white pixels
            for (int j = 0; j < 4; ++j) {
                image.data[index++] = 0xFF; // white pixel
                i++;
            }
        } else if (code == 1) {
            file >> code;
            if (code == 0) {
                //std::cout << "code == 10 " << std::endl;
                // 4 black pixels
                image.data[index++] = 0x00; // black pixel
                i++;
                for (int j = 1; j < 4; ++j) {
                    image.data[index++] = 0x00; // black pixel
                    i++;
                }
            } else if (code == 1) {
                //std::cout << "code == 11 " << std::endl;
                // Sequence of other colors
                unsigned char color;
                file.read(reinterpret_cast<char*>(&color), sizeof(unsigned char));
                image.data[index++] = color;
                i++;
                for (int j = 1; j < 4; ++j) {
                    file >> code;
                    if (code == 0) {
                        image.data[index++] = color;
                        i++;
                    } else {
                        file.read(reinterpret_cast<char*>(&color), sizeof(unsigned char));
                        image.data[index++] = color;
                        i++;
                    }
                }
            }
        }
    }
}

void Compression::restoreImage(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
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

    RawImageData* image = new RawImageData();
    // Read width and height
    file.read(reinterpret_cast<char*>(&image->width), sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(&image->height), sizeof(unsigned int));
    std::cout << "image->width " << image->width << std::endl;
    std::cout << "image->height " << image->height << std::endl;

    // Allocate memory for image data
    image->data = new unsigned char[image->width * image->height];

    // Read empty and non-empty row indexes
    int emptyRowCount, nonEmptyRowCount;
    file.read(reinterpret_cast<char*>(&emptyRowCount), sizeof(int));
    std::cout << "emptyRowCount " << emptyRowCount << std::endl;

    for (int i = 0; i < emptyRowCount; ++i) {
        int rowIndex;
        file.read(reinterpret_cast<char*>(&rowIndex), sizeof(int));
        std::cout<<"emptyRowCount "<<std::endl;
        std::cout<<"rowIndex: "<<rowIndex<<std::endl;
        // Assuming all pixels in empty rows are 0xFF (white)
        for (int j = 0; j < image->width; ++j) {
            image->data[rowIndex * image->width + j] = 0xFF;
        }
    }

    file.read(reinterpret_cast<char*>(&nonEmptyRowCount), sizeof(int));
    std::cout << "nonEmptyRowCount " << nonEmptyRowCount << std::endl;
    for (int i = 0; i < nonEmptyRowCount; ++i) {
        int rowIndex;
        file.read(reinterpret_cast<char*>(&rowIndex), sizeof(int));
        std::cout<<"nonEmpty: "<<i<<std::endl;
        std::cout<<"rowIndex: "<<rowIndex<<std::endl;
        _decodeNonEmptyRow(file, *image, rowIndex);
    }

    file.close();

    // Create grayscale BMP image
    _saveGrayscaleBMP("restored_image.bmp", *image);

    // Free allocated memory
    delete[] image->data;
}

void Compression::_saveGrayscaleBMP(const std::string& filename, const RawImageData& image) {
    std::ofstream file(filename, std::ios::binary);
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

    file.close();
}*/
