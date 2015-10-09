{
	'variables': {
		'oppvs_root%': '<(DEPTH)/../'
	},
	'target_defaults': {
        'default_configuration': 'Debug',
		'conditions': [
            ['OS == "linux"', {
            	'cflags': [
            		'-fPIC'
            	],
    			'cflags_cc': [
            		'-fPIC',
            		'-std=c++11'
    			],
    			'ldflags': [
    				'-pthread',
    			]
            }],
            ['OS == "mac"', {
                'configurations': {
                    'Debug': {

                    },
                    'Release': {

                    }
                }
                
            }]
        ],

		
	}
}