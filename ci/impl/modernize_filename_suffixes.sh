#!/bin/bash

find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.h$/.hpp/'
find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.c$/.cpp/'
find ! -path "./third-party/*" ! -path "./.git/*" -type f | rename 's/\.cc$/.cpp/'
