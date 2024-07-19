def Settings( **kwargs ):
    return {
        'flags': [
            '-x', 'c',
            '-I', '/usr/local/include',
            '-I', '/opt/homebrew/opt/libgit2/include',
            '-I', 'include'
        ],
    }

