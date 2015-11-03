//
//  ViewController.m
//  iosDemo
//
//  Created by Cao Minh Trang on 10/29/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "GLFrameView.h"


@interface ViewController ()
{
@private
    ForthStreaming *mForth;
}

@property (nonatomic, strong) GLFrameView *demoView;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    CGRect demoFrame = CGRectMake(10, 10, _container.frame.size.width - 20, _container.frame.size.height - 20);
    
    self.demoView = [[GLFrameView alloc] initWithFrame:demoFrame];
    self.demoView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.container addSubview:self.demoView];
    
    @autoreleasepool {
        mForth = [[ForthStreaming alloc] init];
        mForth.delegate = self;
    }
    
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)buttonDidPress:(UIButton *)sender {
    //show whatever text in the text field to label, and print to console
    NSString *stringToPrint = self.textField.text;
    
    //show on label
    self.label.text = stringToPrint;
    
    //log to console
    NSLog(@"log string to log %@", stringToPrint);
    [self.demoView setupGL];
    [mForth startStreaming];
    
}

- (IBAction)textFieldDidEnd:(UITextField *)sender {
    //finished editing text
}

#pragma mark - Forth Streaming delegate
- (void)frameCallback:(void *)data withWidth:(int)width andHeight:(int)height andStride:(int)stride
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.demoView render:data withWidth:width andHeight:height];
    });
}
@end
