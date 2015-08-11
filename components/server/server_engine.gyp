{   

    'includes': [
        '../../common.gypi'
    ],
    
    'targets': [
        {
            'target_name': 'libstun_server',
            'type': 'static_library',
            'include_dirs': [
                '../include',
                '../thread',
                '../network',
                '../misc'
            ],
            'dependencies': [
                '../thread/thread_engine.gyp:thread_engine',
                '../network/network_engine.gyp:network_engine',
            ],
            'sources': [
                'stun_server.hpp',
                'stun_server.cpp',
                'stun_server_thread.hpp',
                'stun_server_thread.cpp'
            ]

        }
        ,
        {
            'target_name': 'libsignaling_server',
            'type': 'static_library',
            'include_dirs': [
                '../include',
                '../thread',
                '../network',
                '../misc'
            ],
            'dependencies': [
                '../thread/thread_engine.gyp:thread_engine',
                '../network/network_engine.gyp:network_engine',
            ],
            'sources': [
                'signaling_server.hpp',
                'signaling_server.cpp',
                'signaling_server_thread.hpp',
                'signaling_server_thread.cpp'
            ]
        }
    ]    
}