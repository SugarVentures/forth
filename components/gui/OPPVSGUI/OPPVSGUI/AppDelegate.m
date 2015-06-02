//
//  AppDelegate.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 3/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application

}
 
- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)flag
{
    return NO;
}

- (void)windowWillClose:(NSNotification *)notification
{
    NSLog(@"Window will close");
}

@end
