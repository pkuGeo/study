//
//  ViewController.m
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"
#import "SliderPuzzleViewController.h"

@interface ViewController () {
    __strong IBOutlet UIButton *_btnStartGame;
}
@property (nonatomic, strong) SliderPuzzleViewController *puzzleViewController;
@end

@implementation ViewController

@synthesize puzzleViewController = _puzzleViewController;

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIImage *img = [UIImage imageNamed:@"globe.jpg"];
    SliderPuzzleViewController *puzzleViewController = [[SliderPuzzleViewController alloc] init];
    puzzleViewController.view.frame = CGRectMake(0,0,320,320);
    puzzleViewController.tileNumX = 4;
    puzzleViewController.tileNumY = 4;
    [puzzleViewController initGameWithPuzzleImage:img delegate:self];
    [self.view addSubview:puzzleViewController.view];
    self.puzzleViewController = puzzleViewController;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (IBAction)startGame:(id)sender
{
    _btnStartGame.hidden = YES;
    [self.puzzleViewController selectBlankTile];
    [self.puzzleViewController performSelector:@selector(initializeGame) withObject:nil afterDelay:0.35];
}

- (void)getWinMsg:(SliderPuzzleViewController *)puzzleView
{
    UIAlertView *view = [[UIAlertView alloc] initWithTitle:@"Congratulations" message:@"You Win!" delegate:nil cancelButtonTitle:@"OK" otherButtonTitles:nil];
    [view show];
    _btnStartGame.hidden = NO;
}

- (void)getBlankTileCreatedMsg:(SliderPuzzleViewController *)puzzleView
{
}
- (void)getBlankTileRefilledMsg:(SliderPuzzleViewController *)puzzleView
{
}
@end
