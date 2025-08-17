/*-----------------------------------------------------------------------------
The MIT License

Copyright © 2025-present Hillel Steinberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------*/

// C++ and STL includes
#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

// Include libraw for DNG import
#include <libraw/libraw.h>

// Qt includes
#include <QImage>
#include <QImageReader>
#include <QDataStream>
#include <QFileInfo>
#include <QString>
#include <QColor>
#include <QFile>

#include "image_to_cubemap.h"

//
// Function to load a DNG using libraw as a QImage
//
QImage loadDNG(const QString& path) {

    // Open the raw DNG file
    LibRaw rawProcessor;
    if (rawProcessor.open_file(path.toUtf8().data()) != LIBRAW_SUCCESS)
        return QImage();

    // Unpack DNG
    rawProcessor.unpack();
    rawProcessor.dcraw_process();

    // Get a handle to the image data
    libraw_processed_image_t* image = rawProcessor.dcraw_make_mem_image();

    // Is this an RGB 8-bits per component DNG?
    if (!image || image->colors != 3 || image->bits != 8)
        return QImage();

    // Yes, create a new QImage with the DNG data
    QImage qimg(image->width, image->height, QImage::Format_RGB888);
    memcpy(qimg.bits(), image->data, image->height * image->width * 3);

    // Cleanup
    LibRaw::dcraw_clear_mem(image);
    rawProcessor.recycle();
    return qimg;
}

//
// Function to write provided QImage as a DDS file to the save_file_path provided
//
bool writeCubemapToDDS(const QImage& cubemapImage, const QString& save_file_path) {

    std::ofstream file(save_file_path.toStdString(), std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Could not open file for writing: " << save_file_path.toStdString() << std::endl;
        return false;
    }

    int outW = cubemapImage.width();
    int edge = outW / 4;
    int bytesPerPixel = 4; // For RGBA8888

    // 1. Define and populate the header
    DDS_HEADER header = {};
    header.dwSize = sizeof(DDS_HEADER);
    header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_PITCH;
    header.dwHeight = edge;
    header.dwWidth = edge;
    header.dwPitchOrLinearSize = edge * bytesPerPixel;
    header.dwMipMapCount = 1; // No mipmaps for this example
    header.ddspf.dwSize = sizeof(DDS_PIXELFORMAT);
    header.ddspf.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    header.ddspf.dwRGBBitCount = 32;
    header.ddspf.dwRBitMask = 0x00FF0000;
    header.ddspf.dwGBitMask = 0x0000FF00;
    header.ddspf.dwBBitMask = 0x000000FF;
    header.ddspf.dwABitMask = 0xFF000000;
    header.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
    header.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_ALLFACES;

    // 2. Write the magic number and header
    file.write(reinterpret_cast<const char*>(&DDS_MAGIC), sizeof(quint32));
    file.write(reinterpret_cast<const char*>(&header), sizeof(DDS_HEADER));

    // 3. Extract and write pixel data for each face
    // DDS cubemap face order: +X, -X, +Y, -Y, +Z, -Z
    
    // We'll use our previous logic to extract faces but convert to raw data
    // The QImage format needs to be compatible with RGBA8888
    const QImage& img = cubemapImage.convertToFormat(QImage::Format_RGBA8888);

    // +X (Right)
    QImage facePX = img.copy(2 * edge, edge, edge, edge);
    file.write(reinterpret_cast<const char*>(facePX.constBits()), facePX.sizeInBytes());

    // -X (Left)
    QImage faceNX = img.copy(0, edge, edge, edge);
    file.write(reinterpret_cast<const char*>(faceNX.constBits()), faceNX.sizeInBytes());

    // +Y (Top)
    QImage facePY = img.copy(edge, 0, edge, edge);
    file.write(reinterpret_cast<const char*>(facePY.constBits()), facePY.sizeInBytes());

    // -Y (Bottom)
    QImage faceNY = img.copy(edge, 2 * edge, edge, edge);
    file.write(reinterpret_cast<const char*>(faceNY.constBits()), faceNY.sizeInBytes());
    
    // +Z (Front)
    QImage facePZ = img.copy(edge, edge, edge, edge);
    file.write(reinterpret_cast<const char*>(facePZ.constBits()), facePZ.sizeInBytes());

    // -Z (Back)
    QImage faceNZ = img.copy(3 * edge, edge, edge, edge);
    file.write(reinterpret_cast<const char*>(faceNZ.constBits()), faceNZ.sizeInBytes());

    // Done writing DDS
    file.close();

    return true;
}

