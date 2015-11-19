//
//  ForthPlayer.m
//  ForthIOS
//
//  Created by Cao Minh Trang on 11/4/15.
//  Copyright © 2015 Cao Minh Trang. All rights reserved.
//

#import "ForthPlayer.h"

#include "mac_audio_play.hpp"

using namespace oppvs;

@interface ForthPlayer()
{
@private
    MacAudioPlay* _player;
}

@end

@implementation ForthPlayer

- (instancetype)init
{
    self = [super init];
    if (self) {
        AudioDevice output(0);
        _player = new MacAudioPlay(output, 44100, 2);

        if (_player->init() < 0)
        {
            NSLog(@"Cannot init audio player");
        }
    }
    return self;
}

@end