#!/bin/bash

# invoked like: ./mod_doc_gen.sh web doc/library/web.rst
# from the root dir of empirical
# (gotta get relative paths right for doxygen to be happy)

dir="$1*.h"
ls $dir > file_list
python doc/gen_module_docfile.py
rm file_list
mv docfile.rst $2
echo "Done!"
