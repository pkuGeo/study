//
//  SliderPuzzleView.h
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SliderPuzzleStateDelegate.h"
@class ImageTileView;

/*
 * Enum to show available move direction
 */
typedef enum {
    MOVE_DIR_LEFT,
    MOVE_DIR_RIGHT,
    MOVE_DIR_UP,
    MOVE_DIR_DOWN,
    MOVE_DIR_NONE
} MoveDirection;

@interface SliderPuzzleViewController : UIViewController <UIGestureRecognizerDelegate>

@property (nonatomic, assign) unsigned short tileNumX;//num of tiles in x axis, default is 1
@property (nonatomic, assign) unsigned short tileNumY;//num of tiles in y axis, default is 1
@property (nonatomic, assign) float gapWidth;//width of the gap between tiles, default is 1
@property (nonatomic, assign) id<SliderPuzzleStateDelegate> gameStateDelegate;//delegate to recieve the game state msg



/*
 * Init a game with paticular image
 */
- (void)initGameWithPuzzleImage:(UIImage *)image delegate:(id<SliderPuzzleStateDelegate>)delegate;


- (void)resetAllTilesPosition;

/*
 * choose a random tile to remove
 */
- (void)selectBlankTile;

/*
 * The tiles will be rearranged in this function
 */
- (void)initializeGame;


/*
 * Called if some tile is tapped
 * If the tile is finally moved, return Yes, else return No.
 */
- (BOOL)moveTileView:(ImageTileView *)tileView;

@end
