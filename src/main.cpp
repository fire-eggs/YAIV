#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

#include <clocale>     // setlocale()..

#include "yaiv_win.h"
#include "Fl_TransBox.h"
#include "XBoxDisplayInfoEvent.h"
#include "toolbar/toolgrp.h"
#include "buttonBar.h"
#include "textbar.h"
#include "mediator.h"

void cmdline(int argc, char **argv, XBox *box)
{
    if (argc < 2 || !argv)
        return;
    for (int i=1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-w"))
            box->forceSlideshow();
        else if (!strcmp(argv[i], "-q"))
            box->forceQuitAtEnd();
        else if (!strncmp(argv[i], "-s:", 3))
            box->forceScale(&(argv[i][3]));
        else if (!strncmp(argv[i], "-d:", 3))
            box->forceDither(&(argv[i][3]));
        else
            box->load_file(argv[i]);
    }
}

// TODO nasty globals for mediator
dockgroup* dock;
XBox *b2;

#define TB_HEIGHT 38 // TODO tb hack

int main(int argc, char **argv) {

#if (FLTK_EXT_VERSION>0)
    Fl::scheme("flat");
#else
    Fl::scheme("gtk+"); // TODO ability to change - see unittests
#endif
    // set to system local to "C" default for mostly work.
    setlocale(LC_ALL, "C");
    makeChecker(); // TODO move to more appropriate location

    Fl_Image::RGB_scaling(FL_RGB_SCALING_NEAREST); // TODO use a fl_imgtk scaler by default


    // TODO tb : mediator needs to know about main, XBox
    YaivWin* _w = makeMainWindow();

    dock = new dockgroup(1, 1,  _w->w() - 2, TB_HEIGHT + 2);
    dock->box(FL_THIN_DOWN_BOX);
    dock->color(FL_BLACK); // TODO from prefs/theme
    dock->end();
    dock->set_window(_w);

    add_btn_bar(dock, 0);
    dock->redraw();

    _w->set_dock(dock);

    _w->begin();

    _w->workspace = new Fl_Group(2,TB_HEIGHT+2,_w->w()-4, _w->h()-4-TB_HEIGHT-2);
    _w->workspace->box(FL_THIN_DOWN_BOX);

    //XBox *b2 = new XBox(2,TB_HEIGHT + 2,_w->w(),_w->h(), _w->prefs());
    b2 = new XBox(2,TB_HEIGHT+3,_w->workspace->w()-2,_w->workspace->h(), _w->prefs());

    _w->child(b2);
    _w->workspace->resizable(static_cast<Fl_Widget *>(b2));

    //b2->parent(_w);
    //_w->resizable(static_cast<Fl_Widget *>(b2));

    // TODO tb : mediator needs to know about transbox
    // TODO transbox location, size from prefs
    int TB_HIGH=35;
    Fl_TransBox *tb = new Fl_TransBox(0, _w->h()-TB_HIGH, _w->w(), TB_HIGH);

    _w->workspace->end();
    _w->end();
    _w->resizable(_w->workspace);


    Fl::lock(); /// thread lock must be called in this time for init.

    // TODO tb : replace by mediator
    XBoxDspInfoEI* xbdiei = new XBoxDspInfoEI(tb);
    b2->displayEventHandler(xbdiei);
    xbdiei->OnActivate(false); // TODO tie to initial state from options

    // TODO tb : replace by mediator
    XBoxDisplayInfoTitle* xbdit = new XBoxDisplayInfoTitle(_w);
    b2->displayEventHandler(xbdit);

    //add_text_bar(_w, nullptr); // TODO tb currently for danbooru

    _w->show();
    toolgrp::show_all();
    cmdline(argc, argv, b2); // do this _after_ show() for label etc to be correct

    return Fl::run();
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif