#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

#include <clocale>     // setlocale()..

#include <FL/Fl_File_Icon.H>

#include "yaiv_win.h"
#include "Fl_TransBox.h"
#include "XBoxDisplayInfoEvent.h"
#include "buttonBar.h"
#include "mediator.h"
#include "themes.h"

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

int main(int argc, char **argv) {

#if (FLTK_EXT_VERSION>0)
    Fl::scheme("flat");
#else
    Fl::scheme("gtk+"); // TODO ability to change - see unittests
    //Fl::scheme("plastic"); // TODO ability to change - see unittests
#endif

    FL_NORMAL_SIZE = 18;
    
    TanColormap_FLTKSUBS(); // TODO from settings

    // set to system local to "C" default for mostly work.
    setlocale(LC_ALL, "C");
    
    // appearance
    Fl_File_Icon::load_system_icons();
    // Issue #114: asian characters missing from open dialog, 'last used' menu
    // TODO if this font isn't installed, falls back to something that won't give the desired result
    Fl::set_font(FL_HELVETICA, " Noto Sans CJK SC");
    Fl::set_font(FL_HELVETICA_BOLD, "BNoto Sans CJK SC"); // note this isn't the 'display name'
    
    makeChecker(); // TODO move to more appropriate location

    Fl_Image::RGB_scaling(FL_RGB_SCALING_NEAREST); // TODO use a fl_imgtk scaler by default

    // TODO tb : mediator needs to know about main, XBox, prefs
    YaivWin* _w = makeMainWindow();
    ButtonBar *tb = makeToolbar(_w);

    _w->begin();

    int ws_x = tb->getXoffset();
    int ws_y = tb->getYoffset();
    _w->workspace = new Fl_Group(ws_x,ws_y,_w->w()-1-ws_x, _w->h()-1-ws_y);
    _w->workspace->box(FL_NO_BOX);

    XBox *b2 = new XBox(ws_x, ws_y,_w->workspace->w(),_w->workspace->h(), _w->prefs());

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

    Mediator::initialize(b2, _w->prefs(), tb);
    
    // TODO this the wrong place / way to initialize these buttons?
    Mediator::send_message(Mediator::MSG_TB, Mediator::ACT_NOPREV);
    Mediator::send_message(Mediator::MSG_TB, Mediator::ACT_NONEXT);
    // initializer for checkerboard
    // TODO checker init relies on state initialized to OFF 
    Mediator::send_message(Mediator::MSG_TB, Mediator::ACT_CHK);
    
    
    cmdline(argc, argv, b2); // do this _after_ show() for label etc to be correct

    auto result = Fl::run();
    b2->wipeShowImage(true);
    delete overlay;
    delete b2;
    delete tb;
    overlay = nullptr;
    b2 = nullptr;
    tb = nullptr;
    return result;
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
