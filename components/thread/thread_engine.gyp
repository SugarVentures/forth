{
    'includes': [
        '../../common.gypi'
    ],
    
    "targets": [
        {
            "target_name": "thread_engine",
            "type": 'static_library',
            'include_dirs': [
                '../include',
            ],
            "sources": [
                "thread.hpp",
                "thread.cpp",
                "tsqueue.hpp",
                "ring_buffer.h",
                "ring_buffer.cpp"
            ]

        }
    ]
}