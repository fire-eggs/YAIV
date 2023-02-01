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

#include "list_rand.h" // TODO loadFile
#include "fl_imgtk.h"
#include "Slideshow.h"
#include "XBoxDisplayInfoEvent.h"
#include "mediator.h" // TODO define messages elsewhere?
#include "filelist.h"
#include <assert.h>
#include <unistd.h> // sleep
#include <future> // std::future
#include <chrono> // chrono literal
using namespace std::chrono_literals;

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
    if (f) {
        fprintf(f, format, arg);
        fputs("\n", f);
        fclose(f);
    }
}

filelist *box_filelist; // TODO member

// image dimensions, pixels
int iw;
int ih;
// image dimensions, zoomed
int zoomIw;
int zoomIh;

void XBox::load_file(const char *n) {
    box_filelist = filelist::initFilelist(n);

    /* this is always "failure" because the file scanner hasn't run yet
    const char *fullpath = box_filelist->getCurrentFilePath();   
    if (!fullpath) // load fail. don't update MRU. 
    {
        timeoutCallback(this, "Failure");
        return; 
    }
    */
    
    // don't add to MRU if unsuccessful load
    if (!box_filelist || box_filelist->oldFileCount() < 1)
    {
        _mru->Remove(n);
        showUserMessage("Path is missing");
        clearUserMesssageLater();
        return; 
    }
    
    // Update the MRU list
    clearUserMessage();
    _mru->Add(n);
    _mru->Save();
    
    Fl::wait(0.5); // Give the filescanner thread a chance to load file(s)
    load_current();
}

void XBox::drawMessage()
{
    // Draw a message to present to the user.
    // TODO consider replacing with 'toast' style messaging
    
    if (_message[0] == '\0') // No message
        return;
    
    XBox *xb = this;

    Fl_Label *lbl = new Fl_Label();
    
    lbl->value = _message;
    lbl->size = 22;
    lbl->color = FL_GREEN;
    lbl->align_ = FL_ALIGN_CENTER;   

    int mx = xb->x();
    int mw = xb->w();
    int my = xb->y();
    int mh = xb->h();
    
    {
        int lw=0;
        int lh=0;
        lbl->measure(lw, lh);
        fl_draw_box(FL_FLAT_BOX, 
                    mx + mw/2 - lw/2 - 2,
                    my + mh/2 - lh / 2 - 2,
                    lw + 4, lh + 4,
                    FL_BLACK
                   );
        lbl->draw(mx + mw/2 - lw / 2,
                    my + mh/2 - lh / 2,
                    lw, lh, FL_ALIGN_CENTER);
    }
    delete lbl;
    
}

void XBox::showUserMessage(const char *msg)
{
    // Establish a message to show to the user on next redraw
    strcpy(_message, msg);
    redraw();
    Fl::wait();
}

void XBox::clearUserMessage()
{
    // Clear any existing user message, is wiped on next redraw
    _message[0] = '\0';
    // NOTE: this is not immediate, waits until next redraw
}

void XBox::clearUserMesssageLater()
{
    // Wipe the user message in two seconds via timeout
    Fl::add_timeout(2.0, cb_UserMsgTimeout, this);
}

void XBox::cb_UserMsgTimeout(void *data)
{
    // Clear an existing user message after timeout
    XBox *xb = static_cast<XBox *>(data);
    xb->showUserMessage("");
    Fl::remove_timeout(cb_UserMsgTimeout);
}

