{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'testaudio',
            'type': 'executable',
            'include_dirs': [
                '../include',
                '../audio',
                '../thread',
                '../encoding',
                '../../libs/libopus/include'
            ],
            'dependencies': [
                '../thread/thread_engine.gyp:thread_engine',
                '../audio/audio_engine.gyp:libaudio_engine',
                '../encoding/encoding_engine.gyp:encoding_engine'
            ],
            
            'conditions': [
                ['OS == "mac"' or 'OS == "ios"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                }],
            ],
            'sources': [
                'testaudio.cpp'
            ]

        }
    ]    
}