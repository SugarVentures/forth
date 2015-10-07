{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'encoding_engine',
            'type': 'shared_library',
            'dependencies': [

            ],
            'include_dirs': [
                '../include',
                '../../libs/libopus/include',
            ],

            'conditions': [
                ['OS == "mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                    },
                    'libraries': [
                        '../../libs/libopus.a',
                    ],

                }],
                ['OS == "linux"', {
                    'libraries': [
                        '../../../libs/libopus.a',
                    ],
                }]
            ],

            'sources': [
                'audio_opus_encoder.hpp',
                'audio_opus_encoder.cpp',
                'audio_opus_decoder.hpp',
                'audio_opus_decoder.cpp'
            ]

        }
    ],


}