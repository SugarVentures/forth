{   

    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'audio_engine',
            
            'include_dirs': [
                '../include',
                '../thread',
                '../misc',
                '../../libs/libsrtp/include',
                '../../libs/libsrtp/crypto/include',
            ],
            'dependencies': [
                '../thread/thread_engine.gyp:thread_engine',
            ],


            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                    'include_dirs': [
                        'System/Library/Frameworks/CoreFoundation.Framework/Headers',
                    ],
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
                            '$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework',
                            '$(SDKROOT)/System/Library/Frameworks/AudioUnit.framework',
                            '$(SDKROOT)/System/Library/Frameworks/CoreAudio.framework',
                        ]
                    },

                    'sources': [
                        'audio_util.h',
                        'audio_capture.hpp',
                        'audio_device.cpp',
                        'audio_device.hpp',
                        'audio_engine.hpp',
                        'mac_audio_capture.hpp',
                        'mac_audio_capture.cpp',
                        'mac_audio_engine.cpp',
                        'mac_audio_engine.hpp',
                        'mac_audio_play.cpp',
                        'mac_audio_play.hpp',
                        'mac_audio_resampler.cpp',
                        'mac_audio_resampler.hpp',
                        'mac_audio_tool.cpp',
                        'mac_audio_tool.hpp',
                        'mac_utility/CAStreamBasicDescription.cpp',
                        'mac_utility/CAStreamBasicDescription.h',
                        'mac_utility/CARingBuffer.cpp',
                        'mac_utility/CARingBuffer.h'
                    ]

                }],
                
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
                        'audio_util.h',
                        'audio_capture.hpp',
                        'audio_device.cpp',
                        'audio_device.hpp',
                        'audio_engine.hpp',
                        'mac_audio_play.cpp',
                        'mac_audio_play.hpp',
                        'mac_audio_resampler.cpp',
                        'mac_audio_resampler.hpp',
                        'mac_audio_tool.cpp',
                        'mac_audio_tool.hpp',
                        'mac_utility/CAStreamBasicDescription.cpp',
                        'mac_utility/CAStreamBasicDescription.h',
                        'mac_utility/CARingBuffer.cpp',
                        'mac_utility/CARingBuffer.h'
                    ]
                }]
            ],

            
        }
    ]    
}