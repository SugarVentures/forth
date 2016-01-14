{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'streaming_engine',
            
            'dependencies': [
                '../network/network_engine.gyp:network_engine',
                '../encoding/encoding_engine.gyp:encoding_engine',
                '../thread/thread_engine.gyp:thread_engine',
                '../misc/misc.gyp:libmisc1',
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
                '<(oppvs_3rdlibs_path)/include',
                '<(oppvs_3rdlibs_path)/include/glib-2.0',
                '<(oppvs_3rdlibs_path)/lib/glib-2.0/include'
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
                    
                    'link_settings': {
                        'libraries': [
                            '<(oppvs_3rdlibs_path)/lib/libgio-2.0.0.dylib',
                            '<(oppvs_3rdlibs_path)/lib/libgobject-2.0.0.dylib',
                            '<(oppvs_3rdlibs_path)/lib/libgthread-2.0.0.dylib',
                            '<(oppvs_3rdlibs_path)/lib/libglib-2.0.0.dylib',
                            '<(oppvs_3rdlibs_path)/lib/libintl.a',
                            '<(oppvs_3rdlibs_path)/lib/libnice.10.dylib'
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
                'streaming_send_thread.hpp',
                'video_packetizer.cpp',
                'video_packetizer.h'
            ]

        }
    ],


}