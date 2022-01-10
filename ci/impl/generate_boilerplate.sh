#!/bin/bash
set -e

./ci/impl/generate_boilerplate_file_docstrings.sh
./ci/impl/generate_boilerplate_headerguards.sh
