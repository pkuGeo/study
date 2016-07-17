//
//  SliderPuzzleView.m
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "SliderPuzzleViewController.h"
#import "ImageDivider.h"
#import "ImageTileView.h"
#import <UIKit/UIGestureRecognizerSubclass.h>

//====================MoveGestureRecognizer=============================
//===============Declaration And Implementation=========================
/*
 * This MoveGestureRecognizer is only used for preventing 
 * UITapGestureRecognizer been activated when tiny move occurred.
 */
@interface MoveGestureRecognizer : UIGestureRecognizer
@end

@implementation MoveGestureRecognizer
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.state = UIGestureRecognizerStateRecognized;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.state = UIGestureRecognizerStateFailed;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    self.state = UIGestureRecognizerStateCancelled;
}
@end

//============Hidden Details of SliderPuzzleViewController=============
@interface SliderPuzzleViewController () {
@private 
    //The tile-division detail of last time.
    unsigned short _lastTileNumX;
    unsigned short _lastTileNumY;
    
    //The image before divided into several tiles
    __strong UIImage *_puzzleImage;
    
    //The image of last time
    __strong UIImage *_lastPuzzleImage;
    
    //The pos that do not have any tile
    unsigned short _blankX;
    unsigned short _blankY;
    
    //Size of tile
    float _xPixels;
    float _yPixels;
    
    //Tiles, arranged by real position
    __strong NSMutableArray *_arrTiles;
    
    //arrange the game state
    unsigned int _tileNumInCorrectPos;
    
    //If it is NO, the function "win" will not be called, and user interaction is disabled
    BOOL _initialized;
}


/*
 * reset some values of variables
 */
- (void)resetStates;

/*
 * restore the position of tile by current x and y index, no matter where the view's actual position
 * (with animation)
 */
- (void)restoreTilePosition:(ImageTileView *)tileView;

/*
 * Get the index in the array by some tile view 
 */
- (int)getIndexInArrayOfTileView:(ImageTileView *)tileView;

/*
 * Get the index in the array by values of x and y
 */
- (int)getIndexInArrayByIndexX:(unsigned short)x indexY:(unsigned short)y;

/*
 * Get available move direction of the tile view
 */
- (MoveDirection)getAvailableMoveDirection:(ImageTileView *)tileView;

/*
 * some move functions
 */
- (void)moveTileToLeft:(ImageTileView *)tileView;
- (void)moveTileToRight:(ImageTileView *)tileView;
- (void)moveTileUp:(ImageTileView *)tileView;
- (void)moveTileDown:(ImageTileView *)tileView;

/*
 * When some tile is moved, adjust the currentX and currentY variables of the removed tile
 */
- (void)adjustPositionOfRemovedTileView;
/*
 * Called if some tile is moved to the correct position
 */
- (void)getCorrectTile;

/*
 * Called if some tile is moved from a correct position to an incorrect position
 */
- (void)getIncorrectTile;

/*
 * Set the value of inner variable "_initialized"
 * and enable/disable user interaction
 */
- (void)setInitialized:(BOOL)initialized;

/*
 * Called if user win
 */ 
- (void)win;

/*
 * Paste the tile moved before game start back to the screen
 */
- (void)restoreMovedTileWithAnimation:(BOOL)animated;

/*
 * Calculate the value of the frame by paticular tile view(currentX and currentY)
 */
- (CGRect)calcViewFrameByTileView:(ImageTileView *)tileView;

/*
 * Calculate the value of the frame by x and y
 */
- (CGRect)calcViewFrameByIndexX:(unsigned short)x indexY:(unsigned short)y;

/*
 * Called when a tap gesture is recognized
 */
- (void)handleTapFromSender:(UITapGestureRecognizer *)sender;

/*
 * Send some messages to game state delegate, if delegate is nil, the msg will not be sent
 */
- (void)sendWinMsg;
- (void)sendBlankTileCreatedMsg;
- (void)sendBlankTileRefilledMsg;

