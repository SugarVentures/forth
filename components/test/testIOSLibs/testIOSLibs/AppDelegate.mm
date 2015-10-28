//
//  AppDelegate.m
//  testIOSLibs
//
//  Created by Cao Minh Trang on 10/26/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import "AppDelegate.h"
#import <ForthIOS.h>

@interface AppDelegate ()
{
    ForthIOS* mForth;
}
@end

@implementation AppDelegate



- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    @autoreleasepool {
        mForth = [[ForthIOS alloc] init];
        
        NSMutableArray *listSources = [[NSMutableArray alloc] init];
        NSMutableDictionary *source = [[NSMutableDictionary alloc] init];
        [source setObject: kStreamSourceTypeVideo forKey:kStreamSourceType];
        [source setObject: [NSNumber numberWithInt:1280] forKey:kStreamVideoWidth];
        [source setObject: [NSNumber numberWithInt:780] forKey:kStreamVideoHeight];
        [source setObject: [NSNumber numberWithInt:5120] forKey:kStreamVideoStride];
        [listSources addObject:source];
        
        [mForth initStreamingEngine:listSources];
    }
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
