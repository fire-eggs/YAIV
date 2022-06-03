//
// Created by kevin on 6/5/21.
//

#include "yaiv_win.h"
#include "toolbar/toolgrp.h"
#include "mediator.h"

YaivWin* makeMainWindow()
{
    int x, y, w, h;
    Prefs *prefs = new Prefs();
    prefs->getWinRect(MAIN_PREFIX, x, y, w, h);

    return new YaivWin(x,y,w,h, prefs);
}

#define TB_HEIGHT 38 // TODO tb hack

static YaivWin *win_main;

static void cb_Exit(Fl_Button *, void *)
{
    win_main->hide();
    toolgrp::hide_all();
#ifdef DANBOORU
    shutdown_danbooru();
#endif
}

YaivWin::YaivWin(int x, int y, int _w, int h, Prefs* prefs) : dropwin(x,y,_w,h),
                                       _border(1), _prefs(prefs), _xoff(0), _yoff(0)
{
    // NOTE do _not_ toggle the border in here. Prevents minimize/restore.

#ifdef __linux__
    // some Linux window manager cannot handle resize by thin window border.
    border( 2 );
#endif

    // Colors from preferences so RaphK can have dark and I can have light :)
    unsigned int fg, bg;
    _prefs->getHex(MAIN_COLOR, bg, FL_BACKGROUND_COLOR);
    _prefs->getHex(MAIN_LABEL_COLOR, fg, FL_FOREGROUND_COLOR);
    color(bg);
    labelcolor(fg);

    callback((Fl_Callback*)cb_Exit);
    win_main = this;
    
    xclass("yaivmain"); // useful on KDE Plasma
}

void YaivWin::resize(int x, int y, int w, int h) {
    dropwin::resize(x,y,w,h);
    _prefs->setWinRect(MAIN_PREFIX, x, y, w, h);
}

void YaivWin::toggle_border() {
    _border = !_border;
    border(_border);
    _prefs->set2(BORDER_FLAG, _border);
}

void YaivWin::push() {
    _xoff = x() - Fl::event_x_root();
    _yoff = y() - Fl::event_y_root();
}

void YaivWin::drag() {
    position(_xoff + Fl::event_x_root(), _yoff + Fl::event_y_root());
    redraw();
}


int YaivWin::handle(int e)
{
    int ret = dropwin::handle(e);

    if (e == FL_FOCUS) return 1;
    if (e == FL_UNFOCUS) return 1;

    if (e == FL_KEYDOWN)
    {
        //printf("YW: key %d (%d)\n", Fl::event_key(), ret);
        Mediator::handle_key();
        return 1;
    }


    switch (e)
    {
        case FL_PUSH: push(); ret=1; break;

        case FL_DRAG: drag(); ret=1; break;

//        case FL_FOCUS:
//            _child->take_focus();
//            break;
//        case FL_UNFOCUS:
//            ret = 1;
//            break;

        case FL_SHOW:
            {
            // cannot initialize border state until window has actually been shown
            int val;
            _prefs->get(BORDER_FLAG, val, true);
            _border = !val;
            toggle_border();
            }
            break;

        default: break;
    }
    return ret;
}
