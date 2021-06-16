A very basic "acid test" for YAIV. The image files in this folder are small examples of the known variants of 
supported image formats.

Make certain that `test.sh` is executable. Make certain the path to `yaiv` is correct.

Execute the script: `.\test.sh`

YAIV will run, in slideshow mode, with a delay of 2 seconds between images. At the end of the slideshow, YAIV will shut down. 
The final line of output should be "24 yaiv.log". If yaiv crashes, the last line of `yaiv.log` should show the last successfully
loaded file.

The script is currently configured to use a scaling of "none" and a dithering of "none". These can be changed to any of the 
supported scaling and dithering values. See scalemodes.h and tkscalemodes.h for names.

TODO: provide the ability to set a different slideshow delay
TODO: expand the script to exercise combinations of scale/dither values
