{   

    'includes': [
        '../../common.gypi'
    ],
    
    'targets': [
        {
            'target_name': 'libaudio_engine',
            'type': 'static_library',
            'include_dirs': [
                '../include',
            ],
            'conditions': [
                ['OS == "mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
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
                    }
                }]
            ],
            'sources': [
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
        }
    ]    
}