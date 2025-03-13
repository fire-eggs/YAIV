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

    void forceDelay(int val);

    void setPrefs(Prefs * prefs) {_prefs = prefs;}
    void setWindow(XBox *win) {_window = win;}
    void start(int current);
    static void stop();
    static void clearTimer();
    void setTimer();
    void countdown();
    void slideNext();
    void resetTimer(); // On manual image change, force timer to start over

private:
    Prefs *_prefs = nullptr;
    XBox *_window = nullptr;
    int _timeout = -1;    // current slide countdown
    int _slideDelay = -1; // seconds between slides
    int _slideShuffle = 0; // shuffle at beginning of slideshow
    int _slideBorder = 0; // border for slideshow
    int _slideErrors = 1;
    int _slideWrap   = 1;
};

#endif //YAIV_SLIDESHOW_H
