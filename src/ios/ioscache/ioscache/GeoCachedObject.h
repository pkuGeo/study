//
//  GeoCachedObject.h
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import <Foundation/Foundation.h>

@interface GeoCachedObject : NSObject

@property (nonatomic, strong, readonly) id<NSCoding, NSObject> key;
@property (nonatomic, strong) id<NSCoding> value;

- (id)initWithKey:(id<NSCoding, NSObject>)key value:(id<NSCoding>)value;

@end