/*
 * To get the tile view of the tap position in self.view
 */
- (ImageTileView *)getImageTileViewByPositionInView:(CGPoint)pos;
@end


//==============Implementation of SliderPuzzleViewController===============
@implementation SliderPuzzleViewController

@synthesize tileNumX = _tileNumX;//The Set Method is overwritten
@synthesize tileNumY = _tileNumY;//The Set Method is overwittern
@synthesize gapWidth = _gapWidth;
@synthesize gameStateDelegate = _gameStateDelegate;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self resetStates];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self resetStates];
    }
    return self;
}

- (id)init
{
    self = [super init];
    if (self) {
        [self resetStates];
    }
    return self;
}

- (void)awakeFromNib
{
    [self resetStates];
}


- (void)viewDidLoad
{
    [self setInitialized:NO];
    self.view.multipleTouchEnabled = NO;
    //Add additional gesture recognizers
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTapFromSender:)];
    tapRecognizer.cancelsTouchesInView = NO;
    tapRecognizer.delegate = self;
    [self.view addGestureRecognizer:tapRecognizer];
    MoveGestureRecognizer *moveRecognizer = [[MoveGestureRecognizer alloc] initWithTarget:self action:nil];
    moveRecognizer.cancelsTouchesInView = NO;
    moveRecognizer.delegate = self;
    [self.view addGestureRecognizer:moveRecognizer];
    [tapRecognizer requireGestureRecognizerToFail:moveRecognizer];
    
}
- (void)resetStates
{
    self.tileNumX = 1;
    self.tileNumY = 1;
    _lastTileNumX = 0;
    _lastTileNumY = 0;
    _blankX = 0xFFFF;
    _blankY = 0xFFFF;
    self.gapWidth = 1;
    _tileNumInCorrectPos = 0;
    
}

- (void)initGameWithPuzzleImage:(UIImage *)image delegate:(id<SliderPuzzleStateDelegate>)delegate;
{
    self.gameStateDelegate = delegate;
    
    assert(image != nil);
    _puzzleImage = image;
    [self setInitialized:NO];
    [self resetAllTilesPosition];
    
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return YES;
}

- (void)resetAllTilesPosition
{
    if (_puzzleImage == nil) {
        return;
    }
    
    //Divide the image
    if (self.tileNumX != _lastTileNumX || self.tileNumY != _lastTileNumY || _lastPuzzleImage != _puzzleImage) { //Redivide the image
        //Remove old tile views
        for (ImageTileView *tileView in _arrTiles) {
            [tileView removeFromSuperview];
        }
        _xPixels = (self.view.frame.size.width-self.gapWidth*(self.tileNumX+1))/self.tileNumX;
        _yPixels = (self.view.frame.size.height-self.gapWidth*(self.tileNumY+1))/self.tileNumY;
        _arrTiles = [NSMutableArray arrayWithArray:[ImageDivider divideImage:_puzzleImage tileNumX:self.tileNumX tileNumY:self.tileNumY]];
        _tileNumInCorrectPos = self.tileNumX*self.tileNumY;
        
        //Set last state of division
        _lastPuzzleImage = _puzzleImage;
        _lastTileNumX = self.tileNumX;
        _lastTileNumY = self.tileNumY;
    }
    
    //add tile views into self.view
    for (ImageTileView *tileView in _arrTiles) {
        if (tileView.superview != self.view) {
            [tileView removeFromSuperview];
        }
        if (tileView.superview == nil) {
            [self.view addSubview:tileView];
        }
        tileView.frame = [self calcViewFrameByTileView:tileView];
    }
}


- (void)selectBlankTile
{
    [self restoreMovedTileWithAnimation:NO];
    _blankX = arc4random()%self.tileNumX;
    _blankY = arc4random()%self.tileNumY;
    int index = [self getIndexInArrayByIndexX:_blankX indexY:_blankY];
    ImageTileView *tileView = [_arrTiles objectAtIndex:index];
    if (tileView == nil) {
        return;
    }
    CGPoint pt = tileView.center;
    [UIView animateWithDuration:0.3 animations:^(void) {
        [tileView setFrame:CGRectMake(pt.x - 1, pt.y -1, 2, 2)];
    } completion:^(BOOL finished){
        [tileView removeFromSuperview];
        [self sendBlankTileCreatedMsg];
    }];
}

