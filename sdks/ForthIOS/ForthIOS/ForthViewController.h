//
//  ForthViewController.h
//  ForthIOS
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GLFrameView.h"

@interface ForthViewController : UIViewController

@property (nonatomic, strong) GLFrameView *frameView;

- (void) startCaptureSession;

@end
