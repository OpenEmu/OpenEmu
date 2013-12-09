//
//  OESQLiteDatabase.m
//  LookupDB
//
//  Created by Christoph Leimbrock on 07/12/13.
//  Copyright (c) 2013 Christoph Leimbrock. All rights reserved.
//

#import "OESQLiteDatabase.h"
#import <sqlite3.h>

NSString * const OESQLiteErrorDomain = @"OESQLiteErrorDomain";
@interface OESQLiteDatabase ()
{
    sqlite3 *connection;
}
@end
@implementation OESQLiteDatabase

- (id)initWithURL:(NSURL*)url error:(NSError *__autoreleasing *)error
{
    if(self = [super init])
    {
        NSString *path = [url path];
        int sqlerr = sqlite3_open([path cStringUsingEncoding:NSUTF8StringEncoding], &connection);
        if(sqlerr != SQLITE_OK)
        {
            sqlite3_close(connection);
            if(error != NULL)
            {
                NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : [NSString stringWithFormat:@"%s", sqlite3_errmsg(connection)]};
                *error = [NSError errorWithDomain:OESQLiteErrorDomain code:sqlerr userInfo:userInfo];
            }
            self = nil;
        }
    }
    return self;
}

- (void)dealloc
{
    int res = sqlite3_close(connection);
    if(res != SQLITE_OK)
    {
        DLog(@"Could not close SQL Database correctly. (Error Code %d: %s)", res, sqlite3_errmsg(connection));
    }
}

- (id)executeQuery:(NSString*)sql error:(NSError *__autoreleasing *)error
{
    sqlite3_stmt *stmt = NULL;
    int        sql_err = SQLITE_OK;
    const char   *csql = [sql cStringUsingEncoding:NSUTF8StringEncoding];
    
    sql_err = sqlite3_prepare_v2(connection, csql, -1, &stmt, NULL);
    if(sql_err != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        if(error != NULL)
        {
            NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : [NSString stringWithFormat:@"%s", sqlite3_errmsg(connection)]};
            *error = [NSError errorWithDomain:OESQLiteErrorDomain code:sql_err userInfo:userInfo];
        }
        return nil;
    }
    
    NSMutableArray *result = [NSMutableArray array];
    
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        int columnCount = sqlite3_column_count(stmt);
        NSMutableDictionary *dict = [NSMutableDictionary dictionaryWithCapacity:columnCount];
        
        for (int i=0; i < columnCount; i++)
        {
            const char * cname = sqlite3_column_name(stmt, i);
            NSString *name = [NSString stringWithCString:cname encoding:NSUTF8StringEncoding];
            
            id value = [self _valueOfSQLStatement:stmt atColumn:i];
            if(value)
                [dict setObject:value forKey:name];
        }
        
        [result addObject:dict];
    }
    
    sqlite3_finalize(stmt);
    
    return result;
}

- (id)_valueOfSQLStatement:(sqlite3_stmt*)stmt atColumn:(int)column
{
    id  value = nil;
    int type  = sqlite3_column_type(stmt, column);
    switch (type) {
        case SQLITE_INTEGER:
            value = [NSNumber numberWithFloat:sqlite3_column_int(stmt, column)];
            break;
        case SQLITE_FLOAT:
            value = [NSNumber numberWithDouble:sqlite3_column_double(stmt, column)];
            break;
        case SQLITE_TEXT:
        {
            const char * ctext = (const char * )sqlite3_column_text(stmt, column);
            value = [NSString stringWithCString:ctext encoding:NSUTF8StringEncoding];
            break;
        }
        case SQLITE_NULL:
        {
            value = nil;
            break;
        }
        case SQLITE_BLOB:
        {
            DLog(@"SQLITE Type Blob not supported right now");
            value = nil;
            break;
        }
        default:
            DLog(@"Unknown data type: %d", type);
            value = nil;
            break;
    }
    return value;
}

@end
