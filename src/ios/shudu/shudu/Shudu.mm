//
//  Shudu.m
//  shudu
//
//  Created by 周海洋 on 13-9-6.
//  Copyright (c) 2013年 Autonavi Software Co., Ltd. All rights reserved.
//

#import "Shudu.h"

@interface Shudu()
- (int)valOfIdxX:(int)x IdxY:(int)y;
- (bool)fixValOfArr:(int *)arr currIdx:(int)idx;
- (bool)hasConflictsInArr:(int *)arr ofIdx:(int)idx;
@end

@implementation Shudu

- (id)initByInput:(const char *)inputArr
{
    self = [super init];
    if (self) {
        for (int i = 0; i < 81; ++i) {
            _arrNums[i] = -1;
        }
        unsigned long len = strlen(inputArr);
        int idx = 0;
        for (int i = 0; i < len; ++i) {
            if (idx >= 81) {
                break;
            }
            if (inputArr[i] > '0' && inputArr[i] <= '9') {
                _arrNums[idx++] = inputArr[i] - '0';
            } else if (inputArr[i] == '-') {
                _arrNums[idx++] = -1;
            }
        }
         for (int i = 0; i < 81; ++i) {
             if (_arrNums[i]==-1) {
                 continue;
             }
             assert(![self hasConflictsInArr:_arrNums ofIdx:i]);
         }

    }
    return self;
}

- (void)solveProblem
{
    
    int solveArr[81];
    for (int i = 0; i < 81; ++i) {
        solveArr[i] = _arrNums[i];
    }
    for (int i = 0; i < 81; ++i) {
        if (_arrNums[i] != -1) {
            //初始化时候确定的值
            continue;
        }
        bool fixed = [self fixValOfArr:solveArr currIdx:i];
        if (fixed) {
            for (int j = 0; j < 81; ++j) {
                _arrNums[j] = solveArr[j];
            }
            printf("Success!\n");
        } else {
            printf("Failed!!!\n");
        }
        break;
    }
}

- (void)printResult
{
    for (int i = 0; i < 9 ; ++i) {
        for (int j = 0; j < 9; ++j) {
            int val = [self valOfIdxX:j IdxY:i];
            printf("%d\t",val);
        }
        printf("\n");
    }
}

- (bool)hasConflictsInArr:(int *)arr ofIdx:(int)idx
{
    int xIdx = idx%9;
    int yIdx = idx/9;
    for (int x = 0 ; x < 9; ++x) {
        //检测同一行
        if (x == xIdx) {
            continue;
        }
        if (arr[x+yIdx*9]==arr[idx]) {
            return true;
        }
    }
    for (int y = 0; y < 9; ++y) {
        //检测同一列
        if (y == yIdx) {
            continue;
        }
        if (arr[xIdx+y*9]==arr[idx]) {
            return true;
        }
    }
    
    //检测同一格子
    int xStart = (xIdx/3)*3;
    int yStart = (yIdx/3)*3;
    for (int x = xStart; x < xStart + 3; ++x) {
        for (int y = yStart; y < yStart + 3; ++y) {
            if (x == xIdx && y == yIdx) {
                continue;
            }
            if (arr[x+y*9]==arr[idx]) {
                return true;
            }
        }
    }
    return false;
}

- (bool)fixValOfArr:(int *)arr currIdx:(int)idx
{
    for (int i = 1; i <= 9; ++i) {
        arr[idx] = i;
        if (![self hasConflictsInArr:arr ofIdx:idx]) {
            if (idx == 80) {
                //最后一个 无冲突，递归出口
                return true;
            }
            for (int nextIdx = idx + 1; nextIdx < 81; ++nextIdx) {
                if (_arrNums[nextIdx] != -1) {
                    //初始化时候确定的值
                    continue;
                }
                //无冲突就确定下一个
                if ([self fixValOfArr:arr currIdx:nextIdx]) {
                    return true;
                }
                break;
            }
        }
    }
    arr[idx] = -1;
    return false;
}



- (int)valOfIdxX:(int)x IdxY:(int)y
{
    return _arrNums[x+y*9];
}
@end
