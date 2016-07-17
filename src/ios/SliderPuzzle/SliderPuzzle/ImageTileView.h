//
//  ImgTile.h
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ImageTileView : UIImageView

/*
 * The original(correct) position of the tile
 */
@property (nonatomic, readonly) unsigned short originalX;
@property (nonatomic, readonly) unsigned short originalY;

/*
 * Current position of the tile
 */
@property (nonatomic, assign) unsigned short currentX;
@property (nonatomic, assign) unsigned short currentY;


/*
 * Use the image tile and the orignal position to initialize the object
 */
- (id)initWithImage:(UIImage *)img originalX:(unsigned short)x originalY:(unsigned short)y;

/*
 * To judge whether or not the tile is in correct position
 */
- (BOOL)isInCorrectPos;



@end