- (void)initializeGame
{
    [self setInitialized:NO];
    for (int i = 0; i < 2000; ++i) {
        int x = arc4random()%self.tileNumX;
        int y = arc4random()%self.tileNumY;
        if (x == _blankX && y == _blankY) {
            continue;
        }
        int index = x+y*self.tileNumX;
        ImageTileView *tileView = [_arrTiles objectAtIndex:index];
        [self moveTileView:tileView];
    }
    [self setInitialized:YES];
}

- (void)setTileNumX:(unsigned short)tileNumX
{
    _tileNumX = tileNumX;
    [self resetAllTilesPosition];
}

- (void)setTileNumY:(unsigned short)tileNumY
{
    _tileNumY = tileNumY;
    [self resetAllTilesPosition];
}

- (void)setInitialized:(BOOL)initialized
{
    _initialized = initialized;
    self.view.userInteractionEnabled = _initialized;
}

- (MoveDirection)getAvailableMoveDirection:(ImageTileView *)tileView;
{
    
    if (tileView.currentX == _blankX && tileView.currentY == _blankY) {
        return MOVE_DIR_NONE;
    }
    if (tileView.currentX == _blankX) {
        if (tileView.currentY > _blankY) {
            return MOVE_DIR_UP;
        } else {
            return MOVE_DIR_DOWN;
        }
    }
    if (tileView.currentY == _blankY) {
        if (tileView.currentX > _blankX) {
            return MOVE_DIR_LEFT;
        } else {
            return MOVE_DIR_RIGHT;
        }
    }
    return MOVE_DIR_NONE;
}

- (BOOL)moveTileView:(ImageTileView *)tileView;
{
    MoveDirection dir = [self getAvailableMoveDirection:tileView];
    
    switch (dir) {
        case MOVE_DIR_NONE:
            return NO;
        case MOVE_DIR_UP:
        {
            unsigned short xIndex = tileView.currentX;
            unsigned short yIndexFrom = _blankY+1;
            unsigned short yIndexTo = tileView.currentY;
            _blankY = tileView.currentY;
            [self adjustPositionOfRemovedTileView];
            for (int yIndex = yIndexFrom; yIndex <= yIndexTo; ++yIndex) {
                ImageTileView *tmpTileView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self moveTileUp:tmpTileView];
            }
            break;
        }
        case MOVE_DIR_DOWN:
        {
            unsigned short xIndex = tileView.currentX;
            unsigned short yIndexFrom  = tileView.currentY;
            unsigned short yIndexTo = _blankY-1;
            _blankY = tileView.currentY;
            [self adjustPositionOfRemovedTileView];
            for (int yIndex = yIndexTo; yIndex >= yIndexFrom; --yIndex) {
                ImageTileView *tmpTileView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self moveTileDown:tmpTileView];
            }
            break;
        }
        case MOVE_DIR_LEFT:
        {
            unsigned short yIndex = tileView.currentY;
            unsigned short xIndexFrom = _blankX+1;
            unsigned short xIndexTo = tileView.currentX;
            _blankX = tileView.currentX;
            [self adjustPositionOfRemovedTileView];
            
            for (int xIndex = xIndexFrom; xIndex <= xIndexTo; ++xIndex) {
                ImageTileView *tmpTileView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self moveTileToLeft:tmpTileView];
            }
            break;
        }
        case MOVE_DIR_RIGHT:
        {
            unsigned short yIndex = tileView.currentY;
            unsigned short xIndexFrom = tileView.currentX;
            unsigned short xIndexTo = _blankX-1;
            _blankX = tileView.currentX;
            [self adjustPositionOfRemovedTileView];

            for (int xIndex = xIndexTo; xIndex >= xIndexFrom; --xIndex) {
                ImageTileView *tmpTileView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self moveTileToRight:tmpTileView];
            }
            break;
        }
        default:
            return NO;
    }
    return YES;
}

