//
//  main.m
//  shudu
//
//  Created by 周海洋 on 13-9-6.
//  Copyright (c) 2013年 Autonavi Software Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Shudu.h"
#include <sys/time.h>
int main(int argc, const char * argv[])
{

    @autoreleasepool {
        
        // insert code here...
        const char *arr =
        "1---9678-"
        "----25---"
        "-----8-39"
        "96-------"
        "--7------"
        "-3-----51"
        "--2----93"
        "-1--7-5--"
        "8----912-";
//        time_t s = time(NULL);
//        for (int i = 0; i < 10000; ++i) {
            Shudu *shudu = [[Shudu alloc] initByInput:arr];
            [shudu solveProblem];
            [shudu printResult];
//        }
//        time_t e = time(NULL);
//        printf("%ld",e-s);
    }
    return 0;
}

