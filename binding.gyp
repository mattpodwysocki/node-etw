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
    }
  ],
}
