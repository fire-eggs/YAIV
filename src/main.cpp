#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

#include <clocale>     // setlocale()..

#include "yaiv_win.h"
#include "Fl_TransBox.h"
#include "XBoxDisplayInfoEvent.h"
#include "buttonBar.h"
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
XBox *b2;
ButtonBar* tb;

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

    tb = makeToolbar(_w);

    _w->begin();

    int ws_x = tb->getXoffset();
    int ws_y = tb->getYoffset();
    _w->workspace = new Fl_Group(ws_x,ws_y,_w->w()-1-ws_x, _w->h()-1-ws_y);
    _w->workspace->box(FL_NO_BOX);

    b2 = new XBox(ws_x, ws_y,_w->workspace->w(),_w->workspace->h(), _w->prefs());

    _w->workspace->resizable(static_cast<Fl_Widget *>(b2));

    // TODO tb : mediator needs to know about transbox
    // TODO transbox location, size from prefs
    int TB_HIGH=35;
    Fl_TransBox *overlay = new Fl_TransBox(0, _w->h()-TB_HIGH, _w->w(), TB_HIGH);

    _w->workspace->end();
    _w->end();
    _w->resizable(_w->workspace);


    Fl::lock(); /// thread lock must be called in this time for init.

    // TODO tb : replace by mediator
    XBoxDspInfoEI* xbdiei = new XBoxDspInfoEI(overlay);
    b2->displayEventHandler(xbdiei);
    xbdiei->OnActivate(false); // TODO tie to initial state from options

    // TODO tb : replace by mediator
    XBoxDisplayInfoTitle* xbdit = new XBoxDisplayInfoTitle(_w);
    b2->displayEventHandler(xbdit);

    _w->show();
    toolgrp::show_all();

    Mediator::send_message(Mediator::MSG_TB, Mediator::ACT_NOPREV);
    Mediator::send_message(Mediator::MSG_TB, Mediator::ACT_NONEXT);

    cmdline(argc, argv, b2); // do this _after_ show() for label etc to be correct

    return Fl::run();
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
