//
//  ViewController.m
//  ForthViewerIosDemo
//
//  Created by Cao Minh Trang on 1/28/16.
//  Copyright © 2016 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()
@property (strong, nonatomic) IBOutlet UITextField *streamKey;
@property (strong, nonatomic) IBOutlet UIButton *playButton;
- (IBAction)playStreaming:(id)sender;
@property (strong, nonatomic) IBOutlet UIView *container;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    CGRect demoFrame = CGRectMake(0, 0, self.container.frame.size.width, self.container.frame.size.height);
    self.frameView = [[GLFrameView alloc] initWithFrame: demoFrame];
    self.frameView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    [self.container addSubview:self.frameView];
    [self.frameView setupGL];

    self.streamKey.text = @"7116f0d7-5c27-44e6-8aa4-bc4ddeea9935";
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)playStreaming:(id)sender {
    [self startStreaming:self.streamKey.text atServer:@"192.168.1.9"];
}
@end
