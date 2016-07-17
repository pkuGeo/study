//
//  GeoCacheHouse.h
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import <Foundation/Foundation.h>


/**
 * Some requirements to use this class to work normal
 * 1. The key and the value should both support protocol NSCoding, 
 *   for storing these into database
 * 2. The key should response to selector:isEqual: (method in protocol NSObject), 
 *   because the module uses it to judge if cache hits
 * 3. The key should never been changed outside the module
 *   (Actually it will not be changed in this module either). 
 *   Or you need to rewrite the function: addObject2Cache:withKey:.
 */

@interface GeoCacheHouse : NSObject

/**
 * Should be called before use if you wanna all cached tables is in a customized database;
 * Otherwise the this module will use a default db: HomeDirectory()/Library/Cache/geocache.db
 */
+ (void)setDatabasePath:(NSString *)dbPath; 


/**
 * To init a Cache House with num of memCache and a Customized table name which will be part of
 * user defined database or HomeDirectory()/Library/Caches/geocache.db
 * 
 */
- (id)initWithMemCacheNum:(int)memCacheNum dataTableName:(NSString *)dataTableName;


/**
 * Get the cached object by key, if cache hits, return the object, otherwise return nil
 * And also the key should response to selector: isEqual, for this fucntion will be called to 
 * verify if cache hits.
 */
- (id<NSCoding>)getCachedObjectByKey:(id<NSCoding,NSObject>)key;


/**
 * Attention: key should never been changed outsize this module,
 * Otherwise you should rewrite this function to copy the key instead of retain this
 * And also after NSCoding different key should have different value
 */
- (void)addObject2Cache:(id<NSCoding>)value withKey:(id<NSCoding, NSObject>)key;

@end
