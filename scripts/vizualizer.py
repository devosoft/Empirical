#!/usr/bin/env python3

from PIL import Image

def print_list(l):
    for blarg in l:
        print(blarg)

def parse_map_file(fname):
    grid_timeline = []

    num_lines = 0

    with open(fname, 'r') as mapfile:
        for line in mapfile:

            if num_lines == 0:
                num_lines += 1
                continue
            else:
                chopped = line.split(",", 1)[1]
                chopped = chopped.replace(" } \n", "").split("[")

                current_state = []
                for element in chopped:
                    gridline = []
                    for subel in element.split(","):
                        filtered = subel.replace("]", "").replace("{", "").strip()
                        if len(filtered) > 0:
                            gridline.append(int(filtered))
                    
                    if len(gridline) > 0:
                        current_state.append(gridline)
                
                grid_timeline.append(current_state)

    return grid_timeline

def fill_square(px, x, y, pixwidth, coloring):
    for i in range(x * pixwidth, x*pixwidth + pixwidth):
        for j in range(y * pixwidth, y*pixwidth + pixwidth):
            px[i,j] = coloring

def gen_image(grid, mapping, pixwidth = 10):
    img = Image.new('RGB', (len(grid) * pixwidth, len(grid[0]) * pixwidth), "black")

    pixels = img.load()

    for i in range(0, len(grid)):
        for j in range(0, len(grid[i])):
            fill_square(pixels, i, j, pixwidth, mapping[grid[i][j]])

    return img

def file_to_frames(gridfname, prefix, mapping, pixwidth):
    grid = parse_map_file(gridfname)

    framenum = 0
    for snapshot in grid:
        img = gen_image(snapshot, mapping, pixwidth)
        img.save(prefix + "%05d.png" % framenum)
        framenum += 1
        

def main():
    print("Enter name of map file: ", end="")
    fname = input().strip()
    #print("Enter prefix for frame image files: ", end="")
    #prefix = input().strip()
    print("Enter width of each grid cell (in pixels): ", end="")
    width = input().strip()
    
    mapping = dict()

    print("Enter symbol -> RGB mapping, e.g. '10, 255, 0, 0'")
    print("This will display all '10's in the map as red squares.")
    print("Blank to continue.")

    while True:
        print("Symbol -> Mapping: ", end = "")
        line = input().strip()
        if len(line) == 0:
            break
        line = line.split(',')
        
        mapping[int(line[0])] = (int(line[1]), int(line[2]), int(line[3]))

    print("Building frames....")
    file_to_frames(fname, 'frame', mapping, int(width))
    print("Done.")

if __name__ == '__main__':
    main()
