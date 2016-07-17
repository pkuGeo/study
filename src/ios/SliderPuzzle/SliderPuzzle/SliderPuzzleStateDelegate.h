//
//  SliderPuzzleStateDelegate.h
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-16.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SliderPuzzleViewController;
@protocol SliderPuzzleStateDelegate <NSObject>
@required
/*
 * Will be called when player win 
 */
- (void)getWinMsg:(SliderPuzzleViewController *)puzzleView;

/*
 * Will be called when a tile is selected to remove from the game to create a blank position
 */
- (void)getBlankTileCreatedMsg:(SliderPuzzleViewController *)puzzleView;

/*
 * Will be called when the removed tile is refilled in the puzzle view
 */
- (void)getBlankTileRefilledMsg:(SliderPuzzleViewController *)puzzleView;
@end