- (int)getIndexInArrayOfTileView:(ImageTileView *)tileView
{
    return [self getIndexInArrayByIndexX:tileView.currentX indexY:tileView.currentY];
}

- (int)getIndexInArrayByIndexX:(unsigned short)x indexY:(unsigned short)y
{
    return x+self.tileNumX*y;
}


- (void)moveTileToLeft:(ImageTileView *)tileView
{
    if (tileView.currentX <= 0) {
        return;
    }
    
    bool correctBeforeMove = [tileView isInCorrectPos];
    int originIndex = [self getIndexInArrayOfTileView:tileView];
    --tileView.currentX;
    int targetIndex = [self getIndexInArrayOfTileView:tileView];
    [_arrTiles exchangeObjectAtIndex:originIndex withObjectAtIndex:targetIndex];
    [self restoreTilePosition:tileView];
    if (correctBeforeMove) {
        [self getIncorrectTile];
    } else {
        if ([tileView isInCorrectPos]) {
            [self getCorrectTile];
        }
    }
}

- (void)moveTileToRight:(ImageTileView *)tileView
{
    if (tileView.currentX >= self.tileNumX-1) {
        return;
    }
    bool correctBeforeMove = [tileView isInCorrectPos];
    int originIndex = [self getIndexInArrayOfTileView:tileView];
    ++tileView.currentX;
    int targetIndex = [self getIndexInArrayOfTileView:tileView];
    [_arrTiles exchangeObjectAtIndex:originIndex withObjectAtIndex:targetIndex];
    [self restoreTilePosition:tileView];
    if (correctBeforeMove) {
        [self getIncorrectTile];
    } else {
        if ([tileView isInCorrectPos]) {
            [self getCorrectTile];
        }
    }
}

- (void)moveTileUp:(ImageTileView *)tileView
{
    if (tileView.currentY <= 0) {
        return;
    }
    bool correctBeforeMove = [tileView isInCorrectPos];
    int originIndex = [self getIndexInArrayOfTileView:tileView];
    --tileView.currentY;
    int targetIndex = [self getIndexInArrayOfTileView:tileView];
    [_arrTiles exchangeObjectAtIndex:originIndex withObjectAtIndex:targetIndex];
    [self restoreTilePosition:tileView];
    if (correctBeforeMove) {
        [self getIncorrectTile];
    } else {
        if ([tileView isInCorrectPos]) {
            [self getCorrectTile];
        }
    }
}

- (void)moveTileDown:(ImageTileView *)tileView
{
    if (tileView.currentY >= self.tileNumY-1) {
        return;
    }
    bool correctBeforeMove = [tileView isInCorrectPos];    
    int originIndex = [self getIndexInArrayOfTileView:tileView];
    ++tileView.currentY;
    int targetIndex = [self getIndexInArrayOfTileView:tileView];
    [_arrTiles exchangeObjectAtIndex:originIndex withObjectAtIndex:targetIndex];
    [self restoreTilePosition:tileView];
    if (correctBeforeMove) {
        [self getIncorrectTile];
    } else {
        if ([tileView isInCorrectPos]) {
            [self getCorrectTile];
        }
    }
}

- (void)adjustPositionOfRemovedTileView
{
    int index = [self getIndexInArrayByIndexX:_blankX indexY:_blankY];
    if (index < [_arrTiles count]) {
        ImageTileView *tileView = [_arrTiles objectAtIndex:index];
        tileView.currentX = _blankX;
        tileView.currentY = _blankY;
    }    
}

- (CGRect)calcViewFrameByTileView:(ImageTileView *)tileView
{
    return [self calcViewFrameByIndexX:tileView.currentX indexY:tileView.currentY];
}

