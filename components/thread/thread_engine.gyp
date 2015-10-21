{
    'includes': [
        '../../common.gypi'
    ],
    
    "targets": [
        {
            "target_name": "thread_engine",
            "type": 'shared_library',
            'include_dirs': [
                '../include',
            ],
            'conditions': [
                ['OS == "mac"', {
                    "sources": [
                        "thread.hpp",
                        "thread.cpp",
                        "tsqueue.hpp",
                        "ring_buffer.h",
                        "ring_buffer.cpp",
                        "CAAtomic.h",
                        "audio_ring_buffer.h",
                        "audio_ring_buffer.cpp"
                    ]         
                }],
                ['OS == "linux"', {
                    'sources': [
                        'thread.hpp',
                        'thread.cpp',
                    ]
                }]
            ]
           
        }
    ]
}