void XBox::load_current() {

    // Update anything which needs to know if the user can go forward/back
    Mediator::send_message(Mediator::MSGS::MSG_TB,
                           box_filelist->canPrev() ? Mediator::ACT_ISPREV
                                                   : Mediator::ACT_NOPREV);
    Mediator::send_message(Mediator::MSGS::MSG_TB,
                           box_filelist->canNext() ? Mediator::ACT_ISNEXT
                                                   : Mediator::ACT_NONEXT);
    
    const char *fullpath = box_filelist->getCurrentFilePath();   
    if (!fullpath) 
        return; // Nothing to do

    logit("Load %s", (char *)fullpath);

    rotation = 0; // TODO anything else need resetting?

    if (fl_filename_isdir(fullpath)) // TODO shouldn't be happening any more?
    {
        align(FL_ALIGN_CENTER);
        label("@fileopen"); // show a generic folder
        labelsize(64);
        labelcolor(FL_LIGHT2);
        image(nullptr,nullptr);

        send_message(Mediator::MSGS::MSG_NEWFILE, -1);
    }
    else if (box_filelist->ishidden())
    {
        align(FL_ALIGN_CENTER);
        label("@filenew");
        labelsize(64);
        labelcolor(FL_BLACK);
        image(nullptr,nullptr);

        send_message(Mediator::MSGS::MSG_NEWFILE, 0);
    }
    else
    {        
        // Load image async. If it takes longer than 0.75 seconds, show a message
        std::future<Fl_Image *> future = std::async(std::launch::async, loadFile, (char*)fullpath, this);
        std::future_status status = future.wait_for(std::chrono::milliseconds(750));
        if (status != std::future_status::ready)
        {
            showUserMessage("Loading...");
        }
        Fl_Image *img = future.get();
        clearUserMessage();
        
        // 590B-01.jpg failed to load and resulted in crash further on
        if (!img || img->fail() || img->w() == 0 || img->h() == 0)
        {
            // failed to load - show a red file placeholder
            align(FL_ALIGN_CENTER);
            label("@filenew");
            labelsize(64);
            labelcolor(FL_RED);
            image(nullptr,nullptr);

            send_message(Mediator::MSGS::MSG_NEWFILE, -1);
        }
        else
        {
            //Fl_Anim_GIF_Image::min_delay = 0.01;
            // KBR 20220528 undocumented 'standard': minimum GIF playback is 0.2 seconds, see Netscape history
            Fl_Anim_GIF_Image::min_delay = 0.02;
            Fl_Anim_GIF_Image::animate = true;

            send_message(Mediator::MSGS::MSG_NEWFILE, 0);

            label(nullptr); // wipe any previous folder/error-file
            Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(img);
            image(img, animgif);
        }
    }

    redraw();
    // Update the title bar
    updateLabel(); // TODO issue #98: on error, provide a message. need to expose something in list_rand.cpp ... Nice for overlay also.
    
    // TODO do this again - action was run before toolbar status update
    // Update anything which needs to know if the user can go forward/back
    Mediator::send_message(Mediator::MSGS::MSG_TB,
                           box_filelist->canPrev() ? Mediator::ACT_ISPREV
                                                   : Mediator::ACT_NOPREV);
    Mediator::send_message(Mediator::MSGS::MSG_TB,
                           box_filelist->canNext() ? Mediator::ACT_ISNEXT
                                                   : Mediator::ACT_NONEXT);
    
}

void XBox::next_image() {
    if (!box_filelist) // TODO initialize at start
        return;
    // 20220530 Don't reset the visible image if already at the end
    if (box_filelist->canNext())
    {
        box_filelist->next();
        load_current();
    }
    else
    {
        if (_quitAtEnd) // NOTE: command-line option
            exit(0);
    }
}

void XBox::prev_image() {
    if (!box_filelist) // TODO initialize at start
        return;
    // 20220530 don't reset the visible image if already at the beginning
    if (box_filelist->canPrev())
    {
        box_filelist->prev();
        load_current();
    }
}

