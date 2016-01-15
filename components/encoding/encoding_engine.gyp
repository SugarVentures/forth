{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'encoding_engine',
            
            'dependencies': [
                '../../libs/libyuv/libyuv.gyp:yuv',
            ],
            
            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                    'libraries': [
                        '<(oppvs_3rdlibs_path)/lib/libopus.a',
                        '<(oppvs_3rdlibs_path)/lib/libvpx.a'
                    ],
                    'include_dirs': [
                        '../include',
                        '../error',
                        '../misc',
                        '<(oppvs_3rdlibs_path)/include',
                        '../../libs/libyuv/include',
                    ],

                }],
                ['OS == "linux"', {
                    'type': 'shared_library',
                    'libraries': [
                        '<(oppvs_3rdlibs_path)/lib/libopus.a',
                    ],
                }],
                ['OS == "ios"', {
                    'type': 'static_library',
                    'include_dirs': [
                        '../include',
                        '../error',
                        '../misc',
                        '../../libs/libyuv/include',
                        '<(oppvs_3rdlibs_path)/include',
                    ],
                }]
            ],

            'sources': [
                'audio_opus_encoder.hpp',
                'audio_opus_encoder.cpp',
                'audio_opus_decoder.hpp',
                'audio_opus_decoder.cpp',
                'video_decoding_vp.cpp',
                'video_decoding_vp.hpp',
                'video_encoding_vp.cpp',
                'video_encoding_vp.hpp'
            ]

        }
    ],


}