#!/usr/bin/env python3

from PIL import Image
import argparse

"""
Simple module to take a CSV file of grid states and generate images for them, to later be stitched
together into a video.

Useable both as a script and as a module. See docstring for 'main' for deets.
"""

def parse_map_file(fname):
    """
    This function expects a CSV in the following format, with a header line:

    update, state
    0, { [ col0row0, col1row0, col2row0], [col0row1, col1row1, col2row1], [col0row2, col1row2, col2row2 ] }
    10, { [ col0row0, col1row0, col2row0], [col0row1, col1row1, col2row1], [col0row2, col1row2, col2row2 ] }
    ...

    Thus each line contains an entire 'state' of the grid for the given update.
    """
    grid_timeline = []

    needed_mappings = set()

    num_lines = 0

    with open(fname, 'r') as mapfile:
        for line in mapfile:

            if num_lines == 0:
                num_lines += 1
                continue
            else:
                chopped = line.strip().split(",", 1)[1]
                chopped = chopped.replace("}", "").split("[")

                current_state = []
                for element in chopped:
                    gridline = []
                    for subel in element.split(","):
                        filtered = subel.replace("]", "").replace("{", "").strip()
                        for org in filtered.split(' '):
                            if len(org) > 0:
                                gridline.append(int(org))
                                needed_mappings.add(int(org))
                    
                    if len(gridline) > 0:
                        current_state.append(gridline)
                
                grid_timeline.append(current_state)

    return grid_timeline, needed_mappings


def fill_square(px, x, y, pixwidth, coloring):
    """
    Utility function to fill a square within the image with a specific color

    probably ridiculously inefficient.
    """
    for i in range(x * pixwidth, x*pixwidth + pixwidth):
        for j in range(y * pixwidth, y*pixwidth + pixwidth):
            px[i,j] = coloring


def gen_image(grid, mapping, pixwidth = 10):
    """
    Given a grid timeline (e.g. a series of grid states; grid[0] = int[][] at tick 0, grid[1] =
    int[][] at tick 1, etc) and color mapping, generate an image for that grid state

    Color mapping is a dict of ints to tuples of RGB values: {1: (0, 1, 2), 2: (3, 4, 5)}
    The ints are the symbols in the grid indicating what organism occupies an individual cell.
    The symbols (Currently) must be ints, but that can be changed relatively easily.
    """
    img = Image.new('RGB', (len(grid) * pixwidth, len(grid[0]) * pixwidth), "black")

    pixels = img.load()

    for i in range(0, len(grid)):
        for j in range(0, len(grid[i])):
            fill_square(pixels, i, j, pixwidth, mapping[grid[i][j]])

    return img


def file_to_frames(gridfname, prefix, mapping, pixwidth, grid = None):
    """
    Esentially a wrapper for a loop--take a given CSV file and make the frames for all the grid
    states listed in the file. See gen_image's docstring for deets on mapping and grid format.
    """

    if grid == None:
        grid, _ = parse_map_file(gridfname)

    framenum = 0
    for snapshot in grid:
        img = gen_image(snapshot, mapping, pixwidth)
        img.save(prefix + "%05d.png" % framenum)
        framenum += 1
       

def load_config_from_file(fname):
    """
    Things we need: 
        name of the map.csv file
        width of each cell (pix)
        symbol -> {rgb} mappings

    ways we'll do it:
        a stupid simple flat text file with a bunch of lines
    """
    with open(fname, 'r') as cfile:
        conflines = cfile.readlines()
        
        config = {}
        config['mapping'] = {}
        readlines = 0

        for line in conflines:
            if line.strip().startswith('#'):
                continue
            else:
                if readlines == 0:
                    config['mapfile_name'] = line.strip()
                elif readlines == 1:
                    config['cell_width'] = int(line.strip())
                elif readlines >= 2:
                    chopped = line.strip().split()
                    if len(chopped) == 0:
                        continue
                    try:
                        config['mapping'][int(chopped[0])] = (int(chopped[1]), int(chopped[2]),
                            int(chopped[3]))
                    except IndexError as err:
                        print("Bad index--chopped is '{}', from line {}".format(chopped, line))
            readlines += 1

        return config


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--auto-color", "-a",
                        help="The script will automatically determine colors for the needed "
                        "values", action="store_true")

    return parser.parse_args()


def tuple_tween(begin, end, ratio):
    result = []
    for index, element in enumerate(begin):
        result.append(begin[index] + int(ratio * (end[index] - element)))

    return result

def main():
    """
    Driver function for when this is called on the command line.

    Note: This is very easily used within the python interpreter:

        `
        from vizualizer import *
        mapping = {mapping creation here, useful for large maps (e.g. >10 kinds of orgs)}
        file_to_frames('location_of_map.csv', 'frame', mapping, 10)
        `
    
    This is useful for when you have more complex mappings that you need to generate, and passing
    it as an object is much much faster than manually entering it on the command line.
    """

    args = get_args()
    print("Enter name of map file: ", end="")
    fname = input().strip()
    grid, needed_mappings = parse_map_file(fname)
    #print("Enter prefix for frame image files: ", end="")
    #prefix = input().strip()
    print("Enter width of each grid cell (in pixels): ", end="")
    width = input().strip()
    
    mapping = dict()

    print("Symbols to map: {}".format(needed_mappings)) 
    print("For each symbol enter RGB mapping, e.g. '255, 0, 0'")

    gradient_begin = (0, 255, 0)
    gradient_step = (255/100, -255/100, 0)
    print("Step is {}".format(gradient_step))
    gradient_end = (255, 0, 0)

    for symbol in needed_mappings:
        if int(symbol) < 0 or int(symbol) > 100:
            print("{} -> Mapping: ".format(symbol), end = "")
            line = input().strip()
            if len(line) == 0:
                break
            line = line.split(',')
            
            mapping[symbol] = (int(line[0]), int(line[1]), int(line[2]))
        else:
            mapping[symbol] = tuple_tween(gradient_begin, gradient_end, int(symbol) / 100)
            print("Auto-mapping {} -> ({})".format(
                symbol,
                mapping[symbol]))



    print("Building frames....")
    file_to_frames(fname, 'frame', mapping, int(width))
    print("Done.")

if __name__ == '__main__':
    main()
