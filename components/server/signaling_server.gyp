{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'signaling_server',
            'type': 'executable',
            'include_dirs': [
                '../include',
                '../misc',
                '../network',
                '../thread'
            ],
            'dependencies': [
                'server_engine.gyp:libsignaling_server',
            ],
            
            'conditions': [
                ['OS == "mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                }],
                ['OS == "linux"', {
                }]
            ],
            'sources': [
                'signaling_server_main.cpp'
            ]

        }
    ]    
}