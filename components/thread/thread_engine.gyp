{
    'includes': [
        '../../common.gypi'
    ],
    
    "targets": [
        {
            "target_name": "thread_engine",
            
            'include_dirs': [
                '../include',
            ],
            'conditions': [
                ['OS == "mac"', {
                    "type": 'shared_library',
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
                        'INSTALL_PATH': '@executable_path/../Frameworks',
                    },
                    "sources": [
                        "thread.hpp",
                        "thread.cpp",
                        "tsqueue.hpp",
                        "ring_buffer.h",
                        "ring_buffer.cpp",
                        "CAAtomic.h",
                        "audio_ring_buffer.h",
                        "audio_ring_buffer.cpp",
                        'event_handler.hpp',
                        'event_handler.cpp'
                    ]         
                }],
                ['OS == "linux"', {
                    "type": 'shared_library',

                    'sources': [
                        'thread.hpp',
                        'thread.cpp',
                    ]
                }],
                ['OS == "android"', {
                    "type": 'shared_library',

                    'direct_dependent_settings': {
                        'include_dirs': [
                            '../include',
                        ],
                    },

                    'sources': [
                        'thread.hpp',
                        'thread.cpp',
                    ]
                }],
                ['OS == "ios"', {
                    "type": 'static_library',

                    'sources': [
                        'thread.hpp',
                        'thread.cpp',
                        "tsqueue.hpp",
                        "ring_buffer.h",
                        "ring_buffer.cpp",
                        "audio_ring_buffer.h",
                        "audio_ring_buffer.cpp",
                        'video_frame_buffer.h',
                        'video_frame_buffer.cpp',
                        'event_handler.hpp',
                        'event_handler.cpp'
                    ]
                }]
            ]
           
        }
    ]
}