void XBox::action(int act)
{
    // TODO turn this into a map of act -> method where each method is void()
    
    switch (act)
    {
        case Mediator::ACT_NEXT:
            // User has manually changed image. Make sure the slideshow
            // shows said image for the specified time.
            if (_inSlideshow)
                _slideShow->resetTimer();
            next_image();
            break;
        case Mediator::ACT_PREV:
            // User has manually changed image. Make sure the slideshow
            // shows said image for the specified time.
            if (_inSlideshow)
                _slideShow->resetTimer();
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
            if (_inSlideshow)
                _slideShow->resetTimer();
            break;
        case Mediator::ACT_SLID:
            toggleSlideshow();
            break;
        case Mediator::ACT_SCALE_NONE:
        case Mediator::ACT_SCALE_AUTO:
        case Mediator::ACT_SCALE_WIDE:
        case Mediator::ACT_SCALE_HIGH:
        case Mediator::ACT_SCALE_FIT:
            draw_scale = (ScaleMode)((int)act - (int)Mediator::ACT_SCALE_NONE); // TODO super hack
            _zoom_step = 0;
            updateImage();
            updateLabel();
            redraw();
            break;

        case Mediator::ACT_SCROLLUP:
            centerY += _scroll_speed;
            redraw();
            break;
        case Mediator::ACT_SCROLLDOWN:
            centerY -= _scroll_speed;
            redraw();
            break;
        case Mediator::ACT_SCROLLLEFT:
            centerX += _scroll_speed;
            redraw();
            break;
        case Mediator::ACT_SCROLLRIGHT:
            centerX -= _scroll_speed;
            redraw();
            break;

        case Mediator::ACT_SCALE:
            next_scale();
            break;
        case Mediator::ACT_OVERLAY:
            toggleOverlay();
            break;
        case Mediator::ACT_RANDOM:
            box_filelist->randomize();
            load_current();
            if (_inSlideshow)
                _slideShow->resetTimer();
            break;
        case Mediator::ACT_DITHER:
            nextTkScale();
            break;
        case Mediator::ACT_MINIMAP:
            toggleMinimap();
            break;
        case Mediator::ACT_BORDER:
            notifyBorder();
            break;
        case Mediator::ACT_MOUSEPAN:
            _pan_with_mouse = !_pan_with_mouse;
            _prefs->set2(MOUSE_PAN, _pan_with_mouse);
            break;

        case Mediator::ACT_HOME:
            if (box_filelist) // TODO initialize 
            {
                box_filelist->home();
                load_current();
                if (_inSlideshow)
                    _slideShow->resetTimer();
            }
            break;
        case Mediator::ACT_END:
            if (box_filelist) // TODO initialize
            {
                box_filelist->end();
                load_current();
                if (_inSlideshow)
                    _slideShow->resetTimer();
            }
            break;
            
        default:
            return;
    }
}

