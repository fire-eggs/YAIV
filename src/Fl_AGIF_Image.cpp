// * FLTK-1.4.0 warning : 
//      Fl_AGIF_Image has been embedded into FLTK-1.4.0,
#if (FL_MINOR_VERSION<4)

#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

//
// Copyright 2016-2019 Christian Grabner <wcout@gmx.net>
//
// Fl_AGIF_Image class - FLTK animated GIF extension.
//
// Fl_AGIF_Image is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation,  either version 3 of the License, or
// (at your option) any later version.
//
// Fl_AGIF_Image is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY;  without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details:
// http://www.gnu.org/licenses/.
//

#include <cstdlib>
#include <cerrno>
#include <cmath> // lround()
#include <new> // std::nothrow
#include <vector> // std::vector

#include <FL/Fl.H>
#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_Group.H>
#include <algorithm>

#if (FL_MINOR_VERSION<4)
    #error "Error, required FLTK 1.4 or later"
#endif
#include "gif_load.h"
#include "Fl_AGIF_Image.h"

#ifndef _strdup // TODO KBR fl_strdup
#define _strdup strdup
#endif

/*static*/
bool Fl_AGIF_Image::animate = false;

///////////////////////////////////////////////////////////////////////
//  Internal helper classes/structs
///////////////////////////////////////////////////////////////////////

class Fl_AGIF_Image::FrameInfo {
    friend class Fl_AGIF_Image;
public:

    enum Transparency {
        T_NONE = 0xff,
        T_FULL = 0
    };

    struct RGBA_Color {
        uchar r, g, b, alpha;
        RGBA_Color(uchar r_ = 0, uchar g_ = 0, uchar b_ = 0, uchar a_ = T_NONE) :
                r(r_), g(g_), b(b_), alpha(a_) {}
    };

    enum Dispose {
        DISPOSE_UNDEF = GIF_NONE,
        DISPOSE_NOT = GIF_CURR,
        DISPOSE_BACKGROUND = GIF_BKGD,
        DISPOSE_PREVIOUS = GIF_PREV
    };

    struct GifFrame {
        GifFrame() :
                rgb(0),
                scalable(0),
                average_color(FL_BLACK),
                average_weight(-1),
                desaturated(false),
                x(0),
                y(0),
                w(0),
                h(0),
                delay(0),
                dispose(DISPOSE_UNDEF),
                transparent_color_index(-1) {}
        Fl_RGB_Image *rgb;                // full frame image
        Fl_Image *scalable;        // used for hardware-accelerated scaling
        Fl_Color average_color;           // last average color
        float average_weight;             // last average weight
        bool desaturated;                 // flag if frame is desaturated
        int x, y, w, h;                   // frame original dimensions
        double delay;                     // delay (already converted to ms)
        Dispose dispose;                  // disposal method
        int transparent_color_index;      // needed for dispose()
        RGBA_Color transparent_color;     // needed for dispose()
    };

    FrameInfo(Fl_AGIF_Image *anim_) :
            _anim(anim_),
            valid(false),
            frames_size(0),
            frames(0),
            loop_count(1),
            loop(0),
            background_color_index(-1),
            canvas_w(0),
            canvas_h(0),
            desaturate(false),
            average_color(FL_BLACK),
            average_weight(-1),
            scaling((Fl_RGB_Scaling)0),
            _debug(0),
            optimize_mem(false),
            offscreen(0) {}
    ~FrameInfo();
    void clear();
    void copy(const FrameInfo& fi_);
    double convertDelay(int d_) const;
    int debug() const { return _debug; }
    int frame_count(char *buf_, long len_);
    bool load(char *buf_, long len_);
    bool push_back_frame(const GifFrame &frame_);
    void resize(int W_, int H_);
    void scale_frame(int frame_);
    void set_frame(int frame_);
private:
    Fl_AGIF_Image *_anim;         // a pointer to the Image (only needed for name())
    bool valid;                       // flag ig valid data
    int frames_size;                  // number of frames stored in 'frames'
    std::vector<GifFrame>frames;                 // "vector" for frames
    int loop_count;                   // loop count from file
    int loop;                         // current loop count
    int background_color_index;       // needed for dispose()
    RGBA_Color background_color;      // needed for dispose()
    GifFrame frame;                   // current processed frame
    int canvas_w;                     // width of GIF from header
    int canvas_h;                     // height of GIF from header
    bool desaturate;                  // flag if frames should be desaturated
    Fl_Color average_color;           // color for color_average()
    float average_weight;             // weight for color_average (negative: none)
    Fl_RGB_Scaling scaling;           // saved scaling method for scale_frame()
    int _debug;                       // Flag for debug outputs
    bool optimize_mem;                // Flag to store frames in original dimensions
    uchar *offscreen;                 // internal "offscreen" buffer
private:
    static void cb_gl_frame(void *ctx_, GIF_WHDR *whdr_);
    static void cb_gl_extension(void *ctx_, GIF_WHDR *whdr_);
private:
    void dispose(int frame_);
    void onFrameLoaded(GIF_WHDR &whdr_);
    void onExtensionLoaded(GIF_WHDR &whdr_);
    void setToBackGround(int frame_);
};


