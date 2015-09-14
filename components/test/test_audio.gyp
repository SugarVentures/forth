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
            ],
            'dependencies': [
                '../audio/audio_engine.gyp:libaudio_engine',
            ],
            
            'conditions': [
                ['OS == "mac"', {
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