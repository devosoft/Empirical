#!/bin/bash

ls "$1/*" > file_list
python doc/gen_module_docfile.py
#rm file_list

mv docfile.rst $2
