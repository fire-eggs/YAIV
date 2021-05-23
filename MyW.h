//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_MYW_H
#define CLION_TEST2_MYW_H

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/filename.H>
#include <prefs.h>

#ifdef DANBOORU
#include "danbooru.h"
#endif

extern Prefs *_prefs;

class MyW : public Fl_Double_Window
{
private:
    bool _border;
    int _xoff;
    int _yoff;
    Fl_Widget *_child;

public:
    MyW(int, int, int, int);

    void child(Fl_Widget *c) { _child =c; }

    char filename[FL_PATH_MAX];

    void updateLabel();

    void toggle_border();

    void push()
    {
        _xoff = x() - Fl::event_x_root();
        _yoff = y() - Fl::event_y_root();
    }
    void drag()
    {
        position(_xoff + Fl::event_x_root(), _yoff+Fl::event_y_root());
        redraw();
    }
    void rel()
    {
        // show();
    }

    void resize(int x, int y, int w, int h) override;

    int handle(int e)
    {
        int ret;
        ret = Fl_Double_Window::handle(e);
        switch (e)
        {
            case FL_PUSH: push(); ret=1; break;

            case FL_DRAG: drag(); ret=1; break;

            case FL_RELEASE: rel(); ret=1; break;

            case FL_FOCUS:
                _child->take_focus();
                break;
            case FL_UNFOCUS:
                ret = 1;
                break;

            case FL_KEYDOWN:
                //printf("Win: keydown, state:%d\n", Fl::event_state());
                ret = 0;
                break;

            case FL_SHOW: {
                // cannot initialize border state until window has actually been shown
                int val;
                ::_prefs->get("BORDER", val, true);
                _border = !val;
                toggle_border();
            }
            break;

#ifdef DANBOORU
            case FL_HIDE: {
                // shutting down
                shutdown_danbooru();
            }
            ret = 0;
            break;
#endif

            default: break;
        }
        return ret;
    }
};

#endif //CLION_TEST2_MYW_H
