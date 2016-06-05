#!/usr/bin/env python3

"""
Since gcov is a box of rocks when it comes to identifying uncovered lines of
code I am writing this script to resolve the problem.

This script will, in conjunction with a makefile:

#. Execute the tests
#. Run gcov to generate .gcov notes files
#. Process the git diff
#. Process the source file for each notes file to eliminate preprocessor crap
#. Determine if the git diff contains lines uncovered by tests
#. Reprimand the user
"""

import sys, argparse, subprocess, os
from collections import defaultdict
from level_mapper import *


if sys.version_info[0] != 3:
    print("This script requires Python 3")
    exit()


def print_list(lump, indent=0):
    """simple function to print a list line-by-line to stderr"""
    for line in lump:
        print("\t" * indent + line, file=sys.stderr)


def print_dict(lump):
    for key in lump:
        print("{}:".format(key))
        for element in lump[key]:
            print("\t".format(element))


def get_source_without_comments(source_fname):
    """
    Use g++ to strip the comments out of a source file
    """
    gpp_args = ["g++", "-std=c++11", "-dD", "-E", "-fpreprocessed"]
    gpp_args.append(source_fname.rstrip())
    print("INFO: Running g++: {}".format(gpp_args))

    proc = subprocess.Popen(gpp_args, stdout=subprocess.PIPE)
    proc_lines = []
    for line in proc.stdout:
        proc_lines.append(line.decode('ascii').strip("\n\r"))
    proc.wait()
    proc_lines = proc_lines[1:]

    if proc.returncode != 0:
        print("** Error during compilation; dumping output.", file=sys.stderr)
        print(proc_lines, file=sys.stderr)
        sys.exit(1)

    return proc_lines


def purge_unreachable_lines(lines):
    """
    function to eliminate non-code/unreachable lines
    """

    relevant = [True] * len(lines)
    index = 0

    ifdef_stack = 0

    # determine if this line is part of an ifdef block
    # (very crudely, mind)
    for line in lines:
        if str.startswith(line, "#ifdef"):
            ifdef_stack += 1
            relevant[index] = False
        elif ifdef_stack > 0:
            relevant[index] = False;
        elif str.startswith(line, "#"):
            relevant[index] = False

        if str.startswith(line, "#endif"):
            ifdef_stack -= 1

        index += 1

    # horribly memory inefficient. Meh.
    result = []

    for i in range(len(lines)):
        if relevant[i]:
            result.append(lines[i])

    return result


def parse_gcov(ifile_name):
    """opens and parses gcov file to get uncovered lines; returns list"""
    lines = []
    try:
        with open("../tests/" + ifile_name + ".gcov", "r") as ifile:
            flines = ifile.readlines()

            for line in flines:
                if line.strip().startswith("-:") or line.strip().startswith("#####:"):
                    if len(line.split(":", 2)[2].replace("}", "").strip()) > 0:
                        lines.append(line.strip())
    except FileNotFoundError as err:
        print("\t** Warning: Could not open coverage file for {}".format(ifile_name),
              file=sys.stderr)

    return lines


def parse_gdiff(gdiff_filename):
    """takes path to git diff file against master, parses into a dict of chunks
    indexed by filename w/ payload a list of added lines in that file"""

    parsed_diff = dict()
    last_item = None
    with open(gdiff_filename, 'r') as gdfile:
        for line in gdfile.readlines():
            if(line.startswith("diff --git")):
                last_item = line.split("b/")[-1].strip()
                #print(last_item)
                parsed_diff[last_item] = list()
            elif(line.startswith('+')):
                parsed_diff[last_item].append(line.strip().split('+')[1])

    # remove the first element from each blob
    for key in parsed_diff:
        parsed_diff[key] = parsed_diff[key][1:]

    return parsed_diff


def check_diff_coverage(source_path, diff_lines):
    sfile_lines = []
    bad_lines = []
    cov_lines = parse_gcov(os.path.basename(source_path))
    # get the lines from the source file
    #print("\tGetting relevant source lines for {}".format(source_path), file=sys.stderr)
    #TODO: add "where is root dir" arg so that "../" can be accurately set
    sfile_lines = purge_unreachable_lines(get_source_without_comments("../" + source_path))

    for line in sfile_lines:
        if line in diff_lines:
            for cov_line in cov_lines:
                broken = cov_line.split(":", 2)

                if line == broken[2]:
                    # line is changed and uncovered, boomtime
                    bad_lines.append(cov_line)
                    continue
    return bad_lines


def diff_cover(args):
    # using the dependency mapper, grab the relevant files
    pseudoargs = argstruct()
    pseudoargs.verbose = False
    pseudoargs.extension = 'cc'
    tree = build_tree_from_dir("../tests/", pseudoargs)

    included_files = []
    flagged_files = []

    concern  = []
    uncovered = []
    found_uncovered = False

    for element in tree.nodes:
        # sliced to cut off the leading "../" from all the filenames
        included_files.append(tree.nodes[element].path[3:])

    parsed_diff = parse_gdiff(args.diff_filename)

    # get files from diff covered by tests
    concern = [x for x in parsed_diff if x in included_files]

    # if we're told to ignore files, do so
    if args.ignorelist:
        for name in args.ignorelist.readlines():
            print("Warning: removing file {} from concern".format(name.strip()), file=sys.stdout)
            try:
                included_files.remove(name.strip())
                parsed_diff.pop(name.strip(), None)
            except:
                pass

    uncovered = [x for x in parsed_diff if (x.endswith("cc") or x.endswith("h")) and x not in included_files]

    print("Files to check coverage: ", file=sys.stderr)
    print_list(included_files, 1)

    # complain about totally uncovered files
    if len(uncovered) > 0:
        print("\nFiles that aren't covered at all, but are changed: ", file=sys.stderr)
        print_list(uncovered, 1)
        found_uncovered = True

    # iterate over files we're concerned about and check their coverage
    for el in concern:
        if el in parsed_diff:
            print("\n\nChecking coverage of {}".format(el), file=sys.stderr)

            #uncovered_lines = parse_gcov(os.path.basename(el))
            bad_lines = check_diff_coverage(el, parsed_diff[el])

            if len(bad_lines) > 0:
                print("\tBad lines for {}".format(el), end="\n\n")
                print_list(bad_lines, 2)
                found_uncovered = True

    print("\nDone checking coverage.", file=sys.stderr)

    if found_uncovered:
        print("** Found uncovered lines, failing!")
        sys.exit(1)

