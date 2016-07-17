//
//  JpgImgDivider.h
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface ImageDivider : NSObject

/**
 *  Function for dividing the image to tiles
 *  Return Value is an array of ImageTileViews
 *  
 */
+ (NSArray *)divideImage:(UIImage *)image tileNumX:(int)x tileNumY:(int)y;

@end
