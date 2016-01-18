{
    'includes': [
        '../../common.gypi'
    ],

    'targets': [
        {
            'target_name': 'misc',
            'type': 'shared_library',
             'dependencies': [
                'libmisc1', 'libmisc2'
            ],
        },
        {
            "target_name": "libmisc1",
            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                }],

                ['OS == "ios"', {
                    'type': 'static_library',
                }],
            ],
            'include_dirs': [
                '<(oppvs_3rdlibs_path)/include',
                '../include'
            ],
            
            "sources": [
                "md5.h",
                "md5.c",
                "random_unique32.h",
                "random_unique32.c",
                "utility.h",
                "utility.c"
            ]

        },
        {
            'target_name': 'libmisc2',
            'conditions': [
                ['OS == "mac"', {
                    'type': 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                }],

                ['OS == "ios"', {
                    'type': 'static_library'
                }],
            ],
            
            'include_dirs': [
                '../include'
            ],
            'sources': [
                'atomic_helper.hpp',
                'crc.hpp',
                'fasthash.h',
                'atomic_helper.cpp',  
                'fasthash.cpp'
            ],
            
        }
    ]    
}