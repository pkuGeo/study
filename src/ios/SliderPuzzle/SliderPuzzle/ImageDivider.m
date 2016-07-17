//
//  JpgImgDivider.m
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "ImageDivider.h"
#import "ImageTileView.h"

@implementation ImageDivider

+ (NSArray *)divideImage : (UIImage *)image tileNumX:(int)x tileNumY:(int)y
{
    assert(image!=nil);
    assert(x>0 && y>0);
    NSMutableArray *tileArr = [NSMutableArray new];
    
    float fW = image.size.width/x; //width of each tile
    float fH = image.size.height/y;//height of each tile
    
    /*
     * Create every tile
     */
    for (int h = 0; h < y; ++h) {
        for (int w = 0; w < x; ++w) {
            CGRect tileRect = CGRectMake(w*fW, h*fH, fW, fH);//position and size of the tile
            CGImageRef imageRef = CGImageCreateWithImageInRect([image CGImage], tileRect);
            UIImage *imageTile = [UIImage imageWithCGImage:imageRef];
            ImageTileView *tileView = [[ImageTileView alloc] initWithImage:imageTile originalX:w originalY:h];
            
            //current X and current Y is the same as the original X and Y
            tileView.currentX = w;
            tileView.currentY = h;
            [tileArr addObject:tileView];
        }
    }
    return tileArr;
}
@end
