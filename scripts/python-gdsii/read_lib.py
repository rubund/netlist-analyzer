#!/usr/bin/env python3

import gdsii
from gdsii.library import Library
from gdsii.elements import *

with open('PATH_TO_GDS', 'rb') as stream:
    lib = Library.load(stream)

print(lib)


print(lib[0][40])
