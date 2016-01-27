//
//  ViewController.m
//  ForthBroadcasterIosDemo
//
//  Created by Cao Minh Trang on 1/27/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "GLFrameView.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CGRect demoFrame = CGRectMake(10, 10, self.previewView.frame.size.width - 20, self.previewView.frame.size.height - 20);
    self.frameView = [[GLFrameView alloc] initWithFrame: demoFrame];
    self.frameView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    [self.previewView addSubview:self.frameView];
    [self.frameView setupGL];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
