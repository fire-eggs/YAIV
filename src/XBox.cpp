#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

//
// Created by kevin on 5/12/21.
//
#include <cstring>
#include <cmath> // lround

#include <FL/filename.H>
#include <FL/Fl_Image_Surface.H>
#include <FL/Fl_SVG_Image.H>

#include "list_rand.h"
#include "fl_imgtk.h"
#include "Slideshow.h"
#include "XBoxDisplayInfoEvent.h"
#include "mediator.h" // TODO define messages elsewhere?

#ifdef DANBOORU
#include "danbooru.h"
#endif

#define snprintf_nowarn(...) \
            (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

#if (FL_MINOR_VERSION < 4)
    #define data_w  w
    #define data_h  h
#endif

#ifndef __APPLE__
    #define CTRL_P_KEY      FL_CTRL
#else
    #define CTRL_P_KEY      FL_COMMAND
#endif

#ifdef DANBOORU
static const std::string LOGFILE = "yaiv_db.log";
#else
static const std::string LOGFILE = "yaiv.log";
#endif

void logit(const char *format, char *arg) // TODO varargs
{
    FILE *f = fopen(LOGFILE.c_str(), "a+");
    fprintf(f, format, arg);
    fputs("\n", f);
    fclose(f);
}

int filename_path(const char* buf, char *to) { // TODO hack pending adding to FLTK
    const char *p = buf + strlen(buf) - 1;
    for (; *p != '/' && p != buf; --p) // TODO slash is possible '\' under windows
        ;
    if (p == buf) return 0;
    strncpy(to, buf, (p-buf)+1);
    to[(p-buf)] = '\0';
    return 1;
}

int XBox::find_file(const char *n) {
    // determine the index in the file_list of the given filename
    const char *outfn = fl_filename_name(n);
    logit("file_file:|%s|", (char *)outfn);
    for (int i=0; i < file_count; i++)
        if (strcmp(file_list[i]->d_name, outfn) == 0)
            return i;
    return 0;
}

//TODO a file-filter callback is not in "vanilla" FLTK
#if 0
int removeFolders(struct dirent *entry) {
    // TODO this is a hack, we're not provided the base path
    const char * out = fl_filename_ext(entry->d_name);
    bool val = out != nullptr && *out != 0 && (out[1] != 0);
    return val;
}
#endif

void XBox::load_filelist(const char *n) {

    if (file_list)
        fl_filename_free_list(&file_list, file_count);

    if (!fl_filename_isdir(n))
        filename_path(n, folder_name);
    else
        fl_filename_absolute(folder_name, FL_PATH_MAX, n);

    // TODO how best to filter for images?
    //TODO a file-filter callback is not in "vanilla" FLTK
    //file_count = fl_filename_list(folder_name, &file_list, fl_numericsort, removeFolders);
    file_count = fl_filename_list(folder_name, &file_list, fl_numericsort);
}

void XBox::load_file(const char *n) {

    load_filelist(n); // TODO background process
    current_index = 0;
    if (!fl_filename_isdir(n))
        current_index = find_file(n);
    load_current();

    // TODO don't add to MRU if unsuccessful load
    // Update the MRU list
    _mru->Add(n);
    _mru->Save();
}

void XBox::load_current() {
    if (!file_list || file_count < 1)
        return;

    current_index = std::min(std::max(current_index,0), file_count-1);

    if (folder_name[strlen(folder_name) - 1] == '/')
        folder_name[strlen(folder_name) - 1] = 0x0;
    strncpy(file_name, file_list[current_index]->d_name, FL_PATH_MAX);

    char fullpath[FL_PATH_MAX<<2];
    sprintf(fullpath, "%s/%s", folder_name, file_list[current_index]->d_name);

    logit("Load %s", fullpath);

    rotation = 0; // TODO anything else need resetting?

    if (fl_filename_isdir(fullpath))
    {
        align(FL_ALIGN_CENTER);
        label("@fileopen"); // show a generic folder
        labelsize(64);
        labelcolor(FL_LIGHT2);
        image(nullptr,nullptr);
    }
    else
    {
        Fl_Image *img = loadFile(fullpath, this);

        // 590B-01.jpg failed to load and resulted in crash further on
        if (!img || img->fail() || img->w() == 0 || img->h() == 0)
        {
            // failed to load - show a red file placeholder
            align(FL_ALIGN_CENTER);
            label("@filenew");
            labelsize(64);
            labelcolor(FL_RED);
            image(nullptr,nullptr);
        }
        else
        {
            Fl_Anim_GIF_Image::min_delay = 0.01;
            Fl_Anim_GIF_Image::animate = true;

#ifdef DANBOORU
            update_danbooru(file_name);
#endif

            label(nullptr); // wipe any previous folder/error-file
            Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(img);
            image(img, animgif);
        }
    }

    redraw();
    updateLabel();
}

void XBox::next_image() {
//    current_index = std::min(current_index+1, file_count-1);
    current_index++;
    if (current_index >= file_count) {
        current_index = file_count-1;
        if (_quitAtEnd) // command line option
            exit(0);
    }
    load_current();
}

void XBox::prev_image() {
    current_index = std::max(current_index-1, 0);
    load_current();
}

void XBox::action(int act)
{
    switch (act)
    {
        case Mediator::ACT_NEXT:
            deltax = deltay = 0;
            next_image();
            break;
        case Mediator::ACT_PREV:
            deltax = deltay = 0;
            prev_image();
            break;
        case Mediator::ACT_CHK:
            draw_check = !draw_check;
            updateImage();
            redraw();
            break;
        case Mediator::ACT_ZMI:
            change_zoom(+1);
            redraw();
            break;
        case Mediator::ACT_ZMO:
            change_zoom(-1);
            redraw();
            break;
        case Mediator::ACT_ROTR:
            nextRotation();
            break;
        case Mediator::ACT_OPEN:
            load_request();
            this->take_focus();
            break;
        case Mediator::ACT_GOTO:
            goto_request();
            break;
        default:
            return;
    }
}

int XBox::key(int fullkey)
{
    // TODO this moves to mediator to lookup key customization

    int key = fullkey & FL_KEY_MASK;
    int ctrl = fullkey & FL_CTRL;
#ifdef __APPLE_
    ctrl = fullkey & FL_COMMAND;
#endif

    switch (key)
    {
        case 'q':
            exit(0);

        case 'c':
            action(Mediator::ACT_CHK);
            return 1;

        case 's':
            next_scale();
            return 1;

        case 'n':
            draw_center = !draw_center;
            if (!draw_center)
                deltax = deltay = 0;
            redraw();
            return 1;

        case 'r':
        {
            if (!file_list || file_count<=1)
                break;

            struct dirent **newlist = list_randomize(file_list, file_count);

            for (int i=0; i<file_count; i++)
                file_list[i] = newlist[i];

            free(newlist);
            load_current();
        }
            return 1;

        case FL_Right:
            if (ctrl)
            {
                deltax -= _scroll_speed; // direction matches FEH
                redraw();
            }
            else
            {
                action(Mediator::ACT_NEXT);
            }
            return 1;

        case FL_Left: // TODO consider for pan
            if (ctrl)
            {
                deltax += _scroll_speed; // direction matches FEH
                redraw();
            }
            else
            {
                action(Mediator::ACT_PREV);
            }
            return 1;

        case FL_Up:
            if (ctrl)
            {
                deltay += _scroll_speed; // direction matches FEH
                redraw();
            }
            else
            {
                action(Mediator::ACT_ZMI);
            }
            return 1;

        case FL_Down:
            if (ctrl)
            {
                deltay -= _scroll_speed; // direction matches FEH
                redraw();
            }
            else
            {
                action(Mediator::ACT_ZMO);
            }
            return 1;

        case FL_Page_Down:
        case ' ':
            action(Mediator::ACT_NEXT);
            return 1;

        case FL_Page_Up:
        case FL_BackSpace:
            action(Mediator::ACT_PREV);
            return 1;

        case FL_Home:
            current_index = INT_MIN + 1;
            prev_image();
            return 1;

        case FL_End:
            current_index = INT_MAX - 1;
            next_image();
            return 1;

        case 't':
            action(Mediator::ACT_ROTR);
            return 1;

        case 'z':
            nextTkScale();
            return 1;

        case FL_Escape: // escape to NOT close app
            return 1;

        case 'b':
            notifyBorder();
            return 1;

        case 'w':
            toggleSlideshow();
            return 1;

        case 'm':
            toggleMinimap();
            return 1;

        case 'o':
            toggleOverlay();
            return 1;

        case 'p':
            _pan_with_mouse = !_pan_with_mouse;
            _prefs->set2(MOUSE_PAN, _pan_with_mouse);
            return 1;
            break;

#ifdef DANBOORU
            case 'd':
                if (!file_list || file_count<=1)
                    break;

                if (Fl::event_state() & CTRL_P_KEY)
                {
                    view_danbooru(_prefs);
                    update_danbooru(file_list[current_index]->d_name);
                }
                break;
#endif
    }
    return 0;

}

int XBox::handle(int msg) {
/*
    if (msg == FL_FOCUS || msg == FL_UNFOCUS) // TODO _must_ this go before Fl_Group::handle?
    {
        return 1;
    }
*/
    //if (msg == FL_FOCUS || msg == FL_UNFOCUS) return 0;

    int ret = Fl_Group::handle(msg);

    switch (msg) {
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_LEAVE:
        case FL_DND_RELEASE:
            return 1; // accept drag-and-drop
            break;

        case FL_PASTE: {
            // input will be a URL of form "file://<path>\n" OR "file://<path>\r\n"
            // THERE MAY BE MORE THAN ONE URL! taking only the first
            // TODO confirm that windows DND includes the "file://" prefix!
            const char *urls = Fl::event_text();
#ifndef _WINDOWS
            if (strncmp(urls, "file://", 7) != 0)
                return 1; // not a local file, do nothing

            size_t len = strlen(urls);
            int i = 0;
            while (urls[i] != '\r' && urls[i] != '\n' && i < len) i++;
            char *fpath = new char[i + 1 - 7];
            strncpy(fpath, urls + 7, i + 1 - 7);
            fpath[i - 7] = '\0';
            fl_decode_uri(fpath);
            load_file(fpath);
            delete[] fpath;
#else
            load_file(urls); // TODO multiple drag sources
#endif
            take_focus();
            return 1;
        }
            break;

        case FL_PUSH:
            if (Fl::event_button() == FL_RIGHT_MOUSE) {
                do_menu();
                return 1;
            }
            return mousePan(FL_PUSH);
            break;
        case FL_DRAG:
        case FL_RELEASE:
            return mousePan(msg);
            break;
        default:
            break;
    }

    if (msg == FL_KEYDOWN) {
        return key(Fl::event_key());
    }

    return ret;
}

int XBox::mousePan(int msg)
{
    if (!_pan_with_mouse || !_img) return 0;

    int mouseX = Fl::event_x();
    int mouseY = Fl::event_y();

    switch (msg) {
        case FL_PUSH:
            dragStartX = mouseX - deltax;
            dragStartY = mouseY - deltay;
            dragging = true;
            break;
        case FL_DRAG: {
            if (!dragging) return 0;

            int movX = dragStartX - mouseX;
            int movY = dragStartY - mouseY;
            fl_cursor(FL_CURSOR_MOVE);

            deltax = -movX;
            deltay = -movY;
            redraw();
            }
            break;
        case FL_RELEASE:
            fl_cursor(FL_CURSOR_ARROW);
            dragging = false;
            break;
    }
    return 1;
}

char * XBox::getLabel(bool include_filename, char *buff, int buffsize)
{
    // TODO customize label : full path or just filename
    std::string fullpath = folder_name + std::string("/") + file_name;

    int outzoom = (int)(_zoom * 100.0 + 0.5);
    int w = _img ? _img->w() : 0;
    int h = _img ? _img->h() : 0;
    int depth = _img ? _img->d() : 0;

    char scaletxt[10];
    char * res = humanScale(draw_scale, scaletxt, sizeof(scaletxt)-1);

    if (res == nullptr || _img == nullptr)
    {
        // for the image label currently don't have filename, don't display extra dashes
        snprintf( buff, buffsize, "%d/%d%s%s", current_index+1,
                  file_count, !include_filename ? "" : " - ",
                  include_filename ? fullpath.c_str() : "");
    }
    else
    {
        char nicesize[10];
        humanSize((char *)fullpath.c_str(), nicesize, sizeof(nicesize)-1);

        char Zscaletxt[10];
        humanZScale(imgtkScale, Zscaletxt, sizeof(Zscaletxt)-1);

        // for the image label currently don't have filename, don't display extra dashes
        snprintf_nowarn(buff, buffsize, "%d/%d - [%dx%dx%d%s%s] - (%d%%)[%s][%s]%s%s",
                        current_index+1, file_count, w, h, depth,
                        !include_filename ? "" : " - ",
                        nicesize, outzoom, scaletxt, Zscaletxt, !include_filename ? "" : " - ",
                        include_filename ? fullpath.c_str() : "");
    }
    return buff;
}

void XBox::next_scale() {
    draw_scale = (ScaleMode)((int)draw_scale + 1);
    if (draw_scale >= ScaleModeMAX)
        draw_scale = ScaleMode::Noscale;
    _zoom_step = 0;

    updateImage();
    updateLabel();
    redraw();
}

void XBox::nextTkScale() {
    imgtkScale = (ZScaleMode)((int)imgtkScale + 1);
    if (imgtkScale >= ZScaleModeMAX)
        imgtkScale = ZScaleMode::None;

    updateImage();
    updateLabel();
    redraw();
}

void XBox::nextRotation() {
    rotation++;
    updateImage();
    updateLabel();
    redraw();
}

void XBox::updateLabel() {
    char lbl[FL_PATH_MAX+250];
    lbl[0] = 0;
    getLabel(true, lbl, sizeof(lbl));

    notifyDisplayLabel(lbl);
}

void XBox::image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg)
{
    wipeShowImage();

    if (_anim)
    {
        //Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(_img);
        _anim->stop();
        _anim->canvas(nullptr);
//        Fl_Anim_GIF_Image::animate = false;
        delete _anim;
    }

    else if (_img)
        _img->release();

    _img = newImg;
    _anim = animimg;

    // Reset all state for a new image [pending some sort of 'lock' feature]
    rotation = 0;
    _zoom = 1.0;
    _zoom_step = 0;
    deltax = 0;
    deltay = 0;

    updateImage();
}

