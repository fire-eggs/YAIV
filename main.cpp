
#include <clocale>     // setlocale()..

void load_file(const char *n); // hack

#include "MyW.h"
#include "XBox.h"

MyW *_w;
XBox *_b2;

void MyW::updateLabel() {
    char lbl[1000];
    lbl[0] = 0;
    label(::_b2->getLabel(filename, lbl, sizeof(lbl)));
}

int dvisual = 0;
int arg(int, char **argv, int &i) {
    if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
    return 0;
}

int main(int argc, char **argv) {

    Fl::scheme("gtk+"); // TODO ability to change - see unittests
    setlocale(LC_ALL, "");    // enable multilanguage errors in file chooser
    makeChecker();

    Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR);

    // TODO rework to add options, filename, etc
    int i = 1;
    Fl::args(argc,argv,i,arg);
    if (!dvisual) Fl::visual(FL_RGB);

    _w = new MyW(400,450);
    _b2 = new XBox(5,5,390,440);
    _w->child(_b2);
    _w->resizable(_b2);
    _w->end();

    if (argv[1]) load_file(argv[1]); // TODO add more options

    _w->show(argc,argv);
    return Fl::run();
}