- (CGRect)calcViewFrameByIndexX:(unsigned short)x indexY:(unsigned short)y
{
    return CGRectMake(self.gapWidth+(self.gapWidth+_xPixels)*x, 
                      self.gapWidth+(self.gapWidth+_yPixels)*y, 
                      _xPixels, 
                      _yPixels);    
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    if ([touches count]!=1) {
        return;
    }
    UITouch *touch = [[touches objectEnumerator] nextObject];
    if ([touch.view isKindOfClass:[ImageTileView class]]) {
        ImageTileView *tileView = (ImageTileView *)(touch.view);
        MoveDirection dir = [self getAvailableMoveDirection:tileView];
        if (dir==MOVE_DIR_NONE) {
            return;
        }
        
        if (dir == MOVE_DIR_LEFT || dir == MOVE_DIR_RIGHT) {
            CGRect rectFrom = [self calcViewFrameByTileView:tileView];
            float deltaMoveX = ([touch locationInView:self.view].x - [touch previousLocationInView:self.view].x) + (tileView.frame.origin.x - rectFrom.origin.x);
            int deltaIndexX = dir==MOVE_DIR_LEFT?-1:1;
            CGRect rectTo = [self calcViewFrameByIndexX:tileView.currentX+deltaIndexX indexY:tileView.currentY];
            float maxMoveX = rectTo.origin.x - rectFrom.origin.x;
            if (deltaMoveX*maxMoveX>0) { //the same direction
                //resize the move 
                if (abs(deltaMoveX) > abs(maxMoveX)) {
                    deltaMoveX = maxMoveX;
                }
            } else {
                deltaMoveX = 0;
            }
            int yIndex = tileView.currentY;
            int xIndexFrom = tileView.currentX, xIndexTo = tileView.currentX;
            if (dir == MOVE_DIR_LEFT) {
                xIndexFrom = _blankX+1;
            } else {
                xIndexTo = _blankX-1;
            }
            
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            for (int xIndex = xIndexFrom; xIndex <= xIndexTo; ++xIndex) {
                ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                CGRect frame = [self calcViewFrameByTileView:tmpView];
                frame.origin.x += deltaMoveX;
                tmpView.frame = frame;
            }
            
            
        } else if (dir == MOVE_DIR_DOWN || dir == MOVE_DIR_UP) {
            CGRect rectFrom = [self calcViewFrameByTileView:tileView];
            float deltaMoveY = ([touch locationInView:self.view].y - [touch previousLocationInView:self.view].y) + (tileView.frame.origin.y - rectFrom.origin.y);
            int deltaIndexY = dir==MOVE_DIR_UP?-1:1;
            CGRect rectTo = [self calcViewFrameByIndexX:tileView.currentX indexY:tileView.currentY+deltaIndexY];
            float maxMoveY = rectTo.origin.y - rectFrom.origin.y;
            if (deltaMoveY*maxMoveY>0) { //the same direction
                //resize the move 
                if (abs(deltaMoveY) > abs(maxMoveY)) {
                    deltaMoveY = maxMoveY;
                }
            } else {
                deltaMoveY = 0;
            }
            int xIndex = tileView.currentX;
            int yIndexFrom = tileView.currentY, yIndexTo = tileView.currentY;
            if (dir == MOVE_DIR_UP) {
                yIndexFrom = _blankY+1;
            } else {
                yIndexTo = _blankY-1;
            }
            
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            for (int yIndex = yIndexFrom; yIndex <= yIndexTo; ++yIndex) {
                ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                CGRect frame = [self calcViewFrameByTileView:tmpView];
                frame.origin.y += deltaMoveY;
                tmpView.frame = frame;
            }
            
        }
    }    
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    if ([touches count]!=1) {
        return;
    }
    UITouch *touch = [[touches objectEnumerator] nextObject];
    if ([touch.view isKindOfClass:[ImageTileView class]]) {
        ImageTileView *tileView = (ImageTileView *)(touch.view);
        MoveDirection dir = [self getAvailableMoveDirection:tileView];
        if (dir==MOVE_DIR_NONE) {
            return;
        }
        
        CGPoint orginPt = [self calcViewFrameByTileView:tileView].origin;
        CGPoint currPt = tileView.frame.origin;
        BOOL shouldMoveToBlank = NO;
        if (dir == MOVE_DIR_LEFT || dir == MOVE_DIR_RIGHT) {
            if (abs(orginPt.x-currPt.x)>(_xPixels+self.gapWidth)/2) {
                shouldMoveToBlank = YES;
            }
            int yIndex = tileView.currentY;
            int xIndexFrom = tileView.currentX, xIndexTo = tileView.currentX;
            if (dir == MOVE_DIR_LEFT) {
                xIndexFrom = _blankX+1;
            } else {
                xIndexTo = _blankX-1;
            }
            if (shouldMoveToBlank) {
                _blankX = tileView.currentX;
                [self adjustPositionOfRemovedTileView];
            }
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            if (dir == MOVE_DIR_LEFT) { //move to left
                for (int xIndex = xIndexFrom; xIndex <= xIndexTo; ++xIndex) {
                    ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                    if (shouldMoveToBlank) {
                        [self moveTileToLeft:tmpView];
                    } else {
                        [self restoreTilePosition:tmpView];
                    }
                }
            } else { //move to right
                for (int xIndex = xIndexTo; xIndex >= xIndexFrom; --xIndex) {
                    ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                    if (shouldMoveToBlank) {
                        [self moveTileToRight:tmpView];
                    } else {
                        [self restoreTilePosition:tmpView];
                    }
                }
            }
        } else if (dir == MOVE_DIR_DOWN || dir == MOVE_DIR_UP) {
            if (abs(orginPt.y-currPt.y)>(_yPixels+self.gapWidth)/2) {
                shouldMoveToBlank = YES;
            }
            int xIndex = tileView.currentX;
            int yIndexFrom = tileView.currentY, yIndexTo = tileView.currentY;
            if (dir == MOVE_DIR_UP) {
                yIndexFrom = _blankY+1;
            } else {
                yIndexTo = _blankY-1;
            }
            if (shouldMoveToBlank) {
                _blankY = tileView.currentY;
                [self adjustPositionOfRemovedTileView];
            }
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            if (dir == MOVE_DIR_UP) { //move up
                for (int yIndex = yIndexFrom; yIndex <= yIndexTo; ++yIndex) {
                    ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                    if (shouldMoveToBlank) {
                        [self moveTileUp:tmpView];
                    } else {
                        [self restoreTilePosition:tmpView];
                    }
                    
                } 
            } else { //move down
                for (int yIndex = yIndexTo; yIndex >= yIndexFrom; --yIndex) {
                    ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                    if (shouldMoveToBlank) {
                        [self moveTileDown:tmpView];
                    } else {
                        [self restoreTilePosition:tmpView];
                    }
                } 
            }
            
        }
    }     
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    if ([touches count]!=1) {
        return;
    }
    UITouch *touch = [[touches objectEnumerator] nextObject];
    if ([touch.view isKindOfClass:[ImageTileView class]]) {
        ImageTileView *tileView = (ImageTileView *)(touch.view);
        MoveDirection dir = [self getAvailableMoveDirection:tileView];
        if (dir==MOVE_DIR_NONE) {
            return;
        }
        
        if (dir == MOVE_DIR_LEFT || dir == MOVE_DIR_RIGHT) {
            int yIndex = tileView.currentY;
            int xIndexFrom = tileView.currentX, xIndexTo = tileView.currentX;
            if (dir == MOVE_DIR_LEFT) {
                xIndexFrom = _blankX+1;
            } else {
                xIndexTo = _blankX-1;
            }
            
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            for (int xIndex = xIndexFrom; xIndex <= xIndexTo; ++xIndex) {
                ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self restoreTilePosition:tmpView];
            }
            
            
        } else if (dir == MOVE_DIR_DOWN || dir == MOVE_DIR_UP) {
            int xIndex = tileView.currentX;
            int yIndexFrom = tileView.currentY, yIndexTo = tileView.currentY;
            if (dir == MOVE_DIR_UP) {
                yIndexFrom = _blankY+1;
            } else {
                yIndexTo = _blankY-1;
            }
            
            //all tiles from the tile which recieved the touch to the blank tile should be moved
            for (int yIndex = yIndexFrom; yIndex <= yIndexTo; ++yIndex) {
                ImageTileView *tmpView = [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:xIndex indexY:yIndex]];
                [self restoreTilePosition:tmpView];
            }
        }
    }    
    
}

