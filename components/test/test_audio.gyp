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
                '<(oppvs_3rdlibs_path)/include',
            ],
            'dependencies': [
                '../thread/thread_engine.gyp:thread_engine',
                '../audio/audio_engine.gyp:audio_engine',
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