#! /usr/bin/python

# Generate list of files to index with cscope and then generate cscope index.

# Should be run from root of m5 tree (i.e. as 'util/cscope-index.py').

import os

# absolute paths to skip
skipdirs = [ 'src/unittest', 'src/doxygen' ]

# suffixes of files to index
suffixes = [ '.cc', '.hh', '.c', '.h' ]

def oksuffix(f):
    for s in suffixes:
        if f.endswith(s):
            return True
    return False

file_list = file('cscope.files', 'w')

for dirpath,subdirs,files in os.walk('src'):
    # filter out undesirable subdirectories
    for i,dir in enumerate(subdirs):
        if dir == 'SCCS':
            del subdirs[i]
            break

    # filter out undesirable absolute paths
    if dirpath in skipdirs:
        del subdirs[:]
        continue

    # find C/C++ sources
    okfiles = [f for f in files if oksuffix(f)]
    if okfiles:
        print >> file_list, \
              '\n'.join([os.path.join(dirpath, f) for f in okfiles])

file_list.close()

# run cscope to generate index
os.system("cscope -b")