- (void)restoreTilePosition:(ImageTileView *)tileView
{
    CGRect pos = [self calcViewFrameByTileView:tileView];
    CGRect currentPos = tileView.frame;
    float distance = 0;
    if (abs(pos.origin.x - currentPos.origin.x) > 1E-5) {
        distance = abs(pos.origin.x - currentPos.origin.x);
    } else {
        distance = abs(pos.origin.y - currentPos.origin.y);
    }
    float time = distance/262;//Set the speed to 270 pixels per second
    [UIView animateWithDuration:time animations:^(void) {
        tileView.frame = pos;
    }];
}

- (void)handleTapFromSender:(UITapGestureRecognizer *)sender
{
    CGPoint pt = [sender locationInView:self.view];
    ImageTileView *tileView = [self getImageTileViewByPositionInView:pt];
    if (tileView != nil) {
        [self moveTileView:tileView];
    }
}

- (ImageTileView *)getImageTileViewByPositionInView:(CGPoint)pos
{
    int x = (pos.x-self.gapWidth)/(self.gapWidth+_xPixels);
    int y = (pos.y-self.gapWidth)/(self.gapWidth+_yPixels);
    if (x < 0 || y < 0) {
        return nil;
    }
    assert(x<self.tileNumX && y<self.tileNumY);
    return [_arrTiles objectAtIndex:[self getIndexInArrayByIndexX:x indexY:y]];
}