// -- NOTICE --
// do not define 'DEBUG' as preprocessor,
// most compilers are using this word used to compiler's preprocessor.
#define LOG(x) if (debug()) printf x
   #define DBG(x) if (debug() >= 2) printf x
#ifndef LOG
#define LOG(x)
#endif
    #ifndef DBG
        #define DBG(x)
#endif


//
// helper class FrameInfo implementation
//

Fl_AGIF_Image::FrameInfo::~FrameInfo() {
    clear();
}


void Fl_AGIF_Image::FrameInfo::clear() {
    // release all allocated memory
    while (frames_size-- > 0) {
#if(FL_MINOR_VERSION>3)        
        if (frames[frames_size].scalable)
            frames[frames_size].scalable->release();
#else
        if (frames[frames_size].scalable)
            delete frames[frames_size].scalable;
#endif
        delete frames[frames_size].rgb; // valgrind
    }
    delete[] offscreen;
    offscreen = 0;

    //free(frames); // allocated with realloc delete[] frames;
    //frames = 0;
//    for (int i=0; i < frames_size; i++)
//        delete frames[i];
    frames.clear();
    frames_size = 0;
}


double Fl_AGIF_Image::FrameInfo::convertDelay(int d_) const {
    if (d_ <= 0)
        d_ = loop_count != 1 ? 10 : 0;
    return (double)d_ / 100;
}


/*static*/
void Fl_AGIF_Image::FrameInfo::cb_gl_frame(void *ctx_, GIF_WHDR *whdr_) {
    // called from GIF_Load() when image block loaded
    FrameInfo *fi = (FrameInfo *)ctx_;
    fi->onFrameLoaded(*whdr_);
}


/*static*/
void Fl_AGIF_Image::FrameInfo::cb_gl_extension(void *ctx_, GIF_WHDR *whdr_) {
    // called from GIF_Load() when extension block loaded
    FrameInfo *fi = (FrameInfo *)ctx_;
    fi->onExtensionLoaded(*whdr_);
}


void Fl_AGIF_Image::FrameInfo::copy(const FrameInfo& fi_) {
    // copy from source
    for (int i = 0; i < fi_.frames_size; i++) {
        if (!push_back_frame(fi_.frames[i])) {
            break;
        }
        double scale_factor_x = (double)canvas_w / (double)fi_.canvas_w;
        double scale_factor_y = (double)canvas_h / (double)fi_.canvas_h;
        if (fi_.optimize_mem) {
            frames[i].x = lround(fi_.frames[i].x * scale_factor_x);
            frames[i].y = lround(fi_.frames[i].y * scale_factor_y);
            int new_w = lround(fi_.frames[i].w * scale_factor_x);
            int new_h = lround(fi_.frames[i].h * scale_factor_y);
            frames[i].w = new_w;
            frames[i].h = new_h;
        }
        // just copy data 1:1 now - scaling will be done adhoc when frame is displayed
        frames[i].rgb = (Fl_RGB_Image *)fi_.frames[i].rgb->copy();
        frames[i].scalable = 0;
    }
    optimize_mem = fi_.optimize_mem;
    scaling = Fl_Image::RGB_scaling(); // save current scaling mode
    loop_count = fi_.loop_count; // .. and the loop_count!
}


static void deinterlace(GIF_WHDR &whdr_) {
    if (!whdr_.intr) return;
    // this code is from 'gif_load's example program (with adaptions)
    int iter = 0;
    int ddst = 0;
    int ifin = 4;
    size_t sz = whdr_.frxd * whdr_.fryd;
    uchar *buf = new uchar[sz];
    memset(buf, whdr_.tran, sz);
    for (int dsrc = -1; iter < ifin; iter++)
        for (int yoff = 16 >> ((iter > 1) ? iter : 1), y = (8 >> iter) & 7;
             y < whdr_.fryd; y += yoff)
            for (int x = 0; x < whdr_.frxd; x++)
                if (whdr_.tran != (int)whdr_.bptr[++dsrc])
                    buf[whdr_.frxd * y + x + ddst] = whdr_.bptr[dsrc];
    memcpy(whdr_.bptr, buf, sz);
    delete[] buf;
}


