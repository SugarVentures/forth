//
//  ViewController.h
//  iosTestRecordingSDK
//
//  Created by Cao Minh Trang on 1/25/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController

@property (weak, nonatomic) IBOutlet UIView  *container;
- (IBAction)switchCamera:(id)sender;
@property (strong, nonatomic) IBOutlet UIButton *switchCameraButton;

@end

