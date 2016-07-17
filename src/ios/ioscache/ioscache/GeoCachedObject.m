//
//  GeoCachedObject.m
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import "GeoCachedObject.h"

@interface GeoCachedObject ()
@property (nonatomic, strong, readwrite) id<NSCoding, NSObject> key;
@end

@implementation GeoCachedObject

@synthesize key = _key;
@synthesize value = _value;

- (id)initWithKey:(id<NSCoding,NSObject>)key value:(id<NSCoding>)value
{
    self = [super init];
    if (self) {
        self.key = key;
        self.value = value;
    }
    return self;
}

@end
