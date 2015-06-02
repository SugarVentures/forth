//
//  ViewController.m
//  OPPVSClient
//
//  Created by Cao Minh Trang on 3/27/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#import "ViewController.h"
#import "FrameView.h"

@interface ViewController ()
{
    NSMutableArray *listSources;
}

@end
@implementation ViewController

@synthesize previewView;


- (id) init
{
    
    self = [super init];

    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    /*[hostPreviewLayer setWantsLayer:YES];
    previewView = [OpenGLFrame layer];
    [previewView setAsynchronous:NO];
    [previewView setNeedsDisplay];
    [hostPreviewLayer setLayer:previewView];*/
    
    [serverIP setStringValue:@"127.0.0.1"];
    listSources = [[NSMutableArray alloc] init];
}

- (void)viewWillAppear
{
    
    document = [self.view.window.windowController document];
    FrameView *superview = (FrameView*)hostPreviewLayer;
    [superview setBackingScaleFactor:document.windowForSheet.backingScaleFactor];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

- (void) renderFrame: (oppvs::PixelBuffer*) pf
{
    if (pf != NULL)
    {
        NSInteger index = pf->order;
        __block id subview = nil;
        NSMutableData *data;
        
        NSArray *filtered = [listSources filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"source = %d", pf->source]];
        
        if ((unsigned long)[filtered count] > 0)
        {
            NSDictionary *item = [filtered objectAtIndex:0];
            subview = [item objectForKey:@"view"];
            data = [item objectForKey:@"data"];
            [data replaceBytesInRange:NSMakeRange(0, pf->width[0]*pf->height[0]*4) withBytes:pf->plane[0]];
        }
        else
        {
            dispatch_sync(dispatch_get_main_queue(), ^{
                subview = [self addSubView:NSMakeRect(0, 0, [hostPreviewLayer bounds].size.width, [hostPreviewLayer bounds].size.height) atIndex:index];
            });
            if (subview == nil)
                return;
            
            NSNumber *sourceid = [NSNumber numberWithUnsignedShort: pf->source];
            NSMutableDictionary *dict = [[NSMutableDictionary alloc] init];
            data = [NSMutableData dataWithCapacity: pf->width[0]*pf->height[0]*4];
            [data replaceBytesInRange:NSMakeRange(0, pf->width[0]*pf->height[0]*4) withBytes:pf->plane[0]];
            [dict setObject:subview forKey:@"view"];
            [dict setObject:sourceid forKey:@"source"];
            [dict setObject:data forKey:@"data"];
            [listSources addObject: dict];
        }
        
        OpenGLFrame *view = (OpenGLFrame*)subview;
        [view setFrameWidth:pf->width[0]];
        [view setFrameHeight:pf->height[0]];
        [view setPixelBuffer:(GLubyte*)[data mutableBytes]];
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [view setNeedsDisplay];
        });


    }
    
}


- (IBAction)Play:(id)sender {
    NSString* ip = [serverIP stringValue];
    NSInteger port = [serverPort integerValue];
    [document initReceiver:ip withPort:port];
}

- (id)addSubView: (NSRect)frame atIndex: (NSInteger)index
{
    FrameView *superview = (FrameView*)hostPreviewLayer;
    id user = [superview addWindow:frame atIndex:index];
    return user;
}


- (IBAction)showPreview:(id)sender {
}
@end
