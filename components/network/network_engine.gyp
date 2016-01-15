{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'network_engine',
            
            'dependencies': [
                '../misc/misc.gyp:libmisc2',
                '../thread/thread_engine.gyp:thread_engine',
            ],
            'include_dirs': [
                '../include',
                '../misc',
                '../thread',
                '<(oppvs_3rdlibs_path)/include',
                '<(oppvs_3rdlibs_path)/include/glib-2.0',
                '<(oppvs_3rdlibs_path)/lib/glib-2.0/include',
            ],

            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'defines': [
                        '__APPLE_USE_RFC_3542'
                    ],
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                    'libraries': [
                        '<(oppvs_3rdlibs_path)/lib/libsrtp.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libcrypto.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libgio-2.0.0.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libgobject-2.0.0.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libgthread-2.0.0.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libglib-2.0.0.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libintl.dylib',
                        '<(oppvs_3rdlibs_path)/lib/libnice.dylib'
                    ],

                }],
                ['OS == "linux"', {
                    'type': 'shared_library',
                    'libraries': [
                    
                        '../../../libs/libsrtp.a',
                        '-lssl',
                        '-lcrypto'
                    ],
                }],
                ['OS == "ios"', {
                    'type': 'static_library',
                    'defines': [
                        '__APPLE_USE_RFC_3542'
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
                'cache_buffer.hpp',

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
                'ice_stream.cpp',
                'cache_buffer.cpp'
            ]

        }
    ],


}