class CoverageFile(object):
    def __init__(self, path):
        self.name = os.path.basename(path)
        self.path = path
        self.source_lines = open(path, 'r').readlines()
        self.executable_lines = purge_unreachable_lines(get_source_without_comments(path))


class CoverageModule(object):
    def __init__(self, modpath):
        self.file_list = dict()
        self.total_lines = 0
        self.total_covered = 0
        self.module_directory = modpath

    def __str__(self):
        res = "CoverageModule({})".format(self.module_directory) + ": ["
        for element in self.file_list:
            res += element + ", "

        return res + "] ({} / {} lines covered)".format(self.total_covered, self.total_lines)

    def add_file_by_path(self, fpath):
        rel_fpath = fpath.split(self.module_directory)[1]
        self.file_list[rel_fpath] = CoverageFile(fpath)
        self.total_lines += len(self.file_list[rel_fpath].executable_lines)
        print("Added file {} (now at {} lines)".format(rel_fpath, self.total_lines), file=sys.stderr)


def survey_coverage(args):
    """Function to scan specified directories and compile module test coverage"""

    # lots of duplicated code below
    #TODO: abstract out this code so it isn't repeated

    survey_dirlist = [fname.strip() for fname in args.dirlist_filename.readlines()]
    print("Directories to survey: ", file=sys.stderr)
    print_list(survey_dirlist, 1)

    # using the dependency mapper, grab the relevant files
    pseudoargs = argstruct()
    pseudoargs.verbose = False
    pseudoargs.extension = 'cc'
    tree = build_tree_from_dir("../tests/", pseudoargs)

    included_files = []
    flagged_files = []
    modules = []
    concern  = []
    uncovered = []
    ignore_list = None
    if args.ignorelist:
        ignore_list = []
        for line in args.ignorelist.readlines():
            ignore_list.append(line.strip())
    found_uncovered = False

    for element in tree.nodes:
        # sliced to cut off the leading "../" from all the filenames
        included_files.append(tree.nodes[element].path[3:])

    # if we're told to ignore files, do so
    """
    if args.ignorelist:
        for name in args.ignorelist.readlines():
            print("Warning: removing file {} from concern".format(name.strip()), file=sys.stdout)
            try:
                included_files.remove(name.strip())
                parsed_diff.pop(name.strip(), None)
            except:
                pass
    """
    # we now have a list of files that are covered by tests
    # now, using the module path, we need to shove filenames into the relevant files


    # for each listed directory, build a module
    for modpath in survey_dirlist:
        mod = CoverageModule(modpath)

        #TODO: find a way to pass in a list of extensions as args && use those
        for mod_file in glob.glob(modpath + "*" + ".h"):
            if ignore_list and not any([ig in mod_file for ig in ignore_list]):
                mod.add_file_by_path(mod_file)
            else:
                print("** Warning: Ignoring {}".format(mod_file))
        print(mod, file=sys.stderr)

def get_parser():
    """Builds the parser for the script"""

    parser = argparse.ArgumentParser(description="Script to act as a bad diff-cover agent for"
                                     " broken gcov")

    # config for subcommands
    subparsers = parser.add_subparsers(help="Subcommand help")

    # diff-cover subcommand args
    diff_cover_parser = subparsers.add_parser("diff-cover",
                                              help="check git diff for uncovered lines")
    diff_cover_parser.add_argument('-f', '--filename', help="Name of file to include in processing.")
    diff_cover_parser.add_argument('-p', '--path', default="./",
                        help="path of the directory to examine (default = '.')")
    # required args
    diff_cover_parser.add_argument("diff_filename", help="path to the file containing the git diff")
    diff_cover_parser.add_argument("-i", "--ignorelist", type=argparse.FileType('r'),
                        help="path to file containing list of filenames to "
                        "ignore during coverage check.")
    diff_cover_parser.set_defaults(func=diff_cover)

    survey_cov_parser = subparsers.add_parser("survey-cov", help="Survey the coverage "
                                              "of the listed directories and generate"
                                              " a report.")
    survey_cov_parser.add_argument("dirlist_filename", help="Filename containing list of "
                                   "directories to survey", type=argparse.FileType('r'))
    survey_cov_parser.add_argument("-i", "--ignorelist", type=argparse.FileType('r'),
                        help="path to file containing list of filenames to "
                        "ignore during coverage check.")
    survey_cov_parser.set_defaults(func=survey_coverage)

    return parser


def main():

    if len(sys.argv) < 2:
        args = get_parser().parse_args(['--help'])

    args = get_parser().parse_args()
    # thanks to khmer's oxli for this (https://github.com/dib-lab/khmer/blob/b43907695b86f593487d06a38d702ee70988a6f9/oxli/__init__.py)
    args.func(args)


if __name__ == "__main__":
    main()
