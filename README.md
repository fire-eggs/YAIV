# YAIV
"Yet Another Image Viewer"

A FLTK-based image viewer, providing some additional image formats over the "standard" Linux viewers.

Very much a work-in-progress, mostly a platform for experimentation.

Image Formats Supported:
- Jpeg
- Bitmap
- GIF / animated GIF
- PNG / animated PNG
- Webp / animated Webp
- Limited SVG support (static only)

To load images, the choices include:
1. Add YAIV as an application when double-clicking an image in the file manager.
2. Run YAIV, then drag-and-drop an image from the file manager.
3. Run YAIV, then right-click to show the menu, select "Load".
4. Run YAIV, use the "open file" button on the toolbar
5. Specify a file path or folder path on the command line

Keyboard commands:
- b : toggle the title bar (window is still draggable/sizable : _unless_ panning
      with the mouse is ON; see below)
- c : toggle the checkboard background (visible w/ transparent images)
- h : mark the current file as hidden (currently Linux only)
- m : toggle mini-map display
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

Combines: 
- [FLTK 1.4](https://www.fltk.org/)
- [GIF animation](https://github.com/wcout/fltk-gif-animation) from wcout
- [improved FLTK rotation/scaling](https://github.com/rageworx/fl_imgtk) from rageworx
- minimap idea courtesy of rageworx
- [Animated PNG](http://apngdis.sourceforge.net) support from Max Stepin
- [Webp and animated webp](https://developers.google.com/speed/webp/download) from Google
- Themes from [Rangi42](https://github.com/Rangi42/tilemap-studio)
- Toolbar derived from work by Michael Sweet
- Some toolbar icons from [ImageGlass](https://github.com/d2phap/ImageGlass)

Requirements

  - FLTK 1.4
  - libwebp
  - openmp
  - fl_imgtk

The project currently builds against fltk_png rather than use libpng. That is 
not a 'hard' requirement. I've also been using libturbo_jpeg instead of libjpeg.
