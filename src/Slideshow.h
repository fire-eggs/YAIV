//
// Created by kevin on 5/26/21.
//

#ifndef YAIV_SLIDESHOW_H
#define YAIV_SLIDESHOW_H

#include "prefs.h"

class XBox;
class Slideshow
{
public:
    Slideshow();

    void setPrefs(Prefs * prefs) {_prefs = prefs;}
    void setWindow(XBox *win) {_window = win;}
    void start(int current);
    void stop();
    void clearTimer();
    void setTimer();
    void countdown();
    void slideNext();
    void redrawOverlay();

private:
    Prefs *_prefs = nullptr;
    XBox *_window = nullptr;
    int _timeout = -1;    // current slide countdown
    int _slideDelay = -1; // seconds between slides
};

#endif //YAIV_SLIDESHOW_H
