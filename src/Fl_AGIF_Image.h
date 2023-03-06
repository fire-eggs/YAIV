//
// Created by kevin on 5/11/21.
//

// * FLTK-1.4.0 warning : 
//      Fl_AGIF_Image has been embedded into FLTK-1.4.0,
#ifndef CLION_TEST2_Fl_AGIF_Image_H
#define CLION_TEST2_Fl_AGIF_Image_H

class Fl_Image;
class Fl_Widget;

#include <FL/Fl_Pixmap.H>

/**
 The Fl_AGIF_Image class supports loading, caching,
 and drawing of animated Compuserve GIF<SUP>SM</SUP> images.
 The class loads all images contained in the file and animates
 them by cycling through them as defined by the delay times in
 the image file.

 You must supply an FLTK widget as "container" in order to see
 the animation by specifying it in the constructor or later
 using the canvas() method.
 */
//typedef Fl_GIF_Image Inherited;
typedef Fl_Pixmap Inherited;

class FL_EXPORT Fl_AGIF_Image : public Inherited {
public:

    static bool animate;
    class FrameInfo; // internal helper class

    /**
     When opening a Fl_AGIF_Image there are some options
     that can be passed in a 'flags' value.
     */
    enum Flags {
        /**
         This flag indicates to the loader that it should start
         the animation immediately after successful load.
         */
        Start = 1,
        /**
         This flag indicates to the loader that it should not
         resize the canvas widget of the animation to the dimensions
         of the animation, which is the default.
         Needed for special use cases
         */
        DontResizeCanvas = 2,
        /**
         This flag indicates to the loader that it should not
         set the animation as image() member of the canvas widget,
         which is the default.
         Needed for special use cases.
         */
        DontSetAsImage = 4,
        /**
         Often frames change just a small area of the animation canvas.
         This flag indicates to the loader to try using less memory,
         by storing frame data not as canvas-sized images but use the
         sizes defined in the GIF file.
         The drawbacks are higher cpu usage during playback and maybe
         minor artefacts when resized.
         */
        OptimizeMemory = 8,
        /**
         This flag can be used to print informations about the
         decoding process to the console.
         */
        Log = 64,
        /**
         This flag can be used to print even more informations about
         the decoding process to the console.
         */
        Debug = 128
    };
    /**
     The constructor creates an new animated gif object from
     the given file.
     Optionally it applies the canvas() method after successful load.
     If 'Start' is specified in the 'flags' parameter it calls start()
     after successful load.
     */
    explicit Fl_AGIF_Image(const char *name_, Fl_Widget *canvas_ = 0, unsigned short flags_ = 0);
    Fl_AGIF_Image();
    Fl_AGIF_Image(const char *name, int loopCount, int W, int H); // KBR webp