void Fl_AGIF_Image::FrameInfo::dispose(int frame_) {
    if (frame_ < 0) {
        return;
    }
    // dispose frame with index 'frame_' to offscreen buffer
    switch (frames[frame_].dispose) {
        case DISPOSE_PREVIOUS: {
            // dispose to previous restores to first not DISPOSE_TO_PREVIOUS frame
            int prev(frame_);
            while (prev > 0 && frames[prev].dispose == DISPOSE_PREVIOUS)
                prev--;
            if (prev == 0 && frames[prev].dispose == DISPOSE_PREVIOUS) {
                setToBackGround(frame_);
                return;
            }
            DBG(("  dispose frame %d to previous frame %d\n", frame_ + 1, prev + 1));
            // copy the previous image data..
            uchar *dst = offscreen;
            const char *src = frames[prev].rgb->data()[0];
            memcpy((char *)dst, (char *)src, canvas_w * canvas_h * 4);
            break;
        }
        case DISPOSE_BACKGROUND:
            DBG(("  dispose frame %d to background\n", frame_ + 1));
            setToBackGround(frame_);
            break;

        default: {
            // nothing to do (keep everything as is)
            break;
        }
    }
}


int Fl_AGIF_Image::FrameInfo::frame_count(char *buf_, long len_) {
    valid = false;
    return GIF_Load(buf_, len_, 0, 0, this, 0);
}


bool Fl_AGIF_Image::FrameInfo::load(char *buf_, long len_) {
    // decode GIF using gif_load.h
    valid = false;
    GIF_Load(buf_, len_, cb_gl_frame, cb_gl_extension, this, 0);

    delete[] offscreen;
    offscreen = 0;
    return valid;
}


void Fl_AGIF_Image::FrameInfo::onFrameLoaded(GIF_WHDR &whdr_) {
    static bool warn = false;
    if (whdr_.ifrm && !valid) return; // if already invalid, just ignore rest
    int delay = whdr_.time;
    if (delay < 0)
        delay = -(delay + 1);

    LOG(("onFrameLoaded: frame #%d/%d, %dx%d, delay: %d, intr=%d, bkgd=%d/%d, dispose=%d\n",
            whdr_.ifrm + 1, whdr_.nfrm, whdr_.frxd, whdr_.fryd,
            delay, whdr_.intr, whdr_.bkgd, whdr_.clrs, whdr_.mode));

    if (!whdr_.ifrm) {
        // first frame, get width/height
        valid = true; // may be reset later from loading callback
        warn = true;
        canvas_w = whdr_.xdim;
        canvas_h = whdr_.ydim;
        offscreen = new uchar[canvas_w * canvas_h * 4];
        memset(offscreen, 0, canvas_w * canvas_h * 4);
    }

    if (!whdr_.cpal) {
        // no colors: use default table (Note: whdr_.clrs is at least 2)
        static struct GIF_WHDR::CPAL defClrs[256];
        whdr_.cpal = defClrs;
        if (warn) {
            Fl::warning("%s does not have a color table, using default.\n", _anim->name());
            warn = false;
            memset(defClrs, 0, sizeof(defClrs)); // Note: also sets first color to black
            defClrs[1].R = defClrs[1].G = defClrs[1].B = 0xff; // white
            for (int i = 2; i < whdr_.clrs; i++)
                defClrs[i].R = defClrs[i].G = defClrs[i].B = (uchar)(255 * i / (whdr_.clrs - 1));
        }
    }

    deinterlace(whdr_);

    if (!whdr_.ifrm) {
        // store background_color AFTER color table is set
        background_color_index = whdr_.clrs && whdr_.bkgd < whdr_.clrs ? whdr_.bkgd : -1;

        if (background_color_index >= 0) {
            background_color = RGBA_Color(whdr_.cpal[background_color_index].R,
                                          whdr_.cpal[background_color_index].G,
                                          whdr_.cpal[background_color_index].B);
        }
    }

    // process frame
    frame.x = whdr_.frxo;
    frame.y = whdr_.fryo;
    frame.w = whdr_.frxd;
    frame.h = whdr_.fryd;
    frame.delay = convertDelay(delay);
    frame.transparent_color_index = whdr_.tran && whdr_.tran < whdr_.clrs ? whdr_.tran : -1;
    frame.dispose = (Dispose)whdr_.mode;
    if (frame.transparent_color_index >= 0) {
        frame.transparent_color = RGBA_Color(whdr_.cpal[frame.transparent_color_index].R,
                                             whdr_.cpal[frame.transparent_color_index].G,
                                             whdr_.cpal[frame.transparent_color_index].B);
    }
    DBG(("#%d %d/%d %dx%d delay: %d, dispose: %d transparent_color: %d\n",
            (int)frames_size + 1,
            frame.x, frame.y, frame.w, frame.h,
            delay, whdr_.mode, whdr_.tran));

    // we know now everything we need about the frame..
    dispose(frames_size - 1);

    // copy image data to offscreen
    uchar *bits = whdr_.bptr;
    uchar *endp = offscreen + canvas_w * canvas_h * 4;
    for (int y = frame.y; y < frame.y + frame.h; y++) {
        for (int x = frame.x; x < frame.x + frame.w; x++) {
            uchar c = *bits++;
            if (c == whdr_.tran)
                continue;
            uchar *buf = offscreen;
            buf += (y * canvas_w * 4 + (x * 4));
            if (buf >= endp)
                continue;
            *buf++ = whdr_.cpal[c].R;
            *buf++ = whdr_.cpal[c].G;
            *buf++ = whdr_.cpal[c].B;
            *buf = T_NONE;
        }
    }

    // create RGB image from offscreen
    if (optimize_mem) {
        uchar *buf = new uchar[frame.w * frame.h * 4];
        uchar *dest = buf;
        for (int y = frame.y; y < frame.y + frame.h; y++) {
            for (int x = frame.x; x < frame.x + frame.w; x++) {
                if (offscreen + y * canvas_w * 4 + x * 4 < endp)
                    memcpy(dest, &offscreen[y * canvas_w * 4 + x * 4], 4);
                dest += 4;
            }
        }
        frame.rgb = new Fl_RGB_Image(buf, frame.w, frame.h, 4);
    }
    else {
        uchar *buf = new uchar[canvas_w * canvas_h * 4];
        memcpy(buf, offscreen, canvas_w * canvas_h * 4);
        frame.rgb = new Fl_RGB_Image(buf, canvas_w, canvas_h, 4);
    }
    frame.rgb->alloc_array = 1;

    if (!push_back_frame(frame)) {
        valid = false;
    }
}


