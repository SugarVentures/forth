{
    'targets': [
    {
        'target_name': 'threadengine-jni',
        'type':'shared_library',
        
        'include_dir':[
            './jni',
            '<(DEPTH)'
        ],
        
        
        'direct_dependent_settings':{
            'include_dirs':[
                
            ],
        },
        
        'export_dependent_settings':[
        ],
        
        'dependencies':[
            '<(DEPTH)/thread_engine.gyp:*',
        ],
        
        'sources':[
            './jni/threadengine-jni.cpp',
        ],
        
        
        'link_settings': {
            'ldflags': [
               '-llog',
            ]
        },

    }
    
    ],

}