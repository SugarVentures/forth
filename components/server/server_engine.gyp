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
                #'../network/network_engine.gyp:network_engine',
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
            ],
            'conditions': [
                ['OS == "mac"', {
                    'defines': [
                        '__APPLE_USE_RFC_3542'
                    ],
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    }
                }],
                ['OS == "linux"', {
                    'libraries': [                    
                        '-lssl',
                        '-lcrypto'
                    ],
                }]
            ],
            'sources': [
                'signaling_server.hpp',
                'signaling_server.cpp',
                'signaling_server_thread.hpp',
                'signaling_server_thread.cpp',
                'signaling_server_subthread.hpp',
                'signaling_server_subthread.cpp',
                '../network/dynamic_buffer.hpp',
                '../network/dynamic_buffer.cpp',
                '../network/data_stream.hpp',
                '../network/data_stream.cpp',
                '../network/ipaddress.hpp',
                '../network/ipaddress.cpp',
                '../network/socket_address.hpp',
                '../network/socket_address.cpp',
                '../network/physical_socket.hpp',
                '../network/physical_socket.cpp',
                '../network/signaling_common.hpp',
                '../network/signaling_message_builder.hpp',
                '../network/signaling_message_builder.cpp',
                '../network/signaling_message_reader.hpp',
                '../network/signaling_message_reader.cpp'
            ]
        }
    ]    
}