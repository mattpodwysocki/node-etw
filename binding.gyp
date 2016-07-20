{
  'targets': [
    {
      'target_name': 'etwtrace',
      'sources': [
        'src/etwtrace.cpp',
        'src/tracesession.cpp',
        'src/stdafx.cpp'
      ],
      "include_dirs" : [ 
        "<!(node -e \"require('nan')\")",
      ],
      'msvs_precompiled_header': 'src/stdafx.h',
      'msvs_precompiled_source': 'src/stdafx.cpp',
      'libraries': [ 
        'tdh.lib',
      ],
      'defines': [
        'UNICODE',
      ],
    },
  ],
}
