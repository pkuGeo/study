//
//  ViewController.h
//  SliderPuzzle
//
//  Created by 海洋 周 on 12-4-14.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "SliderPuzzleStateDelegate.h"

@interface ViewController : UIViewController <SliderPuzzleStateDelegate>

- (IBAction)startGame:(id)sender;
@end
