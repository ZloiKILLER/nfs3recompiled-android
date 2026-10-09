"""Where tools/build_libs.py puts the decompiled code as shared libraries."""
import os
import sys

EXT = '.dll' if sys.platform == 'win32' else ('.dylib' if sys.platform == 'darwin' else '.so')
BUILD = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build'))


def lib(name):
    return os.path.join(BUILD, name + EXT)
