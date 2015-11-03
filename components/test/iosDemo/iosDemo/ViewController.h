//
//  ViewController.h
//  iosDemo
//
//  Created by Cao Minh Trang on 10/29/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ForthIOS.h"

@interface ViewController : UIViewController<ForthStreamingDelegate>

@property (weak, nonatomic) IBOutlet UITextField    *textField;
@property (weak, nonatomic) IBOutlet UILabel        *label;
@property (weak, nonatomic) IBOutlet UIButton       *button;

@property (weak, nonatomic) IBOutlet UIView         *container;
- (IBAction)buttonDidPress:(UIButton *)sender;
- (IBAction)textFieldDidEnd:(UITextField *)sender;

@end