void Fl_AGIF_Image::FrameInfo::onExtensionLoaded(GIF_WHDR &whdr_) {
    uchar *ext = whdr_.bptr;
    if (memcmp(ext, "NETSCAPE2.0", 11) == 0 && ext[11] >= 3) {
        uchar *params = &ext[12];
        loop_count = params[1] | (params[2] << 8);
        DBG(("netscape loop count: %u\n", loop_count));
    }
}


bool Fl_AGIF_Image::FrameInfo::push_back_frame(const GifFrame &frame_) {
    frames.push_back(frame_);
    frames_size++;
    return true;
}


void Fl_AGIF_Image::FrameInfo::resize(int W_, int H_) {
    double scale_factor_x = (double)W_ / (double)canvas_w;
    double scale_factor_y = (double)H_ / (double)canvas_h;
    for (int i=0; i < frames_size; i++) {
        if (optimize_mem) {
            frames[i].x = lround(frames[i].x * scale_factor_x);
            frames[i].y = lround(frames[i].y * scale_factor_y);
            int new_w = lround(frames[i].w * scale_factor_x);
            int new_h = lround(frames[i].h * scale_factor_y);
            frames[i].w = new_w;
            frames[i].h = new_h;
        }
    }
    canvas_w = W_;
    canvas_h = H_;
}


void Fl_AGIF_Image::FrameInfo::scale_frame(int frame_) {
    // Do the actual scaling after a resize if neccessary
    int new_w = optimize_mem ? frames[frame_].w : canvas_w;
    int new_h = optimize_mem ? frames[frame_].h : canvas_h;
    if (frames[frame_].scalable &&
        frames[frame_].scalable->w() == new_w &&
        frames[frame_].scalable->h() == new_h)
        return;
#if !(FL_ABI_VERSION >= 10304 && USE_SHIMAGE_SCALING)
    else if (frames[frame_].rgb->w() == new_w && frames[frame_].rgb->h() == new_h)
        return;
#endif

    Fl_RGB_Scaling old_scaling = Fl_Image::RGB_scaling(); // save current scaling method
    Fl_Image::RGB_scaling(scaling);
#if FL_ABI_VERSION >= 10304 && USE_SHIMAGE_SCALING
    if (!frames[frame_].scalable) {
    frames[frame_].scalable = Fl_Shared_Image::get(frames[frame_].rgb, 0);
  }
  frames[frame_].scalable->scale(new_w, new_h, 0, 1);
#else
    Fl_RGB_Image *copied = (Fl_RGB_Image *)frames[frame_].rgb->copy(new_w, new_h);
    delete frames[frame_].rgb; // valgrind
    frames[frame_].rgb = copied;
#endif
    Fl_Image::RGB_scaling(old_scaling); // restore scaling method
}


void Fl_AGIF_Image::FrameInfo::setToBackGround(int frame_) {
    // reset offscreen to background color
    int bg = background_color_index;
    int tp = frame_ >= 0 ? frames[frame_].transparent_color_index : bg;
    DBG(("  setToBackGround [%d] tp = %d, bg = %d\n", frame_, tp, bg));
    RGBA_Color color = background_color;
    if (tp >= 0)
        color = frames[frame_].transparent_color;
    if (tp >= 0 && bg >= 0)
        bg = tp;
    color.alpha = tp == bg ? T_FULL : tp < 0 ? T_FULL : T_NONE;
    DBG(("  setToColor %d/%d/%d alpha=%d\n", color.r, color.g, color.b, color.alpha));
    for (uchar *p = offscreen + canvas_w * canvas_h * 4 - 4; p >= offscreen; p -= 4)
        memcpy(p, &color, 4);
}


