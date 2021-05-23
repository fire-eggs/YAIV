# YAIV
"Yet Another Image Viewer"

A FLTK-based image viewer, intended to (eventually) replace feh but with additional capabilities.

Very much a work-in-progress, mostly a platform for experimentation.

Pulls together [GIF animation](https://github.com/wcout/fltk-gif-animation) from wcout, [improved FLTK rotation/scaling](https://github.com/rageworx/fl_imgtk) from rageworx, Webp and animated webp.

The GUI is very limited right now. To load images the choices are:
1. Add YAIV as an application when double-clicking an image in the file manager.
2. Run YAIV, then drag-and-drop an image from the file manager.
3. Run YAIV, then right-click to show the menu, select "Load".

Keyboard commands:
- t : cycle through rotation by 90 degrees
- n : center the current image in the window
- b : toggle the title bar (window is still draggable/sizable)
- c : toggle the checkboard background (visible w/ transparent images)
- s : cycle through scaling options (none,fit,to-width,to-height)
- z : cycle through imgTk options (when not at 100% zoom)
- Ctrl+arrows : scroll the current image
- Up / down arrows: zoom in/out
- Left / right arrows : next/prev image
- Pageup/page down : next/prev image
- Space / backspace : next/prev image