int XBox::handle(int msg) 
{
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
            // TODO confirm if windows drag-and-drop includes the "file://" prefix!
            const char *urls = Fl::event_text();
#ifndef _WINDOWS
            if (strncmp(urls, "file://", 7) != 0)
                return 1; // not a local file, do nothing

            size_t len = strlen(urls);
            size_t i = 0;
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
                do_menu(Fl::event_x(), Fl::event_y(), true);
                return 1;
            }
            
            if (Fl::event_clicks() > 0)
            {
                // User has double-clicked in the box. Attempt to zoom in with that location
                // at the center.
                int inboxX = Fl::event_x() - x(); // click x,y relative to box
                int inboxY = Fl::event_y() - y();
                int boxCntX = w() / 2;            // center of the box
                int boxCntY = h() / 2;

                // NOTE: _not_ worrying about clicking outside the image! 
                
                // Move the image location so the spot the user double-clicked on 
                // is now in the center of the box.
                // NOTE: this is very approximate and shifts a bit from the zoom as well.
                centerX += (boxCntX - inboxX);
                centerY += (boxCntY - inboxY);
                
                change_zoom(+1);
                redraw();
                return 1;
            }
            else               
                return mousePan(FL_PUSH);
            break;
        case FL_DRAG:
        case FL_RELEASE:
            return mousePan(msg);
            break;
        default:
            break;
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
            dragStartX = mouseX - centerX;
            dragStartY = mouseY - centerY;

            dragging = true;
            break;
        case FL_DRAG: {
            if (!dragging) return 0;

            int movX = dragStartX - mouseX;
            int movY = dragStartY - mouseY;
            fl_cursor(FL_CURSOR_MOVE);

            centerX = -movX;
            centerY = -movY;
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

const char * XBox::getLabel(bool include_filename, char *buff, int buffsize)
{
    if (!box_filelist)
        return "";

    std::string fullpath;
    bool filenameonly = true; // TODO customize label : full path or just filename
    if (filenameonly)  // TODO driven by prefs
        fullpath = box_filelist->currentFilename();
    else
        fullpath = box_filelist->getCurrentFilePath();

    int current_index = box_filelist->currentIndex() + 1;
    int file_count = box_filelist->fileCount();

    int outzoom = (int)(_zoom * 100.0 + 0.5);
    int w = _img ? _img->w() : 0;
    int h = _img ? _img->h() : 0;
    int depth = _img ? _img->d() : 0;

    char scaletxt[10];
    char * res = humanScale(draw_scale, scaletxt, sizeof(scaletxt)-1);

    if (res == nullptr || _img == nullptr)
    {
        // for the image label currently don't have filename, don't display extra dashes
        snprintf( buff, buffsize, "%d/%d%s%s", current_index,
                  file_count, !include_filename ? "" : " - ",
                  include_filename ? fullpath.c_str() : "");
    }
    else
    {
        char nicesize[10];
        humanSize(box_filelist->getCurrentFilePath(), nicesize, sizeof(nicesize)-1);

        char Zscaletxt[10];
        humanZScale(imgtkScale, Zscaletxt, sizeof(Zscaletxt)-1);

        // for the image label currently don't have filename, don't display extra dashes
        snprintf_nowarn(buff, buffsize, "%d/%d - [%dx%dx%d%s%s] - (%d%%)[%s][%s]%s%s",
                        current_index, file_count, w, h, depth,
                        !include_filename ? "" : " - ",
                        nicesize, outzoom, scaletxt, Zscaletxt, !include_filename ? "" : " - ",
                        include_filename ? fullpath.c_str() : "");
    }
    return buff;
}

void XBox::next_scale() {
    int next_scale = (ScaleMode)((int)draw_scale + 1);
    if (next_scale >= ScaleModeMAX)
        next_scale = ScaleMode::Noscale;
    Mediator::ACTIONS act = static_cast<Mediator::ACTIONS>((int)next_scale + (int)Mediator::ACTIONS::ACT_SCALE_NONE);
    send_message(Mediator::MSG_TB, act);
/*
    draw_scale = (ScaleMode)((int)draw_scale + 1);
    if (draw_scale >= ScaleModeMAX)
        draw_scale = ScaleMode::Noscale;
    _zoom_step = 0;

    updateImage();
    updateLabel();
    redraw();
*/
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
    char lbl[FL_PATH_MAX+250]; // TODO fix hack
    lbl[0] = 0;
    getLabel(true, lbl, sizeof(lbl));

    notifyDisplayLabel(lbl);
}

void XBox::updateDisplay() {
    char hack[501];
    const char *l = getLabel(false, hack, 500);  // TODO fix hack
    notifyDisplayInfo(l);
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
    centerX = centerY = INT_MAX; // On next draw, center to window
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

    Fl_RGB_Image *vImg;
    Fl_SVG_Image *svgImg = dynamic_cast<Fl_SVG_Image *>(_img);
    if (svgImg) {

        Fl_SVG_Image *tImg = (Fl_SVG_Image *)svgImg->copy();
        // force rasturization
        tImg->resize(_img->w(), _img->h());
        vImg = tImg;
    }
    else
    {
        // Covert any Fl_Pixmap into a Fl_RGB_Image so imgTk can rotate/scale
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
    }

    // Rotation
    Fl_RGB_Image *rimg = vImg;
    // TODO rotation of _anim : problematic as frames change via draw(), not here
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
    // ?? vImg->release();
    _showImg = rimg;



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
        double new_zoom =  basezoom + .1 * _zoom_step;

        // prevent crash when attempting to zoom below zero
        if (new_zoom > 0)  { _zoom = new_zoom; } else {_zoom_step++;}
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

#if false // TODO keep or toss?
    unsigned int bg, fg;
    _prefs->getHex(CANVAS_COLOR, bg, FL_BACKGROUND_COLOR);
    _prefs->getHex(CANVAS_LABEL_COLOR, fg, FL_FOREGROUND_COLOR);
    color(bg);
    labelcolor(fg);
#endif

    end();

    _img = nullptr;
    _anim = nullptr;
    _inSlideshow = false;

    // TODO initialize to false so toolbar init (see main()) will toggle ON
    draw_check = false;

    std::string defaultScale;
    _prefs->getS(SCALE_MODE, defaultScale, scaleModeToName(Noscale));
    draw_scale = nameToScaleMode(defaultScale);

    _prefs->getS(DITHER_MODE, defaultScale, zScaleModeToName(ZScaleMode::None));
    imgtkScale = nameToZScaleMode(defaultScale);

    _prefs->getS(OVERLAY, defaultScale, overlayModeToName(OverlayNone));
    draw_overlay = nameToOverlayMode(defaultScale);

    rotation = 0;

    _mru = new MostRecentPaths(_prefs); // TODO consider singleton

    _mmoc = Fl_Color(0xFF555500); // TODO preferences
    _mmic = fl_lighter(_mmoc);    // TODO preferences
    _miniMapSize = 150; // TODO preferences?
    _minimap = true;

    int mp;
    _prefs->get(MOUSE_PAN,mp,false); // TODO define 'bool' getter
    _pan_with_mouse = mp != 0;
    dragging = false;

    dragStartX = dragStartY = 0;
    _slideShow = nullptr;
    clearUserMessage();
}

void XBox::forceSlideshow() {
    // user specified '-s' on command line
    _inSlideshow = false;
    toggleSlideshow();
    _slideShow->forceDelay(2);
}

void XBox::toggleSlideshow() {
	if (box_filelist && box_filelist->any())
	{
		_inSlideshow = !_inSlideshow;
		if (_inSlideshow) {
			_slideShow = new Slideshow();
			_slideShow->setPrefs(_prefs);
			_slideShow->setWindow(this);
			_slideShow->start(box_filelist->currentIndex()); // TODO slideshow uses filelist
		}
		else {
			_slideShow->stop();
			delete _slideShow;
			_slideShow = nullptr;
		}
	}
	else
        _inSlideshow = false; // Nothing to show
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
            // TODO prevent the overlay box appearing when no images in list
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

void XBox::drawCenter()
{
    return; // TODO center marker as option?
    
    int ww = w();
    int wh = h();
    int cx = x() + ww / 2;
    int cy = y() + wh / 2;

    auto oldclr = fl_color();
    fl_color(FL_RED);
    fl_xyline(cx-10, cy, cx+10);
    fl_yxline(cx, cy-10, cy+10);
    fl_color(oldclr);
}

void XBox::drawMinimap() {
    if (!_minimap || (!_showImg && !_anim)) return; // minimap off, no image

    int iw = _anim ? _anim->w() : _showImg->w();
    int ih = _anim ? _anim->h() : _showImg->h();
    int ww = w();

    // TODO if option "hide minimap if smaller" is created, need to take panning into account.
    //int wh = h();
    //if (iw <= ww && ih <= wh) return; // image fits inside window, no map necessary

    // Size the 'image' rectangle proportional to the image
    int mmw = _miniMapSize;
    int mmh = _miniMapSize;
    if (iw < ih)
        mmw = (int)(_miniMapSize * (double)iw / (double)ih);
    else
        mmh = (int)(_miniMapSize * (double)ih / (double)iw);

    int mmap_dy = 20; // 2 // TODO options: mmap location
    int mmap_dx = ww - mmw - 20; //2;

    int mmx = x() + mmap_dx;
    int mmy = y() + mmap_dy;
    fl_rect(mmx, mmy, mmw, mmh, _mmoc); // minimap 'image' rectangle

    // draw the 'window' rect. NOTE may be positioned outside the 'image' rect
    // depending on scrolling, zooming, center, etc.
    int mmix;
    int mmiy;
    {
        int dx = centerX - iw / 2;
        int dy = centerY - ih / 2;
        mmix = (int) ((double) -dx / (double) iw * (double) mmw) + 1;
        mmiy = (int) ((double) -dy / (double) ih * (double) mmh) + 1;
    }
    int mmiw = (int)((double)w() / (double)iw * (double)mmw);
    int mmih = (int)((double)h() / (double)ih * (double)mmh);

    _mmic = Fl_Color (FL_BLUE); //debugging
    fl_rect(mmx + mmix, mmy + mmiy, mmiw, mmih, _mmic); // minimap inner
}

void XBox::draw() {
    Fl_Group::draw();

    if ((!_showImg || !_showImg->w() || !_showImg->h()) && !_anim) {
        drawMessage();
        drawOverlay();
        return;
    }

    fl_push_clip( x(), y(), w(), h() );

    // Determine imagewidth / imageheight for future use
    if (_anim) {iw = _anim->w(); ih = _anim->h();}
    else {iw = _showImg->w(); ih = _showImg->h();}

    // New image, force to center of window
    if (centerX == INT_MAX)
    {
        centerX = w() / 2;
        centerY = h() / 2;
    }

    zoomIh = ih * _zoom;
    zoomIw = iw * _zoom;
    
    //printf("Draw:Center (%d,%d)\n", centerX, centerY);
    
    // The upper left X,Y; includes small offset to not draw over box outline
    int drawx = x() + 1 + centerX - iw / 2;
    int drawy = y() + 1 + centerY - ih / 2;

    if (_anim && draw_check)
    {
        // animation frames currently update here, not in updateImage()
        int outw = std::min(w(), _anim->w());
        int outh = std::min(h(), _anim->h());
        drawChecker(drawx, drawy, outw-2, outh-2);
    }

    if (_anim) {
        // NOTE: this assumes the _anim scale has been set in updateImage()
        Fl_Image* tmp = _anim->image();
        //printf("Draw Anim %d,%d | %d,%d \n", tmp->w(),tmp->h(), tmp->data_w(), tmp->data_h());
        // TODO for some reason the frame scale() isn't "sticking"???
        // TODO 4009.webp gets this far then crashes because image() returns null
        if (tmp)
            tmp->scale(iw, ih, 1, 1);
        _anim->draw(drawx, drawy, iw, ih);
    }
    else {
        _showImg->draw(drawx, drawy, iw-2, ih-2);
    }

    drawCenter();
    drawMinimap();
    drawOverlay();
    drawMessage();

    fl_pop_clip();
}

Fl_Image *getImage(const char *imgn)
{
    char buff[500];
    sprintf(buff, "icons/%s.svg", imgn); // TODO find sub-folder? hard-coded in code?

    Fl_SVG_Image *img = new Fl_SVG_Image(buff,0);
    if (img->fail()) {
        img->release();
        return nullptr;
    }
    return img;
}

void XBox::drawOverlay() {

    if ( !box_filelist || !box_filelist->any() || !draw_overlay)
        return;

    char hack[501];
    const char *l = getLabel(false, hack, 500);

    if (draw_overlay == OverlayText) {

        Fl_Label *lbl = new Fl_Label();
        lbl->value = l;
        //char imgnm[10];
        Fl_Image *img = nullptr;
        if (box_filelist->isFav())
            img = getImage("heart");
        else if (box_filelist->isHide())
            img = getImage("hide");
        if (img) {
            lbl->image = img->copy(20, 20);
            img->release();
        }
        lbl->size = 20;
        lbl->color = FL_DARK_GREEN;
        //lbl->type = FL_EMBOSSED_LABEL; // can't use with image
        lbl->align_ = FL_ALIGN_CENTER|FL_ALIGN_IMAGE_NEXT_TO_TEXT;

        {
            int lw=0;
            int lh=0;
            lbl->measure(lw, lh);
            lbl->draw(x() + w() - lw - 10, y()+h()-lh-5, lw, lh,
                      FL_ALIGN_BOTTOM_RIGHT|FL_ALIGN_IMAGE_NEXT_TO_TEXT);
        }
        delete lbl;
/*
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
*/
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
    centerX = centerY = INT_MAX;
    updateImage();
    updateLabel(); // resize may have impacted zoom, etc
    redraw();
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