// Convert output image coordinates to 3D coordinates
// This function maps a pixel in a specific face of the cubemap to a 3D vector.
// The face is determined by the `face` parameter.
void outImgToXYZ(int i, int j, int face, int edge, float& x, float& y, float& z) {

    // Correctly scale i and j to a -1 to 1 range for each face
    // i and j are relative to the top-left of the current face
    const float a = 2.0f * (float)i / (float)edge - 1.0f;
    const float b = 2.0f * (float)j / (float)edge - 1.0f;

    switch (face) {

        // We'll use a standard cubemap face order:
        // 0: right (+X)
        // 1: left (-X)
        // 2: top (+Y)
        // 3: bottom (-Y)
        // 4: front (+Z)
        // 5: back (-Z)

        // Right (+X) face
        case 0: x = 1.0f; y = -b; z = -a; break; 
        
        // Left (-X) face
        case 1: x = -1.0f; y = -b; z = a; break; 
        
        // Top (+Y) face
        case 2: x = a; y = 1.0f; z = b; break; 
        
        // Bottom (-Y) face
        case 3: x = a; y = -1.0f; z = -b; break; 
        
        // Front (+Z) face
        case 4: x = a; y = -b; z = 1.0f; break; 
        
        // Back (-Z) face
        case 5: x = -a; y = -b; z = -1.0f; break;
        
        default: x = y = z = 0; break;
    }
}

// Main conversion logic
void convertEquirectToCubemap(const QImage& image_in, QImage& image_out) {

    const int inW = image_in.width();
    const int inH = image_in.height();
    const int outW = image_out.width();
    const int outH = image_out.height();
    
    // The cubemap output image should be a 4x3 grid of faces,
    // so the edge length of a single face is outW / 4.
    const int edge = outW / 4;

    std::cout << "Edge length in pixels: " << edge << std::endl;
    std::cout << "Output image dimensions: " << outW << "x" << outH << std::endl;

    for (int i = 0; i < outW; ++i) {

        if (i % 1000 == 0)
            std::cout << "Processing " << i << " of " << outW << std::endl;

        for (int j = 0; j < outH; ++j) {
            int currentFace = -1;
            int i_face = -1;
            int j_face = -1;

            // Determine which face and its local coordinates
            // We want the Top and Bottom cubes to align vertically with the Front cube.
            // This is a common arrangement. The layout will be:
            //       +---+
            //       | T |
            //   +---+---+---+---+
            //   | L | F | R | B |
            //   +---+---+---+---+
            //       | D |
            //       +---+

            // Top face (+Y), centered over the Front face
            if (i >= edge && i < 2 * edge && j < edge) {
                currentFace = 2;
                i_face = i - edge;
                j_face = j;
            }
            // Front face (+Z)
            else if (i >= edge && i < 2 * edge && j >= edge && j < 2 * edge) {
                currentFace = 4;
                i_face = i - edge;
                j_face = j - edge;
            }
            // Bottom face (-Y), centered under the Front face
            else if (i >= edge && i < 2 * edge && j >= 2 * edge) {
                currentFace = 3;
                i_face = i - edge;
                j_face = j - 2 * edge;
            }
            // Left face (-X)
            else if (i < edge && j >= edge && j < 2 * edge) {
                currentFace = 1;
                i_face = i;
                j_face = j - edge;
            }
            // Right face (+X)
            else if (i >= 2 * edge && i < 3 * edge && j >= edge && j < 2 * edge) {
                currentFace = 0;
                i_face = i - 2 * edge;
                j_face = j - edge;
            }
            // Back face (-Z)
            else if (i >= 3 * edge && i < 4 * edge && j >= edge && j < 2 * edge) {
                currentFace = 5;
                i_face = i - 3 * edge;
                j_face = j - edge;
            }
            
            if (currentFace != -1) {

                float x, y, z;
                outImgToXYZ(i_face, j_face, currentFace, edge, x, y, z);

                // Convert 3D vector to spherical coordinates
                const float theta = atan2(x, z);
                const float phi = atan2(y, hypot(x, z));

                // Convert spherical coordinates back to equirectangular coordinates
                const float uf = (inW * (theta + M_PI)) / (2 * M_PI);
                const float vf = (inH * (M_PI / 2.0f - phi)) / M_PI;

                const int ui = static_cast<int>(floor(uf));
                const int vi = static_cast<int>(floor(vf));
                
                const int u2 = ui + 1;
                const int v2 = vi + 1;

                const float mu = uf - ui;
                const float nu = vf - vi;

                // Bilinear interpolation
                const QColor A = image_in.pixelColor(ui % inW, clip(vi, 0, inH - 1));
                const QColor B = image_in.pixelColor(u2 % inW, clip(vi, 0, inH - 1));
                const QColor C = image_in.pixelColor(ui % inW, clip(v2, 0, inH - 1));
                const QColor D = image_in.pixelColor(u2 % inW, clip(v2, 0, inH - 1));
                
                const int rVal = static_cast<int>(round(A.red() * (1 - mu) * (1 - nu) + B.red() * mu * (1 - nu) + C.red() * (1 - mu) * nu + D.red() * mu * nu));
                const int gVal = static_cast<int>(round(A.green() * (1 - mu) * (1 - nu) + B.green() * mu * (1 - nu) + C.green() * (1 - mu) * nu + D.green() * mu * nu));
                const int bVal = static_cast<int>(round(A.blue() * (1 - mu) * (1 - nu) + B.blue() * mu * (1 - nu) + C.blue() * (1 - mu) * nu + D.blue() * mu * nu));

                image_out.setPixelColor(i, j, QColor(rVal, gVal, bVal));

            } else {

                // Set non-cube areas to black
                image_out.setPixelColor(i, j, Qt::black);
            }
        }
    }
}

