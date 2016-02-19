{
    'targets': [
    {
        'target_name': 'forth-jni',
        'type':'shared_library',
        
        'include_dir':[
            './jni',
            '<(DEPTH)',
        ],
        
        
        'direct_dependent_settings':{
        
        },
        
        'export_dependent_settings':[
        ],
        
        'dependencies':[
            '<(DEPTH)/thread/thread_engine.gyp:*',
            '<(DEPTH)/misc/misc.gyp:*',
            '<(DEPTH)/network/network_engine.gyp:*'
        ],
        
        'sources':[
            './jni/forth-jni.cpp',
        ],
        
        
        'link_settings': {
            'ldflags': [
               '-llog',
            ]
        },

    }
    
    ],

}