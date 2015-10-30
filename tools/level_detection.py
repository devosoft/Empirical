#!/usr/bin/env python
# This file is part of the Empirical project

from __future__ import print_function
import sys
from collections import defaultdict
"""
Simple file to generate a level dependency map for the project.
"""


import os, glob, argparse

class DependencyNode(object):
    """Dependency tree node"""

    def __init__(self, name):
        self.name = name
        self.dependencies = []
        self.dependers = []
        self.dependency_free = (len(self.dependencies) == 0)
    
    def __str__(self):
        return self.name
    
    def add_dependency(self, dependency): 
        """
        Handles the accounting of adding a new dependency to a node
        """
        if dependency is DependencyNode:
            self.dependencies.append(dependency)
        else:
            self.dependencies.append(str(dependency)) # pedantic, I know

    def add_depender(self, depender):
        """
        Handles the addition of a thing that depends on this node to the node
        """

        # this should never happen
        if type(depender) is not DependencyNode:
            raise Exception("Depender is not a DependencyNode!")

        self.dependers.append(depender)

class DependencyTree(object):
    """
    DependencyTree class, holds utility methods for managing
    DependencyNodes
    """

    def __init__(self):
        """
        The dependency tree is constructed without an initial node
        """
        # gonna keep a list of all the nodes in the tree for accounting
        self.nodes = []

    def add_node(self, node):
        self.nodes.append(node)
        if type(node) is not str:
            self.update_name_to_node(node.name, node)

    def update_name_to_node(self, name, newnode):
        """
        Scans the tree for a list of nodes that depend on a specific name (not
        a specific node)

        This is used to update the relevant nodes with the new node of that
        name
        """
        
        for node in self.nodes:
            updated = False
            for dependency in node.dependencies:
                if type(dependency) is not DependencyNode:
                    if dependency in name:
                        node.dependencies.remove(dependency)
                        node.dependencies.append(newnode)
                        newnode.dependers.append(node)

    def scan_for_named_node(self, name):
        """
        Scan known nodes for a name, return the node if found, none if not
        """
        for node in self.nodes:
            if type(node) is not str and name in node.name:
                return node
        return None

def scan_directory(dirpath):
    """
    Takes in a directory path to scan, then builds a dependency tree from the
    files there
    """
    
    tree = DependencyTree()

    # find some .h files!
    for hfile in glob.glob(dirpath + "*.h"):
        node = DependencyNode(hfile)

    # open the file, get first 25 lines (arbitrary number, feel free to change)
        with open(hfile, "r") as currfile:
            try:
                head = [next(currfile) for x in xrange(25)]
            except StopIteration:
                head = currfile.readlines()
            for line in head:
                if line.startswith('#include "'):
                    dep = line.replace('#include "', "")
                    dep = dep.replace('"', "")
                    dep = dep.strip()
                    
                    if dep is None:
                        raise Exception(line)

                    search = tree.scan_for_named_node(dep)
                    if search:
                        search.add_depender(node)
                        node.add_dependency(search)
                    else:
                        # For whatever reason the dependency we found isn't
                        # listed in the tree yet, so we'll toss in a
                        # placeholder string
                        node.add_dependency(dep)
        
        tree.add_node(node)
    
    return tree

def get_parser():
    """
    Constructs the parser for when this script is called from the command line
    """
    parser = argparse.ArgumentParser(
        description='Script to build a dependency map for a directory')

    parser.add_argument('directory_path', help="Path of the directory to make"
                        " a tree for")
    parser.add_argument('-o', '--output', help="Filename to write to",
                        type=argparse.FileType('w'), default=sys.stdout)

    return parser

def main():
    args = get_parser().parse_args()
    tree = scan_directory(args.directory_path)

    level_map = defaultdict(list)

    for node in tree.nodes:
        num_dep = len(node.dependencies)
        level_map[num_dep].append(node)

    for key in level_map:
        if key != 0:
            print("\n\n== Level " + str(key) + " ==", file=args.output)
            for node in level_map[key]:
                print("\n", str(node), "depends:\n\t", end="", file=args.output)
                chars = 24 # tabs make for great consistency
                for dep in node.dependencies:
                    chars += len(str(dep)) + 1
                    if(chars > 80):
                        print("\n\t\t\t\t", end="", file=args.output)
                        chars = 16 + len(str(dep))
                        print(str(dep) + " ", end=" ", file=args.output)
                    else:
                        print(str(dep) + ", ", end="", file=args.output)
        else:
            print("== Level 0 (no dependencies) ==", file=args.output)
            for node in level_map[key]:
                print(str(node), file=args.output)
    
    args.output.close()

if __name__ == '__main__':
    main()