// 
// Application begins
// 
int main(int argc, char** argv) {

    bool unfolded = false;
    QString input_image_path;

    // Make sure user provided an input image, and nothing more for now
    if (argc < 2) {
        std::cout << "Usage: ./image_to_cubemap [-u|--unfolded] <input_image_path>" << std::endl;
        return 1;
    }

    int argIndex = 1;

    // Check for optional argument
    if (std::string(argv[argIndex]) == "-u" || std::string(argv[argIndex]) == "--unfolded") {
        unfolded = true;
        ++argIndex;
    }

    // Now expect the image file path
    if (argIndex < argc) {
        input_image_path = QString::fromStdString(argv[argIndex]);
    } else {
        std::cerr << "Error: missing required argument: <input_image_path>\n";
        return 1;
    }

    // Done parsing, now use the values
    std::cout << "Unfolded option: " << (unfolded ? "true" : "false") << "\n";
    std::cout << "Filename: " << input_image_path.toStdString() << "\n";


    // Get the user's image path
    QFileInfo file_info(input_image_path);

    // Compute the input file's extension
    QString extension = file_info.suffix().toLower();
    QString path_no_extension = file_info.path() + "/" + file_info.completeBaseName();
    printf("Extension: '%s'\n", extension.toStdString().c_str());

    // Compute the output DDS cubemap filename
    QString output_dds = path_no_extension + ".dds";
    printf("DDS: '%s'\n", output_dds.toStdString().c_str());

    // Compute the output PNG filename
    QString output_png = path_no_extension + ".png";
    printf("PNG: '%s'\n", output_png.toStdString().c_str());

    // Make sure Qt will deal with large images
    QImageReader::setAllocationLimit(1000);

    QImage image_in;

    // Are we reading raw?
    if (extension == "dng")
        // Yes, load the raw DNG equirectangular image
        image_in = loadDNG(input_image_path);
    else {

        // No, load the raw PNG/JPG file
        if (!image_in.load(input_image_path)) {
            std::cerr << "Failed to load image: " << input_image_path.toStdString() << std::endl;
            return 1;
        }
    }

    QImage image_unfolded;

    if (unfolded) {
        // Source is assumed to be unfolded already, so just assign it
        image_unfolded = image_in;
    }
    else {
        // Create a black image to fill as unfolded cubemap
        int outHeight = image_in.width() * 3 / 4;
        QImage image_out(image_in.width(), outHeight, QImage::Format_RGB32);
        image_out.fill(Qt::black);
    
        // Fill the cubemap image using the equirectangular image 
        convertEquirectToCubemap(image_in, image_unfolded);
    
        // Save the cubemap first as a PNG
        std::cout << "Saving Cubemap to PNG: " << output_png.toStdString() << std::endl;
        image_unfolded.save(output_png);
        std::cout << "Saved Cubemap to PNG: " << output_png.toStdString() << std::endl;
    }

    // Then save image as a DDS
    writeCubemapToDDS(image_unfolded.rgbSwapped(), output_dds);
    std::cout << "Saved Cubemap to DDS: " << output_dds.toStdString() << std::endl;

    // Done!
    return 0;
}
