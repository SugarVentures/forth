{
	'variables': {
		'oppvs_root%': '<(DEPTH)/../'
	},
	'target_defaults': {
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
            }]
        ],

		
	}
}