void XBox::wipeShowImage() {
    if (_showImg && _showImg != _img) {
            _showImg->release();
    }
    _showImg = nullptr;
}

void XBox::updateImage() {

    // 1. dispose of any existing showImg because we're building a new one
    wipeShowImage();

    if (!_img || _img->w() == 0 || _img->h() == 0) {
        _showImg = nullptr;
        return;
    }

    Fl_SVG_Image *svgImg = dynamic_cast<Fl_SVG_Image *>(_img);
    if (svgImg) {
        _showImg = (Fl_RGB_Image *) svgImg->copy(); // fl_imgTk cannot rotate/scale SVG
        // TODO no rotation of SVG
    }
    else
    {
        // Covert any Fl_Pixmap into a Fl_RGB_Image so imgTk can rotate/scale
        Fl_RGB_Image *vImg;
        Fl_Pixmap *pimg = dynamic_cast<Fl_Pixmap *>(_img);
        if (!pimg) {
            vImg = (Fl_RGB_Image *) _img->copy();
        } else {
            // draw pixmap to a 32-bit image surface

            Fl_Image_Surface *imgSurf = new Fl_Image_Surface(_img->w(), _img->h());
            Fl_Surface_Device::push_current(imgSurf);
            _img->draw(0, 0);
            vImg = imgSurf->image();
            Fl_Surface_Device::pop_current();
            delete imgSurf;
        }

        // 2. rotate the original
        // TODO rotation of _anim : problematic as frames change via draw(), not here
        Fl_RGB_Image *rimg = vImg;
        if (rotation && !_anim) {
            switch (rotation) {
                case 1:
                    rimg = fl_imgtk::rotate90(vImg);
                    break;
                case 2:
                    rimg = fl_imgtk::rotate180(vImg);
                    break;
                case 3:
                    rimg = fl_imgtk::rotate270(vImg);
                    break;
                default:
                    rotation = 0;
                    break;
            }
        }
        _showImg = rimg;
    }

    // 3. scale showimage

    double basezoom = 1.0;
    bool noscale = false;

    switch (draw_scale) {
        case ScaleMode::Noscale:
            {
                // TODO is any of this necessary?
                if (_anim)
                    _anim->scale(_anim->data_w(), _anim->data_h());
                else
                    _showImg->scale(_showImg->data_w(),_showImg->data_h());
                noscale = true;
                basezoom = 1.0;
            }
            break;

        case ScaleMode::Wide:
            {
                int new_h = (int)lround((double)_showImg->h() * w() / (double)_showImg->w());
                if (_anim) {
                    _anim->scale(w(), new_h, 1, 1);
                    basezoom = (double)_anim->w() / _anim->data_w();
                }
                else {
                    _showImg->scale(w(), new_h, 1, 1);
                    basezoom = (double)_showImg->w() / _showImg->data_w();
                }
            }
            break;

        case ScaleMode::High:
            {
                int new_w = (int)lround((double)_showImg->w() * h() / (double)_showImg->h());
                if (_anim) {
                    _anim->scale(new_w, h(), 1, 1);
                    basezoom = (double)_anim->h() / _anim->data_h();
                }
                else {
                    _showImg->scale(new_w, h(), 1, 1);
                    basezoom = (double)_showImg->h() / _showImg->data_h();
                }
            }
            break;

        case ScaleMode::Fit:
            {
                if (_anim) {
                    _anim->scale(w(), h(), 1, 1);
                    basezoom = std::max( (double)_anim->w() / _anim->data_w(),
                                      (double)_anim->h() / _anim->data_h());
                }
                else {
                    _showImg->scale(w(),h(), 1, 1);
                    basezoom = std::max( (double)_showImg->w() / _showImg->data_w(),
                                      (double)_showImg->h() / _showImg->data_h());
                }
            }
            break;

        case ScaleMode::Auto:
            {
                if (_anim) {
                    _anim->scale(w(), h());
                    if (_anim->w() <= w() && _anim->h() <= h()) {
                        noscale = true;
                        basezoom = 1.0;
                    }
                    else
                    basezoom = std::max( (double)_anim->w() / _anim->data_w(),
                                      (double)_anim->h() / _anim->data_h());
                }
                else {
                    _showImg->scale(w(),h());
                    if (_showImg->data_w() <= w() && _showImg->data_h() <= h()) {
                        noscale = true;
                        basezoom = 1.0;
                    }
                    else
                    basezoom = std::max( (double)_showImg->w() / _showImg->data_w(),
                                      (double)_showImg->h() / _showImg->data_h());
                }
            }
            break;

        default:
        case ScaleMode::ScaleModeMAX:
            break;
    }

    if (_zoom_step)
    {
        // TODO change to be a lookup into a list of zoom levels
        _zoom =  basezoom + .1 * _zoom_step;
        if (_anim)
            _anim->scale(_anim->data_w()*_zoom, _anim->data_h()*_zoom,1, 1);
        else {
            _showImg->scale(_showImg->data_w() * _zoom, _showImg->data_h() * _zoom, 1, 1);
        }
    }
    else _zoom = basezoom;

    // anim is updated in draw so we're done
    if (_anim)
        return;

    if ((int) imgtkScale && !noscale && !svgImg)
    {
        // imgTK scaling is going to draw the image, so undo the pseudo-scaling from before
        int target_w = _showImg->w();
        int target_h = _showImg->h();
        _showImg->scale(_showImg->data_w(),_showImg->data_h(),0,1);
        Fl_RGB_Image *itksimg = fl_imgtk::rescale(_showImg, target_w, target_h,
                                            static_cast<fl_imgtk::rescaletype>(imgtkScale-1));

        if (itksimg != nullptr) { // CATMULL was not exposed in released fl_imgtk
            _showImg->release();
            _showImg = itksimg;
        }
    }

    size_t pixels = (size_t)_showImg->w() * _showImg->h();
    if (pixels < (INT_MAX >> 2))
    {
        // Draw the checker and image in a surface and use the surface to draw later
        Fl_Image_Surface *imgSurf = new Fl_Image_Surface(_showImg->w(), _showImg->h());
        Fl_Surface_Device::push_current(imgSurf);
        if (draw_check) {
            drawChecker(0, 0, _showImg->w(), _showImg->h());
        }
        else{
            fl_color(color()); // canvas color from preferences
            fl_rectf(0,0,_showImg->w(),_showImg->h());
        }

        _showImg->draw(0,0);
        _showImg->release();
        _showImg = imgSurf->image();
        Fl_Surface_Device::pop_current();
        delete imgSurf;
    }
}


