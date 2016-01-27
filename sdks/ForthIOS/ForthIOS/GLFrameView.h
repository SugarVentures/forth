//
//  GLFrameView.h
//  iosDemo
//
//  Created by Cao Minh Trang on 10/30/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


@interface GLFrameView : UIView
{
@private

}

- (void)setupGL;
- (void)render: (GLubyte*)data withWidth: (GLuint)width andHeight: (GLuint)height freeWhenDone: (bool)flag;
- (void)render: (GLubyte*)data withWidth: (GLuint)width andHeight: (GLuint)height;

@end
