{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'encoding_engine',
            
            'dependencies': [

            ],
            'include_dirs': [
                '../include',
                '../error',
                '../misc',
                '../../libs/libsrtp/include',
                '../../libs/libsrtp/crypto/include',
                '../../libs/libopus/include',
                '../../libs/libvpx',
                '../../libs/libyuv/include'
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

                }],
                ['OS == "linux"', {
                    'type': 'shared_library',
                    'libraries': [
                        '../../../libs/libopus.a',
                    ],
                }],
                ['OS == "ios"', {
                    'type': 'static_library',

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