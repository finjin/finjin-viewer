#Copyright (c) 2017 Finjin
#
#This file is part of Finjin Viewer (finjin-viewer).
#
#Finjin Viewer is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#This Source Code Form is subject to the terms of the Mozilla Public
#License, v. 2.0. If a copy of the MPL was not distributed with this
#file, You can obtain one at http://mozilla.org/MPL/2.0/.


import sys

if __name__== '__main__':
    arg_count = len(sys.argv)
    if arg_count < 2:
        print('Usage: python fix_macros.py /path/to/file/to/fix')
        print('Example: python fix_macros.py output/ForwardShaders.vert')
        sys.exit(1)

    file_path = sys.argv[1]
    with open(file_path, 'r') as in_file:
        content = in_file.read().strip()
        content = content.replace('@version', '#version')
        content = content.replace('@extension', '#extension')

    with open(file_path, 'w') as out_file:
        out_file.write(content)
