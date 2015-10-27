{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'streaming_engine',
            
            'dependencies': [
                '../network/network_engine.gyp:network_engine',
            ],
            'include_dirs': [
                '../../libs/libsrtp/include',
                '../../libs/libsrtp/crypto/include',
                '../include',
                '../misc',
                '../thread',
                
                '../network',
                '../encoding',
                '../error',
                '../interface',
                '../audio',
                '../../libs/libyuv/include',
                '../../libs/libvpx',
                '../../libs/libopus/include',

                '../../libs/libnice',
                '../../libs/libnice/agent',
                '/usr/local/include/glib-2.0',
                '../../libs/libnice/include'
            ],

            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
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
                'audio_packet_handler.cpp',
                'audio_packet_handler.h',
                'depacketizer.cpp',
                'depacketizer.h',
                'segment_builder.cpp',
                'segment_builder.h',
                'segment_reader.cpp',
                'segment_reader.h',
                'signaling_handler.cpp',
                'signaling_handler.hpp',
                'streaming_engine.hpp',
                'streaming_engine.cpp',
                'streaming_send_thread.cpp',
                'streaming_send_thread.hpp'
            ]

        }
    ],


}