    ~Fl_AGIF_Image() override;
    /**
     The canvas() method sets or gets the current widget, that
     is used for display of the frame images.
     The _flags_ parameter specifies wheather the canvas widget
     is resized to the animation dimensions and/or its image()
     method will be used to set the current frame image
     during animation.
     */
    void canvas(Fl_Widget *canvas_, unsigned short flags_ = 0);
    Fl_Widget *canvas() const;
    /**
     Return the width and height of the animation canvas as
     specified in the file header
     */
    int canvas_w() const;
    /**
     Return the width and height of the animation canvas as
     specified in the file header
     */
    int canvas_h() const;
    /**
     The color_average() method applies the specified color_average
     to all frames of the animation.
     */
    virtual void color_average(Fl_Color c_, float i_) override;
    /**
     The virtual copy() method makes a copy of the animated image
     and resizes all of its frame images to W x H using
     the current resize method.
     */
    Fl_Image *copy (int W_, int H_) const override;
    int debug() const;
    /**
     The desaturate() method applies desaturate() to all frames
     of the animation.
     */
    void desaturate() override;
    void draw(int x_, int y_, int w_, int h_, int cx_ = 0, int cy_ = 0) override; // TODO default parameters not valid on overrides
    /**
     Return the delay of frame 'frame_' `[0-frames() -1]` in seconds
     */
    double delay(int frame_) const;
    /**
     Set the delay of frame 'frame_' `[0-frames() -1]` in seconds
     */
    void delay(int frame_, double delay_);
    /**
     Return the number of frames.
     */
    int frames() const;
    /**
     Set the current frame in the range index `[0-frames() -1]`
     */
    void frame(int frame_);
    /**
     Return the current frame in the range index `[0-frames() -1]`
     or -1 if the image has no frames.
     */
    int frame() const;
    /**
     Return the current frame image.
     */
    Fl_Image *image() const;
    /**
     Return the frame image of frame 'frame_'
     */
    Fl_Image *image(int frame_) const;
    /**
     The is_animated() method is just a convenience method for
     testing the valid flag and the frame count beeing greater 1.
     */
    bool is_animated() const;
    int frame_count(const char *name_);
    /**
     Use frame_uncache() to set or forbid frame image uncaching.
     If frame uncaching is set, frame images are not offscreen cached
     for re-use and will be re-created every time they are displayed.
     This saves a lot of  memory on the expense of cpu usage and
     should to be carefully considered. Per default frame caching will
     be done.
     */
    void frame_uncache(bool uncache_);
    /**
     Return the active frame_uncache() setting.
     */
    bool frame_uncache() const;
    /**
     The load() method is either used from the constructor to load
     the image from the given file, or to re-load an existing
     animation from another file.
     */
    bool load(const char *name_);

    /**
     The loop flag can be used to (dis-)allow loop count.
     If set (which is the default), the animation will be
     stopped after the number of repeats specified in the
     GIF file (typically this count is set to 'forever' anyway).
     If cleared the animation will always be 'forever',
     regardless of what is specified in the GIF file.
     */
    static bool loop;
    /**
     The min_delay value can be used to set a minimum value
     for the frame delay for playback. This is to prevent
     cpu hogs caused by images with very low delay rates.
     This is a global value for all Fl_AGIF_Image objects.
     */
    static double min_delay;
    /**
     Return the name of the played file as specified in the constructor.
     */
    const char *name() const;
    /**
     The start() method (re-)starts the playing of the frames.
     */
    bool start();
    /**
     The stop() method stops the playing of the frames.
     */
    bool stop();
    /**
     The resize() method resizes the image to the
     specified size, replacing the current image.
     */
    Fl_AGIF_Image& resize(int W_, int H_);
    Fl_AGIF_Image& resize(double scale_);
    /**
     The speed() method changes the playing speed
     to speed_ x original speed. E.g. to play at half
     speed call it with 0.5, for double speed with 2.
     */
    void speed(double speed_);
    double speed() const;
    /**
      Uncache all cached image data now. Re-implemented from Fl_Pixmap.
    */
    virtual void uncache() override;
    /**
     The valid() method returns if the class has
     successfully loaded and the image has at least
     one frame.
     */
    bool valid() const;
    /**
     Return the frame position of frame 'frame_'
     Usefull only if loaded with 'optimize_mem' and
     the animation also has size optimized frames.
     */
    int frame_x(int frame_) const;
    int frame_y(int frame_) const;
    /**
     Return the frame dimensions of frame 'frame_'.
     Useful only if loaded with 'optimize_mem' and
     the animation also has size optimized frames.
     */
    int frame_w(int frame_) const;
    int frame_h(int frame_) const;

    /* add a decoded frame from a non-GIF source */
    bool add_frame(unsigned char *frameRGBA, int duration, int cw, int ch, bool alloc=false);  // KBR apng, webp

    /** Sets the drawing size of the image. See Fl_Image::scale() for details.
     */
    void scale(int width, int height, int proportional=1, int can_expand=0) override;

protected:
    bool next_frame();
    void clear_frames();
    void set_frame(int frame_);
private:
    static void cb_animate(void *d_);
    void scale_frame();
private:
    char *_name;
    unsigned short _flags;
    Fl_Widget *_canvas;
    bool _uncache;
    bool _valid;
    int _frame; // current frame
    double _speed;
    FrameInfo *_fi;
};

#endif //CLION_TEST2_Fl_AGIF_Image_H