void Fl_AGIF_Image::FrameInfo::set_frame(int frame_) {
    // scaling pending?
    scale_frame(frame_);

    // color average pending?
    if (average_weight >= 0 && average_weight < 1 &&
        ((average_color != frames[frame_].average_color) ||
         (average_weight != frames[frame_].average_weight))) {
        frames[frame_].rgb->color_average(average_color, average_weight);
        frames[frame_].average_color = average_color;
        frames[frame_].average_weight = average_weight;
    }

    // desaturate pending?
    if (desaturate && !frames[frame_].desaturated) {
        frames[frame_].rgb->desaturate();
        frames[frame_].desaturated = true;
    }
}



///////////////////////////////////////////////////////////////////////
//
// Fl_AGIF_Image
//
// An extension to Fl_GIF_Image.
//
///////////////////////////////////////////////////////////////////////


//
// Fl_AGIF_Image global variables
//

/*static*/
double Fl_AGIF_Image::min_delay = 0.;
/*static*/
bool Fl_AGIF_Image::loop = true;

//
//  helper functions
//
static char *readin(const char *name_, long &sz_) {
    char *buf = nullptr;
    struct stat s = {};
    if (fl_stat(name_, &s)) return buf;
    if (s.st_mode & 0x4000/*_SIFDIR*/) { errno = EISDIR; return buf; }
    sz_ = s.st_size;
    FILE *gif = fl_fopen(name_, "rb"); // KBR needed to add binary flag
    if (!gif)
        return buf;
    buf = new (std::nothrow) char[sz_];
    if (!buf)
    {
        fclose(gif);
        return buf;
    }

    size_t bytesread = fread(buf, 1, sz_, gif);
    if (bytesread != (size_t)sz_) // KBR size difference
    {
        delete[] buf;
        buf = nullptr;
    }

    fclose(gif);
    return buf;
}

//
// class Fl_AGIF_Image implementation
//

Fl_AGIF_Image::Fl_AGIF_Image(const char *name_,
                                     Fl_Widget *canvas_/* = 0*/,
                                     unsigned short flags_/* = 0 */) :
//Inherited((const char *)NULL),  // KBR satisfy Fl_GIF_Image
        Inherited((char *const*)0),  // KBR satisfy Fl_Pixmap
        _name(name_ ? _strdup(name_) : 0),
        _flags(flags_),
        _canvas(canvas_),
        _uncache(false),
        _valid(false),
        _frame(-1),
        _speed(1),
        _fi(new FrameInfo(this)) {
    _fi->_debug = (flags_ & Log) + 2 * (flags_ & Debug);
    _fi->optimize_mem = (flags_ & OptimizeMemory);
    _valid = load(name_);
    
    //printf("Load: base(%d,%d);canvas(%d,%d)\n", w(), h(), canvas_w(), canvas_h());

    // Issue #109: the base class w/h values get set to the *frame* size at some
    // point. However, the defining size of the image is the canvas; a frame may
    // be smaller than the canvas and drawn offset within it.
    /*
    if (canvas_w() && canvas_h()) {
        if (!w() && !h()) {
            w(canvas_w());
            h(canvas_h());
        }
    }
    */
    
    // TODO KBR when would this not be valid to do?
    w(canvas_w());
    h(canvas_h());
    
    canvas(canvas_, flags_);
    if ((flags_ & Start))
        start();
}


Fl_AGIF_Image::Fl_AGIF_Image() :
//  Inherited((const char *)NULL), // KBR satisfy Fl_GIF_Image
        Inherited((char *const*)0),  // KBR satisfy Fl_Pixmap
        _name(0),
        _flags(0),
        _canvas(0),
        _uncache(false),
        _valid(false),
        _frame(-1),
        _speed(1),
        _fi(new FrameInfo(this)) {
}

Fl_AGIF_Image::Fl_AGIF_Image(const char* name, int loopC, int cw, int ch)
        : Fl_AGIF_Image(0L) {
    _name = _strdup(name); // TODO fl_strdup
    _fi->loop_count = loopC;
    _flags = Fl_AGIF_Image::Start;
    _valid = true;
    w(cw);
    h(ch);
    d(4);

    _canvas = nullptr;
    _frame = -1;
    _uncache = false;
    _speed = 1;

    _fi->optimize_mem = false;

    // NOTE: can't call start() until at least one frame loaded
}

/*virtual*/
Fl_AGIF_Image::~Fl_AGIF_Image() {
    Fl::remove_timeout(cb_animate, this);
    delete _fi;
    free(_name); //strdup uses malloc
}


