# YAIV
"Yet Another Image Viewer"

A FLTK-based image viewer, intended to (eventually) replace [feh](https://feh.finalrewind.org/) but with additional capabilities.

Very much a work-in-progress, mostly a platform for experimentation.

Combines: 
- [FLTK 1.4](https://www.fltk.org/)
- [GIF animation](https://github.com/wcout/fltk-gif-animation) from wcout
- [improved FLTK rotation/scaling](https://github.com/rageworx/fl_imgtk) from rageworx
- minimap idea courtesy of rageworx
- [Animated PNG](http://apngdis.sourceforge.net) support from Max Stepin
- [Webp and animated webp](https://developers.google.com/speed/webp/download) from Google

The GUI is somewhat limited right now. To load images the choices are:
1. Add YAIV as an application when double-clicking an image in the file manager.
2. Run YAIV, then drag-and-drop an image from the file manager.
3. Run YAIV, then right-click to show the menu, select "Load".

Keyboard commands:
- b : toggle the title bar (window is still draggable/sizable : _unless_ panning
      with the mouse is ON; see below)
- c : toggle the checkboard background (visible w/ transparent images)
- m : toggle mini-map display
- n : center the current image in the window
- o : toggle the image details display
- p : toggle panning with the mouse. Clicking and moving the mouse will pan the
      image. Note the window cannot be moved if the title bar is OFF.
- q : exit
- s : cycle through scaling options (none,auto,fit,to-width,to-height)
- t : cycle through rotation by 90 degrees
- w : toggle slideshow mode (currently hardcoded at 5 seconds per image)
- z : cycle through imgTk scaling options (when not at 100% zoom)
- Ctrl+arrows : scroll the current image
- Up / down arrows: zoom in/out
- Left / right arrows : next/prev image
- Pageup/page down : next/prev image
- Space / backspace : next/prev image

Requirements

  - FLTK 1.4
  - libwebp
  - openmp
  - fl_imgtk

The project currently builds against fltk_png rather than use libpng. That is 
not a 'hard' requirement. I've also been using libturbo_jpeg instead of libjpeg.
