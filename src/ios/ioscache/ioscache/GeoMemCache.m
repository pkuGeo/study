//
//  GeoMemCache.m
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import "GeoMemCache.h"
#import "GeoCachedObject.h"

@interface GeoMemCache () {
@private 
    __strong NSMutableArray *_arrCache;//LRU record
    int _maxSize;
}


- (void)removeUnwantedCacheObject;

- (BOOL)isCacheFull;//cache空间满

- (void)handleMemWarning:(NSNotification *)notification;

@end

@implementation GeoMemCache

- (id)initWithCacheSize:(int)size
{
    self = [super init];
    if (self) {
        _maxSize = size;
        if (_maxSize > 0) {
            _arrCache = [[NSMutableArray alloc] init];
        }
        //To support release memory automatically if recieve memory warning
        //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(handleMemWarning:) name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
    }
    return self;
}

- (void)addObject2Cache:(id<NSCoding>)value withKey:(id<NSCoding, NSObject>)key
{
    if (_maxSize <= 0) {
        return;
    }
    if (key == nil || value == nil) {
        return;
    }
    if (_arrCache == nil) {
        @synchronized(self) {
            if (_arrCache == nil) {
                _arrCache = [[NSMutableArray alloc] init];
            }
        }
    }
    @synchronized(self) {
        if ([self isCacheFull]) {
            [self removeUnwantedCacheObject];
        }
        GeoCachedObject *cachedObject = [[GeoCachedObject alloc] initWithKey:key value:value];
        [_arrCache insertObject:cachedObject atIndex:0];
    }
}

- (id)getCachedObjectByKey:(id)key
{
    if (_maxSize <= 0) {
        return nil;
    }
    if (_arrCache != nil) {
        @synchronized(self) {
            if (_arrCache == nil) {
                return nil;
            }
            for (__strong GeoCachedObject *cachedItem in _arrCache) {
                if ([cachedItem.key isEqual:key]) {
                    [_arrCache removeObject:cachedItem];
                    [_arrCache insertObject:cachedItem atIndex:0];
                    return cachedItem.value;
                }
                
            }
            
        }
    }
    return nil;
}

- (void)removeUnwantedCacheObject
{
    [_arrCache removeLastObject];
}

- (void)handleMemWarning:(NSNotification *)notification
{
    if (_arrCache != nil) {
        @synchronized(self) {
            if (_arrCache != nil) {
                _arrCache = nil;
            }
        }
    }
}

- (BOOL)isCacheFull
{
    return _maxSize > 0 && _arrCache != nil && [_arrCache count] >= _maxSize;
}

@end