void Fl_AGIF_Image::canvas(Fl_Widget *canvas_, unsigned short flags_/* = 0*/) {
    if (_canvas)
        _canvas->image(nullptr);
    _canvas = canvas_;
    if (_canvas && !(flags_ & DontSetAsImage))
    {
        _canvas->image(this); // set animation as image() of canvas
    }
    if (_canvas && !(flags_ & DontResizeCanvas))
        _canvas->size(w(), h());
    if (_flags != flags_) {
        _flags = flags_;
        _fi->_debug = (flags_ & Log) + 2 * (flags_ & Debug);
    }
    // Note: 'Start' flag is *NOT* used here,
    //       but an already running animation is restarted.
    _frame = -1;
    if (Fl::has_timeout(cb_animate, this)) {
        Fl::remove_timeout(cb_animate, this);
        next_frame();
    }
}


Fl_Widget *Fl_AGIF_Image::canvas() const {
    return _canvas;
}


int Fl_AGIF_Image::canvas_w() const {
    return _fi->canvas_w;
}


int Fl_AGIF_Image::canvas_h() const {
    return _fi->canvas_h;
}


/*static*/
void Fl_AGIF_Image::cb_animate(void *d_) {
    Fl_AGIF_Image *b = (Fl_AGIF_Image *)d_;
    b->next_frame();
}


void Fl_AGIF_Image::clear_frames() {
    _fi->clear();
    _valid = false;
}


/*virtual*/
void Fl_AGIF_Image::color_average(Fl_Color c_, float i_) {
    if (i_ < 0) {
        // immediate mode
        i_ = -i_;
        for (int f=0; f < frames(); f++) {
            _fi->frames[f].rgb->color_average(c_, i_);
        }
        return;
    }
    _fi->average_color = c_;
    _fi->average_weight = i_;
}


/*virtual*/
Fl_Image* Fl_AGIF_Image::copy (int W_, int H_) const
{
    Fl_AGIF_Image *copied = new Fl_AGIF_Image();
    
#if 0 // TODO impact?    
    // copy/resize the base image (Fl_Pixmap)
    // Note: this is not really necessary, if the draw()
    //       method never calls the base class.
    if (_fi->frames_size) {
        w(_fi->frames[0].w);
        h(_fi->frames[0].h);
        Fl_Pixmap *gif = (Fl_Pixmap *)Inherited::copy(W_, H_);
        copied->Inherited::data(gif->data(), gif->count());
        copied->alloc_data = gif->alloc_data;
        gif->alloc_data = 0;
        delete gif;
        w(_fi->canvas_w);
        h(_fi->canvas_h);
    }
#endif

    copied->w(W_);
    copied->h(H_);
    copied->_fi->canvas_w = W_;
    copied->_fi->canvas_h = H_;
    copied->_fi->copy(*_fi); // copy the meta data

    copied->_uncache = _uncache; // copy 'inherits' frame uncache status
    copied->_valid = _valid && copied->_fi->frames_size == _fi->frames_size;
    copied->scale_frame(); // scale current frame now // TODO scaled original; impact?
    if (copied->_valid && _frame >= 0 && !Fl::has_timeout(cb_animate, copied))
        copied->start(); // start if original also was started
    return copied;
}


int Fl_AGIF_Image::debug() const {
    return _fi->debug();
}


double Fl_AGIF_Image::delay(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].delay;
    return 0.;
}


void Fl_AGIF_Image::delay(int frame_, double delay_) {
    if (frame_ >= 0 && frame_ < frames())
        _fi->frames[frame_].delay = delay_;
}


/*virtual*/
void Fl_AGIF_Image::desaturate() {
    _fi->desaturate = true;
}


/*virtual*/
void Fl_AGIF_Image::draw(int x_, int y_, int w_, int h_, int cx_/* = 0*/, int cy_/* = 0*/) {
    if (this->image()) {
        if (_fi->optimize_mem) {
            int f0 = _frame;
            while (f0 > 0 && !(_fi->frames[f0].x == 0 && _fi->frames[f0].y == 0 &&
                               _fi->frames[f0].w == w() && _fi->frames[f0].h == h()))
                --f0;
            for (int f = f0; f <= _frame; f++) {
                if (f < _frame && _fi->frames[f].dispose == FrameInfo::DISPOSE_PREVIOUS) continue;
                if (f < _frame && _fi->frames[f].dispose == FrameInfo::DISPOSE_BACKGROUND) continue;
                Fl_RGB_Image *rgb = _fi->frames[f].rgb;
                if (rgb) {
                    rgb->draw(x_ + _fi->frames[f].x, y_ + _fi->frames[f].y, w_, h_, cx_, cy_);
                }
            }
        }
        else {
            this->image()->draw(x_, y_, w_, h_, cx_, cy_);
        }
    } else {
        // Note: should the base class be called here?
        //       If it is, then the copy() method must also
        //       copy the base image!
//    Inherited::draw(x_, y_, w_, h_, cx_, cy_);
    }
}


