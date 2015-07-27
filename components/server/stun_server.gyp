{
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
            'libraries': [
                '../../libs/libsrtp.a',
            ],
            'conditions': [
                ['OS == "mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },

                }]
            ],
            'sources': [
                'stun_server_main.cpp'
            ]

        }
    ]    
}