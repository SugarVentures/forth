{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'stun_server',
            'type': 'executable',
            'include_dirs': [
                '../include',
                '../misc',
                '../network',
                '../thread'
            ],
            'dependencies': [
                'server_engine.gyp:libstun_server',
            ],
            
            'conditions': [
                ['OS == "mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                    'libraries': [
                        '../../libs/libsrtp.a',
                    ],

                }],
                ['OS == "linux"', {
                    'libraries': [
                    
                        '../../../libs/libsrtp.a',
                    ],
                }]
            ],
            'sources': [
                'stun_server_main.cpp'
            ]

        }
    ]    
}