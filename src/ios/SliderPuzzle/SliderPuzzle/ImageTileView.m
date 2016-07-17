//
//  ImgTile.m
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "ImageTileView.h"

@interface  ImageTileView  ()
//- (void)sendTappedMsgToSuperView:(UITapGestureRecognizer *)sender;
@end

@implementation ImageTileView

@synthesize originalX = _originalX;
@synthesize originalY = _originalY;
@synthesize currentX = _currentX;
@synthesize currentY = _currentY;

- (id)initWithImage:(UIImage *)img originalX:(unsigned short)x originalY:(unsigned short)y
{
    self = [super initWithImage:img];
    if (self) {
        _originalX = x;
        _originalY = y;
        self.userInteractionEnabled = YES;
//        UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(sendTappedMsgToSuperView:)];
//        [self addGestureRecognizer:tapRecognizer];
    }
    return self;
}

- (BOOL)isInCorrectPos 
{
    return self.originalX == self.currentX && self.originalY == self.currentY;
}

// Not using it in order to avoiding the unnecessarily dependance with SliderPuzzleView
//- (void)sendTappedMsgToSuperView:(UITapGestureRecognizer *)sender
//{
//    if (self.superview != nil && [self.superview respondsToSelector:@selector(moveTileView:)]) {
//        [self.superview performSelector:@selector(moveTileView:) withObject:self];
//    }
//}


@end