//
// Created by kevin on 6/5/21.
//

#ifndef YAIV_YAIV_WIN_H
#define YAIV_YAIV_WIN_H

#include <string> // strncpy

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/filename.H>

#include "prefs.h"
#include "XBox.h"

#if (FL_MINOR_VERSION<4)
#error "Error, required FLTK 1.4 or later"
#endif

#ifdef DANBOORU
#include "danbooru.h"
#endif

class YaivWin : public Fl_Double_Window {
private:
    bool _border;
    XBox* _child;
    Prefs *_prefs;

    // dragging borderless window w/mouse
    int _xoff;
    int _yoff;


    char filename[FL_PATH_MAX];

public:
    YaivWin(int, int, int, int, Prefs*);

    void child(XBox *canvas) {_child = canvas;}

    void setFilename(char *fn) { strncpy(filename, fn, FL_PATH_MAX); }

    Prefs* prefs() {return _prefs;}

    int handle(int) override;
    void toggle_border();
    void resize(int,int,int,int) override;
    void push();
    void drag();
    void updateLabel();
};

YaivWin* makeMainWindow();

#endif //YAIV_YAIV_WIN_H