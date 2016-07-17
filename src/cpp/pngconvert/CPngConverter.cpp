//
//  CPngConverter.cpp
//  pngconvert
//
//  Created by Haiyang Zhou on 13-12-17.
//  Copyright (c) 2013 Haiyang Zhou. All rights reserved.
//

#include <png.h>
#include <jpeglib.h>
#include <webp/encode.h>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>

#include "CPngConverter.h"

CPngConverter::CPngConverter()
{
    
}

CPngConverter::CPngConverter(const std::string &baseDir,
                             const std::string &backupDir,
                             const std::string &srcFile)
:m_baseDir(baseDir)
,m_backupDir(backupDir)
,m_srcFile(srcFile)
{
    if (*baseDir.rbegin() !='/'
        && *baseDir.rbegin() !='\\') {
        m_baseDir.append("/");
    }
    if (*backupDir.rbegin() != '/'
        && *backupDir.rbegin() != '\\') {
        m_backupDir.append("/");
    }
}

inline void setAlphaBit(unsigned char *alphabuf,int idx)
{
    int byteIdx = idx/8;
    int bitIdx = 7-(idx%8);
    unsigned char mask = ~(((unsigned char)1)<<bitIdx);
    alphabuf[byteIdx] &= mask;
}

CPngConverter::CONVERT_STATUS CPngConverter::convert()
{
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char *alphabytes = NULL;
    int alphabytecnt = 0;
    unsigned char *picbuf = NULL;
    
    FILE *srcfp = fopen((m_baseDir+m_srcFile).c_str(), "r");
    if (srcfp == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Cannot Open File: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_SRC_CANNOT_OPEN;
    }
    const int sigSize = 8;
    unsigned char signature[sigSize] = {'\0'};
    fread(signature, 1, sigSize, srcfp);
    if (png_sig_cmp(signature, 0, sigSize)!=0) {
        //Not Png File
        fclose(srcfp);
        return CONVERT_STATUS_NONEED;
    }
    //It is a png file, backup
    boost::filesystem::copy_file(m_baseDir+m_srcFile, m_backupDir+m_srcFile, boost::filesystem::copy_option::overwrite_if_exists);
    
    //read png to picbuf, and alpha value to alphabytes
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        fclose(srcfp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    png_init_io(png_ptr, srcfp);
    png_set_sig_bytes(png_ptr, sigSize);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    
    int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
    int color_type = png_get_color_type(png_ptr,info_ptr);
    width = png_get_image_width(png_ptr,info_ptr);
    height = png_get_image_height(png_ptr,info_ptr);

    if (bit_depth != 8) {
        std::cerr<<"In "<<__FUNCTION__<<": Bit Depth Not Supported: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    bool hasAlpha = (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) || (color_type == PNG_COLOR_TYPE_RGB_ALPHA);
    picbuf = (unsigned char *)malloc(width*height*3);
    if (picbuf == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Malloc Pic Buf Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    memset(picbuf, 0xFF, width*height*3);
    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
    alphabytecnt = width*height;
    alphabytecnt = (alphabytecnt%8==0) ? (alphabytecnt/8) : (alphabytecnt/8+1);
    //alphabytecnt = (alphabytecnt%4==0) ? alphabytecnt : (alphabytecnt/4+1)*4;
    alphabytes = (unsigned char *)malloc(alphabytecnt);
    if (alphabytes == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Malloc Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        free(picbuf);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    memset(alphabytes, 0xFF, alphabytecnt);
    for (int row = 0; row < (int)height; ++ row) {
        for (int col = 0; col < (int)width; ++col) {
            if (hasAlpha) {
                //std::cout<<(int)row_pointers[row][col*4+3]<<"\t";
                if (row_pointers[row][col*4+3]==0) {
                    setAlphaBit(alphabytes, row*width+col);
                    picbuf[row*width*3+col*3] = 0;
                    picbuf[row*width*3+col*3+1] = 0;
                    picbuf[row*width*3+col*3+2] = 0;
                } else {
                    picbuf[row*width*3+col*3] = row_pointers[row][col*4] ;
                    picbuf[row*width*3+col*3+1] = row_pointers[row][col*4+1];
                    picbuf[row*width*3+col*3+2] = row_pointers[row][col*4+2];
                }
                
//                double alpha = row_pointers[row][col*4+3]/255.0;
//                picbuf[row*width*3+col*3] = row_pointers[row][col*4]*alpha + 255*(1-alpha);
//                picbuf[row*width*3+col*3+1] = row_pointers[row][col*4+1]*alpha + 255*(1-alpha);
//                picbuf[row*width*3+col*3+2] = row_pointers[row][col*4+2]*alpha+255*(1-alpha);
//                if (row_pointers[row][col*4+3]==0) {
//                    setAlphaBit(alphabytes, row*width+col);
//                }
            } else {
                picbuf[row*width*3+col*3] = row_pointers[row][col*3];
                picbuf[row*width*3+col*3+1] = row_pointers[row][col*3+1];
                picbuf[row*width*3+col*3+2] = row_pointers[row][col*3+2];
            }
            
            
        }
        //std::cout<<std::endl;
    }
    fclose(srcfp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    struct jpeg_compress_struct jcinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;                 //target file
    JSAMPROW row_pointer = NULL;        //pointer to JSAMPLE row
    int     row_stride = width*3;
    jcinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&jcinfo);
    
    outfile = fopen((m_baseDir+m_srcFile).c_str(), "w");
    if (outfile == NULL) {
        free(picbuf);
        free(alphabytes);
        std::cerr<<"In "<<__FUNCTION__<<": Dest File Open Failed: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_DEST_CANNOT_OPEN;
    }
    
    jpeg_stdio_dest(&jcinfo, outfile);
    jcinfo.image_width = width;
    jcinfo.image_height = height;
    jcinfo.input_components = 3;
    jcinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jcinfo);
    jpeg_set_quality(&jcinfo, 255, TRUE );//limit to baseline-JPEG values
    jpeg_start_compress(&jcinfo, TRUE);
    
    while (jcinfo.next_scanline < jcinfo.image_height)
    {
        row_pointer = & picbuf[jcinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&jcinfo, &row_pointer, 1);
    }
    jpeg_finish_compress(&jcinfo);
    jpeg_destroy_compress(&jcinfo);
    fseek(outfile, 0, SEEK_END);
    fwrite(alphabytes, 1, alphabytecnt, outfile);
    unsigned char endSignature[6] ={
        (unsigned char)(width&0xFF),
        (unsigned char)(width>>8),
        (unsigned char)(height&0xFF),
        (unsigned char)(height>>8),
        0xA2,
        0x5D
    };
//    for (int i = 0; i < height; ++i) {
//        for (int j = 0; j < width; ++j) {
//            int pos = i*width+j;
//            int idx1 = pos/8;
//            int idx2 = 7-(pos%8);
//            int val = (alphabytes[idx1]>>idx2)&1;
//            printf("%d",val);
//        }
//        printf("\n");
//    }
    fwrite(endSignature, 1, 6, outfile);
    fflush(outfile);
    fclose(outfile);
    free(alphabytes);
    free(picbuf);
    return CONVERT_STATUS_OK;
}

CPngConverter::CONVERT_STATUS CPngConverter::convertPngToWebp()
{
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned char *picbuf = NULL;
    int pixelbyte = 3;
    FILE *srcfp = fopen((m_baseDir+m_srcFile).c_str(), "r");
    //read png to picbuf, and alpha value to alphabytes
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        fclose(srcfp);
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        std::cerr<<"In "<<__FUNCTION__<<": LibError: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    png_init_io(png_ptr, srcfp);
    png_set_sig_bytes(png_ptr, 0);
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
    
    int bit_depth = png_get_bit_depth(png_ptr,info_ptr);
    int color_type = png_get_color_type(png_ptr,info_ptr);
    width = png_get_image_width(png_ptr,info_ptr);
    height = png_get_image_height(png_ptr,info_ptr);
    
    if (bit_depth != 8) {
        std::cerr<<"In "<<__FUNCTION__<<": Bit Depth Not Supported: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    bool hasAlpha = (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) || (color_type == PNG_COLOR_TYPE_RGB_ALPHA);
    if (hasAlpha) {
        picbuf = (unsigned char *)malloc(width*height*4);
        memset(picbuf, 0xFF, width*height*4);
        pixelbyte = 4;
    } else {
        picbuf = (unsigned char *)malloc(width*height*3);
        memset(picbuf, 0xFF, width*height*3);
    }
    if (picbuf == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Malloc Pic Buf Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(srcfp);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    
    png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
    for (int row = 0; row < (int)height; ++row) {
        memcpy(picbuf+row*width*pixelbyte, row_pointers[row], width*pixelbyte);
    }
    fclose(srcfp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    
    
    unsigned char *webpbuf = NULL;
    size_t size = 0;
    if (hasAlpha) {
        size = WebPEncodeRGBA(picbuf, width, height, width*pixelbyte, 0, &webpbuf);
    }else {
        size = WebPEncodeRGB(picbuf, width, height, width*pixelbyte,0,&webpbuf);
    }
    
    free(picbuf);
    if (size == 0) {
        std::cerr<<"In "<<__FUNCTION__<<": Encode WebP Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    std::string filename = m_srcFile;
    size_t pos = m_srcFile.rfind('.');
    if (pos!=std::string::npos) {
        filename = m_srcFile.substr(0,pos);
    }
    FILE *webpFp = fopen((m_baseDir+filename+".webp").c_str(), "wb");
    if (webpFp == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Create WebP File Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        free(webpbuf);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    fwrite(webpbuf, 1, size, webpFp);
    fflush(webpFp);
    fclose(webpFp);
    free(webpbuf);
    return CONVERT_STATUS_OK;
}


CPngConverter::CONVERT_STATUS CPngConverter::convertJpegToWebp()
{
    struct jpeg_decompress_struct	cinfo;
    struct jpeg_error_mgr jerr;
    FILE * infile;
    int row_stride;
    if ((infile = fopen((m_baseDir+m_srcFile).c_str(), "rb")) == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Cannot Open File: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_SRC_CANNOT_OPEN;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    
    if(cinfo.output_components != 3 && cinfo.output_components != 4)
    {
        jpeg_destroy_decompress(&cinfo);
        std::cerr<<"In "<<__FUNCTION__<<": File Type Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_SRC_TYPE_ERROR;
    }
    int width = cinfo.output_width;
    int height = cinfo.output_height;
    row_stride = cinfo.output_width * cinfo.output_components;
    
    unsigned char *picbuf = (unsigned char *)malloc(row_stride*height);
    if (picbuf == NULL) {
        jpeg_destroy_decompress(&cinfo);
        std::cerr<<"In "<<__FUNCTION__<<": Malloc Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    
    memset(picbuf, 0, row_stride*height);
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *rowp[1];
        rowp[0] = (unsigned char *)picbuf + row_stride * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, rowp, 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    
    
    unsigned char *webpbuf = NULL;
    size_t size = 0;
    if (row_stride/width==4) {
        size = WebPEncodeRGBA(picbuf, width, height, row_stride, 0, &webpbuf);
    }else {
        size = WebPEncodeRGB(picbuf, width, height, row_stride,0,&webpbuf);
    }
    free(picbuf);
    if (size == 0) {
        std::cerr<<"In "<<__FUNCTION__<<": Encode WebP Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    std::string filename = m_srcFile;
    size_t pos = m_srcFile.rfind('.');
    if (pos!=std::string::npos) {
        filename = m_srcFile.substr(0,pos);
    }
    FILE *webpFp = fopen((m_baseDir+filename+".webp").c_str(), "wb");
    if (webpFp == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Create WebP File Error: "<<(m_baseDir+m_srcFile)<<std::endl;
        free(webpbuf);
        return CONVERT_STATUS_UNKNOWN_FAILURE;
    }
    fwrite(webpbuf, 1, size, webpFp);
    fflush(webpFp);
    fclose(webpFp);
    free(webpbuf);
    return CONVERT_STATUS_OK;
}

CPngConverter::CONVERT_STATUS CPngConverter::convertToWebp()
{
    FILE *srcfp = fopen((m_baseDir+m_srcFile).c_str(), "r");
    if (srcfp == NULL) {
        std::cerr<<"In "<<__FUNCTION__<<": Cannot Open File: "<<(m_baseDir+m_srcFile)<<std::endl;
        return CONVERT_STATUS_SRC_CANNOT_OPEN;
    }
    const int sigSize = 8;
    unsigned char signature[sigSize] = {'\0'};
    fread(signature, 1, sigSize, srcfp);
    fclose(srcfp);
    if (png_sig_cmp(signature, 0, sigSize)!=0) {
        //Not Png File
        return convertJpegToWebp();
    }
    return convertPngToWebp();
}
