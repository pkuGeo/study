//
//  CPngConverter.h
//  pngconvert
//
//  Created by Haiyang Zhou on 13-12-17.
//  Copyright (c) 2013年 Haiyang Zhou. All rights reserved.
//

#ifndef __pngconvert__CPngConverter__
#define __pngconvert__CPngConverter__

#include <iostream>



class CPngConverter
{
public:
    typedef enum {
        CONVERT_STATUS_NONEED,
        CONVERT_STATUS_OK,
        CONVERT_STATUS_SRC_CANNOT_OPEN,
        CONVERT_STATUS_SRC_TYPE_ERROR,
        CONVERT_STATUS_DEST_CANNOT_OPEN,
        CONVERT_STATUS_DEST_DIR_FAILURE,
        CONVERT_STATUS_UNKNOWN_FAILURE
    } CONVERT_STATUS;
    CPngConverter();
    CPngConverter(const std::string &baseDir,
                  const std::string &destDir,
                  const std::string &srcFile);
    
    CONVERT_STATUS convert();
    CONVERT_STATUS convertToWebp();
    
private:
    CONVERT_STATUS convertJpegToWebp();
    CONVERT_STATUS convertPngToWebp();
    
    std::string m_baseDir;
    std::string m_backupDir;
    std::string m_srcFile;
};

#endif /* defined(__pngconvert__CPngConverter__) */
