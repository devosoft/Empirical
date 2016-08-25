movie:
	./vizualizer.py
	ffmpeg -framerate 3 -i frame%05d.png -c:v libx264 -r 30 -pix_fmt yuv420p out.mp4

clean:
	rm *.png
	rm *.mp4
