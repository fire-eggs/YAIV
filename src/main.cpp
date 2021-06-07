
#include <clocale>     // setlocale()..

#include "yaiv_win.h"
#include "Fl_TransBox.h"
#include "XBoxDisplayInfoEvent.h"

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

    XBoxDspInfoEI* xbdiei = new XBoxDspInfoEI();
    xbdiei->setDestination(tb);
    b2->displayEventHandler(xbdiei);

    if (argv[1]) b2->load_file(argv[1]); // TODO add more options

    _w->show(argc,argv);
    return Fl::run();
}
