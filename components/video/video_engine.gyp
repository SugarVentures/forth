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
                    ],
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',

                        ]
                    },

                    'sources': [
                        'video_engine.hpp',
                        'video_capture.hpp',
                        'ios_video_capture.hpp',
                        'ios_video_capture.cpp'
                    ]
                }]
            ],

            
        }
    ]    
}