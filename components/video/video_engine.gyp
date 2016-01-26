{   

    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'video_engine',            
            'include_dirs': [
                '../include',
            ],

            'conditions': [
                ['OS == "ios"', {
                    "type": 'static_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                    'include_dirs': [
                        'System/Library/Frameworks/CoreFoundation.Framework/Headers',
                        'System/Library/Frameworks/AVFoundation.Framework/Headers',
                    ],
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
                            '$(SDKROOT)/System/Library/Frameworks/AVFoundation.framework',
                            '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
                            '$(SDKROOT)/System/Library/Frameworks/CoreMedia.framework',
                        ]
                    },

                    'sources': [
                        'video_engine.hpp',
                        'video_capture.hpp',
                        'ios_video_capture.hpp',
                        'ios_video_capture.cpp',
                        'ios_video_capture_interface.h',
                        'ios_video_avfoundation_capture.mm',
                        'ios_video_engine.hpp',
                        'ios_video_engine.cpp',
                        'ios_video_source_info.mm'
                    ]
                }]
            ],

            
        }
    ]    
}