- (void)getCorrectTile
{
    ++_tileNumInCorrectPos;
    if (_tileNumInCorrectPos == self.tileNumX*self.tileNumY) {
        if (_initialized) {
            [self win];
        }
    }
}

- (void)getIncorrectTile
{
    --_tileNumInCorrectPos;
}

- (void)restoreMovedTileWithAnimation:(BOOL)animated
{
    int index = [self getIndexInArrayByIndexX:_blankX indexY:_blankY];
    CGRect pos = [self calcViewFrameByIndexX:_blankX indexY:_blankY];
    _blankX = 0xFFFF;
    _blankY = 0xFFFF;
    if (index < [_arrTiles count]) {
        ImageTileView *tileView = [_arrTiles objectAtIndex:index];

        [self.view addSubview:tileView];
        if (animated) {
            CGPoint pt = CGPointMake(pos.origin.x+pos.size.width/2, pos.origin.y+pos.size.height/2);
            tileView.frame = CGRectMake(pt.x - 1, pt.y -1, 2, 2);
            [UIView animateWithDuration:0.3 animations:^(void) {
                [tileView setFrame:pos];
            }];
        } else {
            tileView.frame = pos;
        }

        [self sendBlankTileRefilledMsg];
    }
}

- (void)win
{
    [self restoreMovedTileWithAnimation:YES];
    [self setInitialized:NO];
    [self sendWinMsg];
}

- (void)sendWinMsg
{
    if (self.gameStateDelegate != nil) {
        [self.gameStateDelegate getWinMsg:self];
    }
}

- (void)sendBlankTileCreatedMsg
{
    if (self.gameStateDelegate != nil) {
        [self.gameStateDelegate getBlankTileCreatedMsg:self];
    }
}

- (void)sendBlankTileRefilledMsg
{
    if (self.gameStateDelegate != nil) {
        [self.gameStateDelegate getBlankTileRefilledMsg:self];
    }
}
@end
