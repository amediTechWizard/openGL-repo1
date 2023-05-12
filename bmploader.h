#ifndef BMPLOADER_H
#define BMPLOADER_H

#include <vector>

std::vector<unsigned char> loadARGB_BMP(const char* imagepath, unsigned int* width, unsigned int* height) {

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    // Actual RGBA data

    /*
    // Open the file
    FILE* file = fopen(imagepath, "rb");
    if (!file) {
        printf("%s could not be opened. Are you in the right directory?\n", imagepath);
        getchar();
        return std::vector<unsigned char>();
    }
    */

    FILE* file;
    errno_t err;
    err = fopen_s(&file, imagepath, "rb");
    if (err != 0) {
        printf("%s could not be opened. Are you in the right directory?\n", imagepath);
        getchar();
        return std::vector<unsigned char>();
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if (fread(header, 1, 54, file) != 54) {
        printf("Not a correct BMP file1\n");
        fclose(file);
        return std::vector<unsigned char>();
    }

    // Read the information about the image
    dataPos = *(int*)&(header[0x0A]);
    imageSize = *(int*)&(header[0x22]);
    *width = *(int*)&(header[0x12]);
    *height = *(int*)&(header[0x16]);
    // A BMP files always begins with "BM"
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file2\n");
        fclose(file);
        return std::vector<unsigned char>();
    }
    // Make sure this is a 32bpp file
    if (*(int*)&(header[0x1E]) != 3) {
        printf("Not a correct BMP file3\n");
        fclose(file);
        return std::vector<unsigned char>();
    }

    // Some BMP files are misformatted, guess missing information
    if (imageSize == 0)    imageSize = (*width) * (*height) * 4; // 4 : one byte for each Red, Green, Blue, Alpha component
    if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

    // Create a buffer
    std::vector<unsigned char> data(imageSize);

    if (dataPos != 54) {
        fread(header, 1, dataPos - 54, file);
    }

    // Read the actual data from the file into the buffer
    fread(data.data(), 1, imageSize, file);

    // Everything is in memory now, the file can be closed.
    fclose(file);

    // Swap Red and Blue components of each pixel to convert from ARGB to RGBA format
    for (unsigned int i = 0; i < imageSize; i += 4) {
        unsigned char temp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = temp;
    }

    return data;
}

#endif // BMPLOADER_H