XBox::XBox(int x, int y, int w, int h, Prefs *prefs) : SmoothResizeGroup(x,y,w,h),
    _prefs(prefs)
{
    align(FL_ALIGN_INSIDE|FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
    box(FL_BORDER_BOX);

    unsigned int bg, fg;
    _prefs->getHex(CANVAS_COLOR, bg, FL_BACKGROUND_COLOR);
    _prefs->getHex(CANVAS_LABEL_COLOR, fg, FL_FOREGROUND_COLOR);
    color(bg);
    labelcolor(fg);
    end();

    _img = nullptr;
    _anim = nullptr;
    _inSlideshow = false;

    draw_check = true;

    std::string defaultScale;
    _prefs->getS(SCALE_MODE, defaultScale, scaleModeToName(Noscale));
    draw_scale = nameToScaleMode(defaultScale);

    _prefs->getS(DITHER_MODE, defaultScale, zScaleModeToName(ZScaleMode::None));
    imgtkScale = nameToZScaleMode(defaultScale);

    _prefs->getS(OVERLAY, defaultScale, overlayModeToName(OverlayNone));
    draw_overlay = nameToOverlayMode(defaultScale);

    draw_center = false;

    deltax = 0;
    deltay = 0;
    rotation = 0;

    _mru = new MostRecentPaths(_prefs); // TODO consider singleton

    _mmoc = Fl_Color(0xFF555500); // TODO preferences
    _mmic = fl_lighter(_mmoc);    // TODO preferences
    _miniMapSize = 150; // TODO preferences?
    _minimap = true;

    file_count = 0;
    current_index = 0;
    file_list = nullptr;

    int mp;
    _prefs->get(MOUSE_PAN,mp,false); // TODO define 'bool' getter
    _pan_with_mouse = mp != 0;
    dragging = false;

    dragStartX = dragStartY = 0;
    _slideShow = nullptr;
}

void XBox::forceSlideshow() {
    _inSlideshow = false;
    toggleSlideshow();
    _slideShow->forceDelay(2);
}

void XBox::toggleSlideshow() {
    _inSlideshow = !_inSlideshow;
    if (_inSlideshow) {
        _slideShow = new Slideshow();
        _slideShow->setPrefs(_prefs);
        _slideShow->setWindow(this);
        _slideShow->start(current_index);
    }
    else {
        _slideShow->stop();
        delete _slideShow;
        _slideShow = nullptr;
    }

}

void XBox::toggleMinimap() {
    _minimap = !_minimap;
    redraw();
}

void XBox::toggleOverlay() {
    draw_overlay = nextOverlay(draw_overlay);
    switch (draw_overlay)
    {
        case OverlayBox:
            notifyActivate(true);
            break;
        default:
            notifyActivate(false);
            break;
    }
    redraw();
}

Fl_Color XBox::_mmoc;
Fl_Color XBox::_mmic;
int XBox::_miniMapSize;

void XBox::drawMinimap() {
    if (!_minimap || (!_showImg && !_anim)) return; // minimap off, no image

    int iw = _anim ? _anim->w() : _showImg->w();
    int ih = _anim ? _anim->h() : _showImg->h();
    int ww = w();
    int wh = h();

    if (iw <= ww && ih <= wh) return; // image fits inside window, no map necessary

    // Size the outer rectangle proportional to the image
    int mmw = _miniMapSize;
    int mmh = _miniMapSize;
    if (iw < ih)
        mmw = (int)(_miniMapSize * (double)iw / (double)ih);
    else
        mmh = (int)(_miniMapSize * (double)ih / (double)iw);

    int mmx = x() + ww - mmw - 2; // TODO options: where mmap is located
    int mmy = y() + 2;
    fl_rect(mmx, mmy, mmw, mmh, _mmoc); // minimap outer

    // draw the inner rect. NOTE may be positioned outside the inner rect depending on scrolling!
    int mmix = (int)((double)-deltax / (double)iw * (double)mmw) + 1;
    int mmiy = (int)((double)-deltay / (double)ih * (double)mmh) + 1;
    int mmiw = std::min((int)((double)w() / (double)iw * (double)mmw), mmw-2);
    int mmih = std::min((int)((double)h() / (double)ih * (double)mmh), mmh-2);

    fl_rect(mmx + mmix, mmy + mmiy, mmiw, mmih, _mmic); // minimap inner
}

void XBox::draw() {
    Fl_Group::draw();

    if ((!_showImg || !_showImg->w() || !_showImg->h()) && !_anim) {
        drawOverlay();
        return;
    }

    fl_push_clip( x(), y(), w(), h() );

    // Note: offset to not overwrite the box outline
    int drawx = x()+1;
    int drawy = y()+1;

    if (draw_center) {
        int iw;
        int ih;
        if (_anim) {iw = _anim->w(); ih = _anim->h();}
        else {iw = _showImg->w(); ih = _showImg->h();}

        deltax = std::max(1, (w() - iw) / 2);
        deltay = std::max(1, (h() - ih) / 2);
    }

    if (_anim && draw_check)
    {
        // animation frames currently update here, not in updateImage()
        int outw = std::min(w(), _anim->w());
        int outh = std::min(h(), _anim->h());
        drawChecker(drawx + deltax, drawy + deltay, outw-2, outh-2);
    }

    if (_anim) {
        // NOTE: this assumes the _anim scale has been set in updateImage()
        Fl_Image* tmp = _anim->image();
        //printf("Draw Anim %d,%d | %d,%d \n", tmp->w(),tmp->h(), tmp->data_w(), tmp->data_h());
        // TODO for some reason the frame scale() isn't "sticking"???
        // TODO 4009.webp gets this far then crashes because image() returns null
        if (tmp)
            tmp->scale(_anim->w(), _anim->h(), 1, 1);
        _anim->draw(drawx, drawy, w() - 2, h() - 2, -deltax, -deltay);
    }
    else {
        _showImg->draw(drawx, drawy, w() - 2, h() - 2, -deltax, -deltay);
    }

    drawMinimap();
    drawOverlay();

    fl_pop_clip();
}

void XBox::drawOverlay() {

    if (!file_count || !draw_overlay)
        return;

    char hack[501];
    char *l = getLabel(false, hack, 500);

    if (draw_overlay == OverlayText) {
        label(l);
        labelsize(20);
        labelcolor(FL_DARK_GREEN);    // TODO options
        labeltype(FL_EMBOSSED_LABEL); // TODO options
        align(FL_ALIGN_BOTTOM_RIGHT); // TODO options

        if (label()) {
            int lw = 0; // valgrind uninit data
            int lh = 0;
            measure_label(lw, lh);
            fl_font(labelfont(), labelsize());
            fl_color(labelcolor());
            draw_label(x() + w() - lw - 2, y() + h() - lh - 2, lw, lh, align()); // TODO options
        }
        label(""); // TODO prevent extra label draw?
    }

    if (draw_overlay == OverlayBox)
        notifyDisplayInfo(l);
}

void XBox::forceScale(const char *val)
{
    ScaleMode res = nameToScaleMode(val);
    draw_scale = res;
}

void XBox::forceDither(const char *val)
{
    ZScaleMode res = nameToZScaleMode(val);
    imgtkScale = res;
}

void XBox::notifyActivate(bool val) {
    if (_dispevents)
        for (XBoxDisplayInfoEvent *e : *_dispevents)
            e->OnActivate(val);
}
void XBox::notifyBorder() {
    if (_dispevents)
        for (XBoxDisplayInfoEvent *e : *_dispevents)
            e->OnBorder();
}
void XBox::notifyDisplayInfo(const char *val) {
    if (_dispevents)
        for (XBoxDisplayInfoEvent *e : *_dispevents)
            e->OnDisplayInfo(val);
}
void XBox::notifyDisplayLabel(const char *val) {
    if (_dispevents)
        for (XBoxDisplayInfoEvent *e : *_dispevents)
            e->OnDisplayLabel(val);
}

void XBox::safe_resize() {
    updateImage();
    redraw();
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif