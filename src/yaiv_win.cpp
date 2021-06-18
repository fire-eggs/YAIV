//
// Created by kevin on 6/5/21.
//

#include "yaiv_win.h"

YaivWin* makeMainWindow()
{
    int x, y, w, h;
    Prefs *prefs = new Prefs();
    prefs->getWinRect(MAIN_PREFIX, x, y, w, h);

    return new YaivWin(x,y,w,h, prefs);
}

YaivWin::YaivWin(int x, int y, int w, int h, Prefs* prefs) : Fl_Double_Window(x,y,w,h),
                                       _xoff(0), _yoff(0), _border(1), _prefs(prefs)
{
    // NOTE do _not_ toggle the border in here. Prevents minimize/restore.

#ifdef __linux__
    // some Linux window manager cannot handle resize by thin window border.
    border( 2 );
#endif

    // Colors from preferences so RaphK can have dark and I can have light :)
    int fg, bg;
    _prefs->get(MAIN_COLOR, bg, FL_BACKGROUND_COLOR);
    _prefs->get(MAIN_LABEL_COLOR, fg, FL_FOREGROUND_COLOR);
    color(bg);
    labelcolor(fg);
    _child = nullptr;
}

void YaivWin::updateLabel() {
    char lbl[FL_PATH_MAX+250];
    lbl[0] = 0;
    label( _child->getLabel(true, lbl, sizeof(lbl)));
}

void YaivWin::resize(int x, int y, int w, int h) {
    Fl_Double_Window::resize(x,y,w,h);
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
    int ret = Fl_Double_Window::handle(e);
    switch (e)
    {
        case FL_PUSH: push(); ret=1; break;

        case FL_DRAG: drag(); ret=1; break;

        case FL_FOCUS:
            _child->take_focus();
            break;
        case FL_UNFOCUS:
            ret = 1;
            break;

        case FL_SHOW:
            {
            // cannot initialize border state until window has actually been shown
            int val;
            _prefs->get(BORDER_FLAG, val, true);
            _border = !val;
            toggle_border();
            }
            break;

#ifdef DANBOORU
        case FL_HIDE:
            // shutting down
            shutdown_danbooru();
            ret = 0;
            break;
#endif

        default: break;
    }
    return ret;
}

