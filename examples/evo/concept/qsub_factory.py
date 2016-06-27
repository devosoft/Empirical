#! /usr/bin/env python3

import os, sys
from collections import defaultdict

def get_config_from_console():
    config = dict()

    print("Enter job name: ", end="")
    config['name'] = input().strip()

    print("Enter walltime (hh:mm:ss): ", end="")
    config['walltime'] = input().strip()

    print("Enter memory per replicant (GB): ", end="")
    config['mem_per_thread'] = input().strip()

    print("Enter number of replicants: ", end="")
    config['num_replicants'] = input().strip()

    parameters = dict()
    unchanging = dict()
    line = "somestr"
    
    print("Enter parameter name and values to sweep, blank to quit")
    print("(e.g., '-INITAL_CONFIG 1,2,3')")
    print("-PREFIX and -RAND_SEED will be automatically set.\n")
    
    while True: # TIL python doesn't have do-whiles
        print("Parameter / values: ", end="")
        line = input().strip()
        if line == "":
            break;
        parameters[line.split(' ')[0]] = line.split(' ')[1].split(',')

    config['parameters'] = parameters

    print("Please enter the unchanging parameters in the same way")
    
    while True: # TIL python doesn't have do-whiles
        print("Parameter / value: ", end="")
        line = input().strip()
        if line == "":
            break;
        unchanging[line.split(' ')[0]] = line.split(' ')[1]

    config['unchanging'] = unchanging
    config['basepath'] = os.getcwd()

    return config


def build_qsub(config, args, path):
    with open(path + "makeitgo.qsub", 'w') as qfile:
        print("#!/bin/bash -login", file=qfile)
        print("#PBS -l walltime={}".format(config['walltime']), file=qfile)
        print("#PBS -l nodes=1:ppn=2", file=qfile)
        print("#PBS -l mem={}gb".format(config['mem_per_thread']), file=qfile)
        print("#PBS -N {}".format(config['name']), file=qfile)
        if int(config['num_replicants']) > 0:
            print("#PBS -t 0-{}".format(config['num_replicants']), file=qfile)

        print("", file=qfile)
        print("{binpath}/gladiator-mixed.out -PREFIX mixed-${{PBS_ARRAYID}} -RAND_SEED"
              " ${{PBS_ARRAYID}} ".format(binpath=config['basepath']), end="", file=qfile)
        for el in args:
            print("{} {} ".format(args[el][0], args[el][1]), end="", file=qfile)
        for el in config['unchanging']:
            print("{} {} ".format(el, config['unchanging'][el]), end="", file=qfile)

        print("\n\n", file=qfile)

        print("{binpath}/gladiator-standard.out -PREFIX standard-${{PBS_ARRAYID}} -RAND_SEED"
              " ${{PBS_ARRAYID}} ".format(binpath=config['basepath']), end="", file=qfile)
        for el in args:
            print("{} {} ".format(args[el][0], args[el][1]), end="", file=qfile)

        for el in config['unchanging']:
            print("{} {} ".format(el, config['unchanging'][el]), end="", file=qfile)

        print("""\n\n
        for job in `jobs -p`
        do
        wait $job
        done
        """, file=qfile)


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
                build_qsub(config, fqc, currbase + el[0])
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

def main():
    config = get_config_from_console()

    if not os.path.exists(config['name']):
        os.makedirs(config['name'])

    generate_folder_tree(config['basepath'] + '/' + config['name'] + '/', 
                         config['parameters'], 
                         config)


if __name__ == '__main__':
    main()