int Fl_AGIF_Image::frame() const {
    return _frame;
}


void Fl_AGIF_Image::frame(int frame_) {
    if (Fl::has_timeout(cb_animate, this)) {
        Fl::warning("Fl_AGIF_Image::frame(%d): not idle!\n", frame_);
        return;
    }
    if (frame_ >= 0 && frame_ < frames()) {
        set_frame(frame_);
    }
    else {
        Fl::warning("Fl_AGIF_Image::frame(%d): out of range!\n", frame_);
    }
}


int Fl_AGIF_Image::frame_count(const char *name_) {
    long len = 0;
    char *buf = readin(name_, len);
    // decode GIF using gif_load.h
    int frames = _fi->frame_count(buf, len);
    delete[] buf;
    return frames;
}


int Fl_AGIF_Image::frame_x(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].x;
    return -1;
}


int Fl_AGIF_Image::frame_y(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].y;
    return -1;
}


int Fl_AGIF_Image::frame_w(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].w;
    return -1;
}

int Fl_AGIF_Image::frame_h(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].h;
    return -1;
}


void Fl_AGIF_Image::frame_uncache(bool uncache_) {
    _uncache = uncache_;
}


bool Fl_AGIF_Image::frame_uncache() const {
    return _uncache;
}


int Fl_AGIF_Image::frames() const {
    return _fi->frames_size;
}


Fl_Image *Fl_AGIF_Image::image() const {
    return _frame >= 0 && _frame < frames() ? _fi->frames[_frame].rgb : 0;
}


Fl_Image *Fl_AGIF_Image::image(int frame_) const {
    if (frame_ >= 0 && frame_ < frames())
        return _fi->frames[frame_].rgb;
    return 0;
}


bool Fl_AGIF_Image::is_animated() const {
    return _valid && _fi->frames_size > 1;
}


bool Fl_AGIF_Image::load(const char *name_) {
    DBG(("\nFl_AGIF_Image::load '%s'\n", name_));
    clear_frames();
    free(_name); // strdup uses malloc
    _name = name_ ? _strdup(name_) : nullptr;

    // as load() can be called multiple times
    // we have to replicate the actions of the pixmap destructor here
    uncache();
    if (alloc_data) {
        for (int i = 0; i < count(); i ++) delete[] (char *)data()[i];
        delete[] (char **)data();
    }
    alloc_data = 0;
    w(0);
    h(0);

    if (!name_)
        return false;

    {
        FILE *fp;
        char header[10];
        // Pre-test for GIF header. Avoids the "not a gif file" message from FL_GIF_Image.
        if ((fp = fl_fopen(name_, "rb")) == nullptr)
            return false;
        size_t count = fread(header, 1, sizeof(header), fp);
        fclose(fp);
        if (count==0)
            return false;
        if (header[0] != 'G' || header[1] != 'I' || header[2] != 'F' || header[3] != '8')
            return false;
    }

    // load the base class pixmap
    Fl_GIF_Image tmp(name_);
    if (tmp.ld() || tmp.w() <= 0 || tmp.h() <= 0 || !tmp.data())
        return false;
    Inherited::data(tmp.data(), tmp.count());
    alloc_data = tmp.alloc_data;
    tmp.alloc_data = 0;
    w(tmp.w());
    h(tmp.h());

    // KBR TODO if name_ is null above, when/how would we get here?

    // read gif file into memory
    long len = 0;
    char *buf = readin(name_, len);
    if (!buf) {
        Fl::error("Fl_Anim_GIF: Unable to open '%s': %s\n", name_, strerror(errno));
        ld(ERR_FILE_ACCESS);
        delete[] buf;
        return false;
    }

    // do own signature checking, to issue proper error/warning msg
    // (gif_load accepts only 'GIF87a' or 'GIF89a')
    if (len < 6 || buf[0] !='G' || buf[1] !='I' || buf[2] != 'F') {
        // KBR Fl::error("Fl_GIF_Image: %s is not a GIF file.\n", name_);
        ld(ERR_FORMAT);
        delete[] buf;
        return false;
    }
    if (buf[3]!='8' || buf[4] >'9' || buf[5] != 'a') {
        Fl::warning("%s is version %c%c%c.", name_, buf[3], buf [4], buf[5]);
        buf[3] = '8'; buf[4]='9'; buf[5] = 'a'; // makes gif_load happy
    }

    // decode GIF using gif_load.h
    _fi->load(buf, len);
    delete[] buf;
    _frame = _fi->frames_size - 1;
    _valid = _fi->valid;

    if (!_valid) {
        //Fl::error("Fl_Anim_GIF: %s has invalid format.\n", name_);
        ld(ERR_FORMAT);
    }
    return _valid;
} // load


