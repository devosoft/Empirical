#! /usr/bin/env python3

import os, sys, argparse
from collections import defaultdict

expecting = ["name", "walltime", "total_mem", "total_replicants", 
             "binaries", "parameters", "unchanging"]

PBS_lines = ["#PBS -N {}",
             "#PBS -l walltime={}",
             "PBS -l mem={}gb",
             "#PBS -l nodes=1:ppn={}"]

def process_config(fname):
    config = dict()
    phase = 0

    parameters = dict()
    unchanging = dict()

    with open(fname, 'r') as f:
        for l in f:
            line = l.strip()
        
            # allow comments
            if len(line) > 0 and line[0] == '#':
                continue

            # load PBS config
            if phase < 4:
                config[expecting[phase]] = line
                phase += 1

            # get binaries
            elif phase == 4:
                config[expecting[phase]] = line.replace(' ', '').split(',')
                phase += 1

            # if phase 5, load parameters
            elif phase == 5:
                if len(line) > 0: 
                    chopped = line.split(' ', 1)
                    parameters[chopped[0]] = chopped[1].split(',')
                else:
                    phase += 1

            elif phase == 6:
                if len(line) > 0: 
                    chopped = line.split(' ', 1)
                    assert ',' not in chopped[1] # sanity check
                    unchanging[chopped[0]] = chopped[1]
                else:
                    phase += 1

    config['num_replicants'] = config['total_replicants']
    config['total_replicants'] = int(int(config['num_replicants']) * len(config['binaries']))
    config['total_mem'] = int(config['total_mem']) * config['total_replicants']

    config['unchanging'] = unchanging
    config['parameters'] = parameters
    config['basepath'] = os.getcwd()

    return config

# base formatting string used to call binary from qsub file
base_string = "{base}/{binary} -PREFIX {binary}-{rep} -RAND {rep} {args}"

def regurgitate(config, args, path):
    with open(path + "makeitgo.qsub", 'w') as qfile:
        # PBS
        for i in range(0, 4):
            print(PBS_lines[i].format(config[expecting[i]]), file=qfile)

        # args have been fully defined by this point--build the string to save time
        argstring = ""
        for arg in args:
            argstring += "\\\n\t\t" + args[arg][0] + " " + args[arg][1]

        for arg in config['unchanging']:
            argstring += "\\\n\t\t" + arg + " " + config['unchanging'][arg]

        for el in config['binaries']:
            for i in range(0, int(config['num_replicants'])):
                print(base_string.format(base=config['basepath'],
                                         binary=el,
                                         rep=i,
                                         args=argstring),
                                 end=" \n\n") # print \ and newline


def generate_folder_level(index, prefixes, currbase, config, fqc = None):
    if fqc == None:
        fqc = dict()

    if index == len(prefixes.keys()) - 1:
        # make the dir
        for el in prefixes[index]:
            fqc[index] = (el[1], el[2])
            if not os.path.exists(currbase + el[0]):
                print("Creating directory {}".format(currbase + el[0]))
                os.makedirs(currbase + '/' + el[0])
                regurgitate(config, fqc, currbase + el[0])
    else:
        # build this level of the tree
        for el in prefixes[index]:
            fqc[index] = (el[1], el[2])
            generate_folder_level(index + 1, prefixes, currbase + el[0], config, fqc)

def generate_folder_tree(base_dir, parameters, config):

    depth = 0
    
    folder_prefixes = defaultdict(list)

    # for each key generate a directory
    for key in parameters:
        level = key.replace('-', '')

        for el in parameters[key]:
            directory = level + '_' + el + '/'
            folder_prefixes[depth].append((directory, key, el))
        depth += 1
   
    print(folder_prefixes)

    generate_folder_level(0, folder_prefixes, base_dir, config)


def get_parser():
    parser = argparse.ArgumentParser(description="Script to build factorial sweeps for HPCC Qsubs")
    parser.add_argument('config_file_name', help="Filename of the argument config")

    return parser

def main():
    cmdln_args = get_parser().parse_args()
    config = process_config(cmdln_args.config_file_name)

    if not os.path.exists(config['name']):
        os.makedirs(config['name'])

    generate_folder_tree(config['basepath'] + '/' + config['name'] + '/', 
                         config['parameters'], 
                         config)


if __name__ == '__main__':
    main()

