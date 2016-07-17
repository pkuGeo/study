//
//  Shudu.h
//  shudu
//
//  Created by 周海洋 on 13-9-6.
//  Copyright (c) 2013年 Autonavi Software Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Shudu : NSObject {
    int _arrNums[81];
}

- (id)initByInput:(const char *)inputArr;

- (void)solveProblem;
- (void)printResult;


@end
