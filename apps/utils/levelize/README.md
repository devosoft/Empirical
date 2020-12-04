# Levelization tool for software packages

This utility takes in a series of filenames and then tracks which of those files
include each other to build a levelization map.

The specific algorithm will load each file, search for lines with a #include, and
then grabs the final filename on those lines (after removing any comments).
