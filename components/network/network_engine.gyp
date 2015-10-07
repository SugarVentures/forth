{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'network_engine',
            'type': 'shared_library',
            'dependencies': [
                '../misc/misc.gyp:libmisc2',
                '../thread/thread_engine.gyp:thread_engine',
            ],
            'include_dirs': [
                '../../libs/libsrtp/include',
                '../../libs/libsrtp/crypto/include',
                '../include',
                '../misc',
                '../thread',
                '../../libs/libnice',
                '../../libs/libnice/agent',
                '/usr/local/include/glib-2.0',
                '../../libs/libnice/include'
            ],

            'conditions': [
                ['OS == "mac"', {
                    'defines': [
                        '__APPLE_USE_RFC_3542'
                    ],
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                    'libraries': [
                        '../../libs/libsrtp.a',
                    ],
                    'link_settings': {
                        'libraries': [
                            '/usr/local/lib/libgio-2.0.0.dylib',
                            '/usr/local/lib/libgobject-2.0.0.dylib',
                            '/usr/local/lib/libgthread-2.0.0.dylib',
                            '/usr/local/lib/libglib-2.0.0.dylib',
                            '/usr/local/lib/libintl.a',
                            '../../libs/libnice.10.dylib'
                        ]
                    }

                }],
                ['OS == "linux"', {
                    'libraries': [
                    
                        '../../../libs/libsrtp.a',
                        '-lssl',
                        '-lcrypto'
                    ],
                }]
            ],

            'sources': [
                #headers
                'client_socket.hpp',
                'data_stream.hpp',
                'dynamic_buffer.hpp',
                'ipaddress.hpp',
                'physical_socket.hpp',
                'server_socket.hpp',
                'socket_address.hpp',
                'srtp_socket.hpp',
                'stun_auth.hpp',
                'stun_header.hpp',
                'stun_message_handler.hpp',
                'stun_socket_address.hpp',
                'stun_socket.hpp',
                'signaling_common.hpp',
                'signaling_manager.hpp',
                'signaling_message_builder.hpp',
                'signaling_message_reader.hpp',
                'ice_common.hpp',
                'ice_manager.hpp',
                'ice_stream.hpp',

                #sources
                 'client_socket.cpp',
                'data_stream.cpp',
                'dynamic_buffer.cpp',
                'ipaddress.cpp',
                'physical_socket.cpp',
                'server_socket.cpp',
                'socket_address.cpp',
                'srtp_socket.cpp',
                'stun_message_handler.cpp',
                'stun_socket_address.cpp',
                'stun_socket.cpp',
                'signaling_manager.cpp',
                'signaling_message_builder.cpp',
                'signaling_message_reader.cpp',
                'ice_manager.cpp',
                'ice_stream.cpp'
            ]

        }
    ],


}