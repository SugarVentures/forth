//
//  DropDownMenu.h
//  OPPVSGUI
//
//  Created by Cao Minh Trang on 5/13/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface DropDownMenu : NSButton
{
    NSPopUpButtonCell *popupCell;
    NSMenu* popupMenu;
}

@property (retain) NSArray *dataSource;


@end
