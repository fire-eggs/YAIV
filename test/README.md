A very basic "smoke test" for YAIV. The image files in this folder are small 
examples of the known variants of supported image formats.

Make certain that `test.sh` is executable. Make certain the path to `yaiv` is 
correct.

Execute the script: `.\test.sh`

YAIV will run, in slideshow mode, with a delay of 2 seconds between images. At 
the end of the slideshow, YAIV will shut down. The final line of output should 
be "nn yaiv.log", where "nn" matches the (number_of_files_in_test_ + 2). If YAIV 
crashes, the last line of `yaiv.log` should show the filename of the crashing 
file.

The script is currently configured to use a scaling of "none" and a dithering 
of "none". These can be changed to any of the supported scaling and dithering 
values. See scalemodes.h and tkscalemodes.h for names.

TODO: document the command line options

TODO: provide the ability to set a different slideshow delay

TODO: expand the script to exercise combinations of scale/dither values

TODO: command line options for checker, overlay, and minimap
