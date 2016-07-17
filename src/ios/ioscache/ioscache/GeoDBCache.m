//
//  GeoDBCache.m
//  ioscacher
//
//  Created by 海洋 周 on 12-4-22.
//  Copyright (c) 2012年 . All rights reserved.
//

#import <Foundation/Foundation.h>
#import <sqlite3.h>

#import "GeoDBCache.h"

@interface GeoDBCache ()
{
    sqlite3 *_database;
}
@property (nonatomic, strong, readwrite) NSString *tableName;
@end

@implementation GeoDBCache

@synthesize tableName = _tableName;
static __strong NSString *s_dbPath = nil;

+ (void)setDBPath:(NSString *)path
{
    if (s_dbPath != nil) {
        return;
    }
    s_dbPath = path;
}

- (id)initWithTableName:(NSString *)tableName
{
    self = [super init];
    if (self) {
        if (s_dbPath == nil) {
            s_dbPath = [NSHomeDirectory() stringByAppendingString:@"/Library/Caches/geocache.db"];
        }
        self.tableName = tableName;
        
        bool needCreateTable = ![[NSFileManager defaultManager] fileExistsAtPath:s_dbPath];
        int rt = sqlite3_open([s_dbPath UTF8String], &_database);
        if (rt!=SQLITE_OK) {
            [NSException raise:@"GeoDBCache:Failed to Open Database" format:@"Path:%@",s_dbPath];
        }
        if (needCreateTable) {
            char sql[500] = {0};
            strcpy(sql, "CREATE TABLE ");
            strcat(sql, [self.tableName UTF8String]);
            strcat(sql, " (key BLOB primary key, value BLOB)");
            sqlite3_stmt *statement = NULL;
            if (sqlite3_prepare_v2(_database, sql, -1, &statement, nil) != SQLITE_OK) {
               [NSException raise:@"GeoDBCache:Failed to prepare statement" format:@"Path: %@  Table: %@",s_dbPath,self.tableName];
            }
            rt = sqlite3_step(statement);
            sqlite3_finalize(statement);
            if (rt != SQLITE_DONE) {
                [NSException raise:@"GeoDBCache:Failed to Create Table" format:@"Path: %@  Table: %@",s_dbPath,self.tableName];
            }
        }
    }
    return self;
}

- (void)addObject2Cache:(id<NSCoding>)value withKey:(id<NSCoding, NSObject>)key
{
    if (key == nil || value == nil) {
        return;
    }
    NSData *keyData = [NSKeyedArchiver archivedDataWithRootObject:key];
    NSData *valueData = [NSKeyedArchiver archivedDataWithRootObject:value];
    
    sqlite3_stmt *statement = NULL;
    char sql[500] = {0};
    strcpy(sql, "INSERT OR REPLACE INTO ");
    strcat(sql, [self.tableName UTF8String]);
    strcat(sql, " (key, value) VALUES(?, ?)");
    int rt = 0;
    @synchronized(self) {
    rt = sqlite3_prepare_v2(_database, sql, -1, &statement, nil);
    if (rt != SQLITE_OK) {
        NSLog(@"GeoDBCache: failed to prepare statement");
        return;
    }
    sqlite3_bind_blob(statement, 1, [keyData bytes], [keyData length], SQLITE_TRANSIENT);
    sqlite3_bind_blob(statement, 2, [valueData bytes], [valueData length], SQLITE_TRANSIENT);
    rt = sqlite3_step(statement);
    sqlite3_finalize(statement);
    }
    if (rt == SQLITE_ERROR) {
        NSLog(@"GeoDBCache: failed to insert cache");
    }
}

- (id<NSCoding>)getCachedObjectByKey:(id<NSCoding, NSObject>)key
{
    if (key == nil) {
        return nil;
    }
    NSData *keyData = [NSKeyedArchiver archivedDataWithRootObject:key];
    NSData *valueData = nil;
    
    sqlite3_stmt *statement = NULL;
    char sql[500] = {0};
    strcpy(sql, "SELECT value FROM ");
    strcat(sql, [self.tableName UTF8String]);
    strcat(sql, " WHERE key = ?");
    @synchronized(self) {
    int rt = sqlite3_prepare_v2(_database, sql, -1, &statement, nil);
    if (rt != SQLITE_OK) {
        NSLog(@"GeoDBCache: failed to prepare statement");
        return nil;
    }
    
    sqlite3_bind_blob(statement, 1, [keyData bytes], [keyData length], SQLITE_TRANSIENT);
    
    rt = sqlite3_step(statement);
    
    if (rt == SQLITE_ROW) {
        Byte *byteData = (Byte *)sqlite3_column_blob(statement, 0);
        int len = sqlite3_column_bytes(statement, 0);
        valueData = [NSData dataWithBytes:byteData length:len];
        
    } else if (rt == SQLITE_ERROR) {
        NSLog(@"GeoDBCache: failed to exec query");
    }
    sqlite3_finalize(statement);
    }
    if (valueData == nil) {
        return nil;
    }
    return [NSKeyedUnarchiver unarchiveObjectWithData:valueData];
}

- (void)dealloc
{
    if (_database != nil) {
        sqlite3_close(_database);
        _database = nil;
    }
}
@end
