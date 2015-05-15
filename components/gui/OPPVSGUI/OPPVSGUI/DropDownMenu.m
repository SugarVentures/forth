//
//  DropDownMenu.m
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "DropDownMenu.h"

@implementation DropDownMenu

@synthesize dataSource;


- (void)runPopUp:(NSEvent *)theEvent
{
    @autoreleasepool {
        popupCell = [[NSPopUpButtonCell alloc] initTextCell:@""];
        [popupCell setPullsDown:YES];
        [popupCell setPreferredEdge:NSMaxYEdge];
        popupMenu = [[NSMenu alloc] init];
        
        //Add items from data source
        int index = 0;
        for (id obj in dataSource)
        {
            @autoreleasepool {
                NSDictionary *item = (NSDictionary*)obj;
                NSMenuItem *menuItem;
                if ([[item objectForKey:@"type"] isEqualToString:@"Dash"])
                {
                    menuItem = [NSMenuItem separatorItem];
                }
                else
                {
                    NSString *title = [item objectForKey:@"title"];
                    menuItem = [[NSMenuItem alloc] initWithTitle:title action:@selector(addSource:) keyEquivalent:@""];
                    [menuItem setTarget:[self target]];
                }
                [menuItem setTag:index];
                [popupMenu addItem:menuItem];
                index++;
            }
        }
        
        NSMenu* menu = [popupMenu copy];
        [menu insertItemWithTitle:@"" action:nil keyEquivalent:@"" atIndex:0];
        [popupCell setMenu:menu];
        
        [popupCell performClickWithFrame:[self bounds] inView: self];
        [self setNeedsDisplay:true];
    }
}

- (void)mouseDown:(NSEvent*)theEvent
{
    [self runPopUp:theEvent];
}

- (void)addSource: (id)sender
{
    
}


@end
