#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"

#include <clocale>     // setlocale()..

#include "yaiv_win.h"

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

   Fl::lock(); /// thread lock must be called in this time for init.
#if (FLTK_EXT_VERSION>0)
    Fl::scheme("flat");
#else
    Fl::scheme("gtk+"); // TODO ability to change - see unittests
#endif
    // set to system local to "C" default for mostly work.
    setlocale(LC_ALL, "C");
    makeChecker(); // TODO move to more appropriate location

    Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR); // TODO use a fl_imgtk scaler by default

    // TODO rework to add options, filename, etc

    YaivWin* _w = makeMainWindow();
    XBox *b2 = new XBox(0,0,_w->w(),_w->h(), _w->prefs());
    _w->child(b2);
    b2->parent(_w);
    _w->resizable(static_cast<Fl_Widget *>(b2));

    // TODO transbox location, size from prefs
    int TB_HIGH=35;
    Fl_TransBox *tb = new Fl_TransBox(0, _w->h()-TB_HIGH, _w->w(), TB_HIGH);
    _w->end();

    XBoxDspInfoEI* xbdiei = new XBoxDspInfoEI(tb);
    b2->displayEventHandler(xbdiei);
    xbdiei->OnActivate(false); // TODO tie to initial state from options

    _w->show();
    cmdline(argc, argv, b2); // do this _after_ show() for label etc to be correct

    return Fl::run();
}

#pragma clang diagnostic pop