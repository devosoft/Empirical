import numpy as np
import matplotlib.pyplot as plt
import argparse
import sys

# https://www.pyimagesearch.com/2017/06/19/image-difference-with-opencv-and-python/
# construct the argument parse and parse the arguments
ap = argparse.ArgumentParser()
ap.add_argument("-f", "--first", required=True,
	help="first input image")
ap.add_argument("-s", "--second", required=True,
	help="second")
args = vars(ap.parse_args())

# load the two input images
imageA = plt.imread(args["first"])
imageB = plt.imread(args["second"])

# Mean squared error
err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
err /= float(imageA.shape[0] * imageA.shape[1])
print("MSE ", args["first"], " to ", args["second"], ": ", err)
tolerance = 0.01    # A 1% differences is acceptable
if err <= tolerance:
    sys.exit(0)
    
sys.exit(1)
