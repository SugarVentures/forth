//
//  ForthViewerController.h
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/28/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GLFrameView.h"

@interface ForthViewerController : UIViewController

@property (nonatomic, strong) GLFrameView *frameView;

- (void) startStreaming: (NSString*)streamKey atServer: (NSString*)serverAddress;

@end
