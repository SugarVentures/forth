{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'encoding_engine',
            
            'dependencies': [

            ],
            
            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                    'libraries': [
                        '../../libs/libopus.a',
                    ],
                    'include_dirs': [
                        '../include',
                        '../error',
                        '../misc',
                        '../../libs/libsrtp/include',
                        '../../libs/libsrtp/crypto/include',
                        '../../libs/libopus/include',
                        '../../libs/libyuv/include'
                    ],

                }],
                ['OS == "linux"', {
                    'type': 'shared_library',
                    'libraries': [
                        '../../../libs/libopus.a',
                    ],
                }],
                ['OS == "ios"', {
                    'type': 'static_library',
                    'include_dirs': [
                        '../include',
                        '../error',
                        '../misc',
                        '../../libs/libsrtp/include',
                        '../../libs/libsrtp/crypto/include',
                        '../../libs/libopus/include',
                        '../../libs/libyuv/include',
                        '<(oppvs_3rdlibs_path)/include'
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