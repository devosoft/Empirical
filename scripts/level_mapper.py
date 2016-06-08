#!/usr/bin/env python

# This file is part of Empirical, https://github.com/devosoft/Empirical
# Copyright (C) Michigan State University, 2016.
# Released under the MIT Software license; see doc/LICENSE


from __future__ import print_function
import sys
from collections import defaultdict

"""
Simple file to generate a level dependency map for the project.
"""

import os, glob, argparse

class argstruct(object):
    """really stupid object to pass in arguments from an interpreter"""
    pass

class DependencyNode(object):
    """Simple object to build a dependency tree"""

    def __init__(self, path):
        self.name = os.path.basename(path)
        self.path = path
        self.dependencies = defaultdict(DependencyNode)
        self.depth = 0

    def __str__(self):
        return self.name

    def add_dep(self, node):
        # nodes are identified by their normalized path
        self.dependencies[node.path] = node

        if node.depth + 1 > self.depth:
            self.depth = node.depth + 1

        return self.depth

class DependencyTree(object):
    """Simple object to hold a dependency tree."""

    def __init__(self):
        # again, nodes are indexed by normalized path
        self.nodes = defaultdict(DependencyNode)

    def add_node(self, node):
        self.nodes[node.path] = node


def file_to_tree(filepath, tree, args):
    """Takes a file and adds it (and its dependencies) to the tree"""
    with open(filepath, 'r') as currfile:

        thisnode = DependencyNode(os.path.normpath(filepath))
        if args.verbose:
            print("INFO: Created node {}".format(thisnode.name))

        for line in currfile.readlines():
            # grab dependencies
            if line.startswith('#include "'):
                dep = line.replace('#include "', "")
                dep = dep.replace('"', "")
                dep = dep.strip()

                if args.verbose:
                    print("INFO: Stepping down to {}".format(os.path.dirname(currfile.name) + '/' + dep),
                        file=sys.stderr)

                childnode = file_to_tree(os.path.dirname(currfile.name) + '/' + dep, tree, args)
                thisnode.add_dep(childnode)
                if childnode.depth + 1 > thisnode.depth:
                    thisnode.depth = childnode.depth + 1


        tree.add_node(thisnode)
        return thisnode

def build_tree_from_dir(dirpath, args):
    """Takes in a directory path and arguments, spits out a tree."""

    tree = DependencyTree()

    # get initial list of files
    target = dirpath + "*." + args.extension

    if args.verbose:
        print("Initial search path is: {}".format(target), file=sys.stderr)

    tlist = glob.glob(target)

    for sfile in tlist:
        if args.verbose:
            print("INFO: Processing {}".format(sfile), file=sys.stderr)

        file_to_tree(sfile, tree, args)

    return tree

def tree_to_dep_map(tree):
    """converts a dependency tree into a dict: dep_level -> nodes"""
    dep_map = defaultdict(set)
    for path in tree.nodes:
        dep_map[tree.nodes[path].depth].add(tree.nodes[path])

    return dep_map

def print_dep_map_for_dir(dirpath, args):
    """Takes in a dependency map and prints it to stdout"""
    dep_map = tree_to_dep_map(build_tree_from_dir(dirpath, args))

    for key in dep_map:
        if key != 0:
            print("\n\n== Level " + str(key) + " ==")
            for node in dep_map[key]:
                print("\n\t", str(node), "depends:\n\t\t", end="")
                chars = 24 # tabs make for great consistency
                for dep in node.dependencies:
                    chars += len(str(node.dependencies[dep])) + 1
                    if(chars > 80):
                        print("\n\t\t", end="")
                        chars = 16 + len(str(dep))
                        print(str(node.dependencies[dep]) + " ", end=" ")
                    else:
                        print(str(node.dependencies[dep]) + ", ", end="")
        else:
            print("== Level 0 (no dependencies) ==")
            for node in dep_map[key]:
                print("\t{}".format(str(node)))

def get_parser():
    """
    Constructs the parser for when this script is called from the command line
    """

    parser = argparse.ArgumentParser(
        description='Script to build a dependency map for a directory',
        epilog='Example: ./level_mapper.py ../tools/ h')

    parser.add_argument('directory_path', help="Path of the directory to make"
                        " a tree for (ending slash required)")
    parser.add_argument('extension', help="Extension to glob (no dot)",
                        default="h")
    parser.add_argument('-v', '--verbose', help="Turn on debugging info",
                        action='store_true')

    return parser

def main():
    args = get_parser().parse_args()
    print_dep_map_for_dir(args.directory_path, args)

if __name__ == "__main__":
    main()
