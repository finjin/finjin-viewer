#!/usr/bin/env python3

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


#------------------------------------------------------------------------------
#Before running this script, modify configure.cfg
#Then run this script without arguments

import os
import os.path
import platform
from collections import OrderedDict

def to_utf8(text):
    return text.encode('utf-8')

def from_utf8(byte_array):
    return str(byte_array.decode('utf-8', 'ignore'))

def read_binary_file(filename):
    with open(filename, 'rb') as f:
        return f.read()

def read_text_file(filename):
    return from_utf8(read_binary_file(filename))

def normalize_path(p):
    return os.path.normpath(p).replace('/', os.path.sep)

if __name__ == '__main__':
    this_file_path = normalize_path(__file__)
    config_file_path = this_file_path.replace('.py', '.cfg')
    iterate_directory = normalize_path(os.getcwd())
    log_progress = True

    #Determine platform name
    platform_name = platform.system().lower() #windows, macos, linux
    if 'mac' in platform_name or 'darwin' in platform_name:
        platform_name = 'macos'

    #Read the configuration file and get the path to find
    lookup = OrderedDict()
    lookup_by_section = OrderedDict()
    section = None
    config_file_content = read_text_file(config_file_path)
    lines = [line.rstrip() for line in config_file_content.split('\n')]
    for line in lines:
        if len(line) > 0 and line[0] != '#':
            if line.startswith('[') and line.endswith(']'):
                section = line[1:-1]
            else:
                platform, platform_value = line.split('=')
                if section is not None:
                    #Split source/destination by '->'
                    source_to_destination_values = platform_value.split('->')

                    #Possibly override destination with environment variable defined by section
                    source_to_destination_values[1] = os.getenv(section, source_to_destination_values[1])

                    temp_lookup = lookup_by_section.get(section, OrderedDict())
                    temp_lookup[platform] = source_to_destination_values
                    lookup_by_section[section] = temp_lookup
                else:
                    lookup[platform] = platform_value

    find_path = lookup.get(platform_name, None)

    #Search in files and replace
    replace_with_path = normalize_path(os.getcwd())

    if log_progress:
        print('Configuring source code. Please wait...\n')

    updated_count = 0
    for root, subdirs, files in os.walk(iterate_directory):
        for filename in files:
            file_path = normalize_path(os.path.join(root, filename))
            if file_path != this_file_path and file_path != config_file_path:
                content_changed = False

                content = read_text_file(file_path)
                if find_path is not None and find_path != replace_with_path and find_path in content:
                    content = content.replace(find_path, replace_with_path)
                    content_changed = True

                for section in lookup_by_section:
                    section_platform_value = lookup_by_section[section].get(platform_name, None)
                    if section_platform_value is not None and section_platform_value[0] != section_platform_value[1] and section_platform_value[0] in content:
                        content = content.replace(section_platform_value[0], section_platform_value[1])
                        content_changed = True

                if content_changed:
                    if log_progress:
                        print('Updating file: ' + file_path)

                    updated_count += 1

                    with open(file_path, 'wb') as content_file:
                        content_file.write(to_utf8(content))

    #Rewrite configuration file
    if updated_count > 0:
        #Update lookup values
        lookup[platform_name] = replace_with_path

        for section in lookup_by_section:
            section_platform_value = lookup_by_section[section].get(platform_name, None)
            if section_platform_value is not None:
                section_platform_value[0] = section_platform_value[1]

        #Write updated lookup values to file
        with open(config_file_path, 'wb') as config_file:
            if len(lookup) > 0:
                config_file.write(to_utf8('#Do not modify these------------------------------\n'))
                config_file.write(to_utf8('#Appropriate values will be used automatically when running configure.py\n'))
                for k in lookup:
                    config_file.write(to_utf8(k + '=' + lookup[k] + '\n'))
                config_file.write(to_utf8('\n'))

            if len(lookup_by_section) > 0:
                config_file.write(to_utf8('#Modify these------------------------------\n'))
                config_file.write(to_utf8("#You only need to modify the targets (to the right of the '->') that correspond to the current operating system (linux, macos, or windows)\n"))
                config_file.write(to_utf8("#Alternatively, you can set up an environment variable with each section name, and that will be used instead of the value to the right of '->'\n"))
                for section in lookup_by_section:
                    config_file.write(to_utf8('[' + section + ']\n'))
                    for section_platform_key in lookup_by_section[section]:
                        section_platform_value = lookup_by_section[section][section_platform_key]
                        config_file.write(to_utf8(section_platform_key + '=' + section_platform_value[0] + '->' + section_platform_value[1] + '\n'))
                    config_file.write(to_utf8('\n'))

    #Finish
    if log_progress:
        if updated_count > 0:
            print('Done!')
        else:
            print('No changes were necessary.')
