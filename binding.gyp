{
  'targets': [
    {
      'target_name': 'etwtrace',
      'sources': [
        'src/etwtrace.cpp',
      ],
      'include_dirs': [
        '<!(node -e "require(\'nan\')")'
      ],
      'conditions': [
        ['OS=="win"',
          {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'ExceptionHandling': '2',
                'DisableSpecificWarnings': [ '4530', '4506' ],
              },
            },
          },
        ],
      ],
    }
  ],
}