const char *Fl_AGIF_Image::name() const {
    return _name;
}


bool Fl_AGIF_Image::next_frame() {
    int frame(_frame);
    frame++;
    if (frame >= _fi->frames_size)  {
        _fi->loop++;
        if (Fl_AGIF_Image::loop && _fi->loop_count > 0 && _fi->loop > _fi->loop_count) {
            DBG(("loop count %d reached - stopped!\n", _fi->loop_count));
            stop();
        }
        else
            frame = 0;
    }
    if (frame >= _fi->frames_size)
        return false;
    set_frame(frame);
    double delay = _fi->frames[frame].delay;
    if (min_delay && delay < min_delay) {
        DBG(("#%d: correct delay %f => %f\n", frame, delay, min_delay));
        delay = min_delay;
    }
    if (is_animated() && delay > 0 && _speed > 0) {  // normal GIF has no delay
        delay /= _speed;
        Fl::add_timeout(delay, cb_animate, this);
    }
    return true;
}


Fl_AGIF_Image& Fl_AGIF_Image::resize(int W_, int H_) {
    int W(W_);
    int H(H_);
    if (_canvas && !W && !H) {
        W = _canvas->w();
        H = _canvas->h();
    }
    if (!W || !H || ((W == w() && H == h()))) {
        return *this;
    }
    _fi->resize(W, H);
    scale_frame(); // scale current frame now
    w(_fi->canvas_w);
    h(_fi->canvas_h);
    if (_canvas && !(_flags & DontResizeCanvas)) {
        _canvas->size(w(), h());
    }
    return *this;
}


Fl_AGIF_Image& Fl_AGIF_Image::resize(double scale_) {
    return resize(lround((double)w() * scale_), lround((double)h() * scale_));
}


void Fl_AGIF_Image::scale_frame() {
    int i(_frame);
    if (i < 0)
        return;
    _fi->scale_frame(i);
}


void Fl_AGIF_Image::set_frame(int frame_) {
    int last_frame = _frame;
    _frame = frame_;
    // NOTE: uncaching decreases performance, but saves a lot of memory
    if (_uncache && this->image())
        this->image()->uncache();

    _fi->set_frame(_frame);

    if (canvas()) {
        canvas()->parent() &&
        (_frame == 0 || (last_frame >= 0 && (_fi->frames[last_frame].dispose == FrameInfo::DISPOSE_BACKGROUND  ||
                                             _fi->frames[last_frame].dispose == FrameInfo::DISPOSE_PREVIOUS))) &&
        (canvas()->box() == FL_NO_BOX || (canvas()->align() && !(canvas()->align() & FL_ALIGN_INSIDE)))      ?
        canvas()->parent()->redraw() : canvas()->redraw();
    }
}


double Fl_AGIF_Image::speed() const {
    return _speed;
}


void Fl_AGIF_Image::speed(double speed_) {
    _speed = speed_;
}


bool Fl_AGIF_Image::start() {
    Fl::remove_timeout(cb_animate, this);
    if (_fi->frames_size) {
        next_frame();
    }
    return _fi->frames_size != 0;
}


bool Fl_AGIF_Image::stop() {
    Fl::remove_timeout(cb_animate, this);
    return _fi->frames_size != 0;
}


/*virtual*/
void Fl_AGIF_Image::uncache() {
    Inherited::uncache();
    for (int i=0; i < _fi->frames_size; i++) {
        if (_fi->frames[i].rgb) _fi->frames[i].rgb->uncache();
    }
}


bool Fl_AGIF_Image::valid() const {
    return _valid;
}

bool Fl_AGIF_Image::add_frame(unsigned char *frameRGBA, int duration, int cw, int ch, bool alloc) {

    memset((void*)&_fi->frame, 0, sizeof(FrameInfo::GifFrame)); // KBR cast to remove warning

    _fi->canvas_w = cw ;
    _fi->canvas_h = ch ;

    _fi->frame.w = cw;
    _fi->frame.h = ch;
    _fi->frame.rgb = new Fl_RGB_Image(frameRGBA, cw, ch, 4);
    _fi->frame.rgb->alloc_array = alloc;
    _fi->frame.delay = _fi->convertDelay(duration) / 10.0;

    return _fi->push_back_frame(_fi->frame);
}

void Fl_AGIF_Image::scale(int width, int height, int proportional, int can_expand) {
    Inherited::scale(width,height,proportional,can_expand);
    for (int i=0; i < _fi->frames_size; i++) {
        if (_fi->frames[i].rgb)
            _fi->frames[i].rgb->scale(width,height,proportional,can_expand);
    }
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif

#endif /// of if (FL_MINOR_VERSION<4)
