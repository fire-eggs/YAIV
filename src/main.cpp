
#include <clocale>     // setlocale()..

void load_file(const char *n); // TODO hack

#include "MyW.h"
#include "XBox.h"
#include "prefs.h"

MyW *_w;
XBox *_b2;
Prefs *_prefs;

MyW::MyW(int x, int y, int w, int h) : Fl_Double_Window(x,y,w,h),
_xoff(0), _yoff(0), _border(1) {
    // NOTE do _not_ toggle the border in here. Prevents minimize/restore.
#ifdef __linux__
    // some Linux window manager cannot handle resize by thin window border.
    border( 2 );
#endif

    // Colors from preferences so RaphK can have dark and I can have light :)
    int fg, bg;
    _prefs->get("MainColor", bg, FL_BACKGROUND_COLOR);
    _prefs->get("MainLabelColor", fg, FL_FOREGROUND_COLOR);
    color(bg);
    labelcolor(fg);
}

void MyW::updateLabel() {
    char lbl[1000];
    lbl[0] = 0;
    label(::_b2->getLabel(filename, lbl, sizeof(lbl)));
}

void MyW::resize(int x, int y, int w, int h) {
    Fl_Double_Window::resize(x,y,w,h);
    _prefs->setWinRect(MAIN_PREFIX, x, y, w, h);
}

void MyW::toggle_border() {
    _border = !_border;
    border(_border);
    _prefs->set2("BORDER", _border);
}

static int dvisual = 0;
int arg(int, char **argv, int &i) {
    if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
    return 0;
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
    int i = 1;
    Fl::args(argc,argv,i,arg);
    if (!dvisual) Fl::visual(FL_RGB);

    int x, y, w, h;
    _prefs = new Prefs();
    _prefs->getWinRect(MAIN_PREFIX, x, y, w, h);

    _w = new MyW(x,y,w,h);
    _b2 = new XBox(5,5,w-10,h-10);
    _w->child(_b2);
    _w->resizable(_b2);
    _w->end();

    if (argv[1]) load_file(argv[1]); // TODO add more options

    _w->show(argc,argv);
    return Fl::run();
}
