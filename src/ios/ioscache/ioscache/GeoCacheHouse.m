//
//  GeoCacheHouse.m
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import "GeoCacheHouse.h"
#import "GeoDBCache.h"
#import "GeoMemCache.h"

@interface GeoCacheHouse ()
{
@private
    __strong GeoDBCache *_dbCache;
    __strong GeoMemCache *_memCache;
}
@end


@implementation GeoCacheHouse

+ (void)setDatabasePath:(NSString *)dbPath
{
    [GeoDBCache setDBPath:dbPath];
}

- (id)initWithMemCacheNum:(int)memCacheNum dataTableName:(NSString *)dataTableName
{
    self = [super init];
    if (self) {
        if (memCacheNum > 0) {
            _memCache = [[GeoMemCache alloc] initWithCacheSize:memCacheNum];
        }
        if (dataTableName > 0) {
            _dbCache = [[GeoDBCache alloc] initWithTableName:dataTableName];
        }
    }
    return self;
}

- (id<NSCoding>)getCachedObjectByKey:(id<NSCoding,NSObject>)key
{
    id<NSCoding> val = nil;
    if (_memCache != nil) {
        val = [_memCache getCachedObjectByKey:key];
    }
    if (val != nil) {
        return val;
    }
    if (_dbCache != nil) {
        val = [_dbCache getCachedObjectByKey:key];
    }
    if (val != nil && _memCache != nil) {
        [_memCache addObject2Cache:val withKey:key];
    }
    return val;
}

- (void)addObject2Cache:(id<NSCoding>)value withKey:(id<NSCoding, NSObject>)key
{
    if (_memCache != nil) {
        [_memCache addObject2Cache:value withKey:key];
    }
    if (_dbCache != nil) {
        [_dbCache addObject2Cache:value withKey:key];
    }
}
@end
