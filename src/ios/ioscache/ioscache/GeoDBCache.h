//
//  GeoDBCache.h
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import <Foundation/Foundation.h>

@interface GeoDBCache : NSObject

+ (void)setDBPath:(NSString *)path;

@property (nonatomic, strong, readonly) NSString *tableName; //name of the cache table

- (id)initWithTableName:(NSString *)tableName;

- (void)addObject2Cache:(id<NSCoding>)value withKey:(id<NSCoding, NSObject>)key;

- (id<NSCoding>)getCachedObjectByKey:(id<NSCoding, NSObject>)key;

@end
