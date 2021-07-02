//
// Created by kevin on 5/26/21.
//
// A crude slideshow: sequences through images in the current window,
// using the current settings.
//
// TODO support from XBox to pause/continue slideshow
// TODO manually changing current image in XBox needs to reset timer

#include <FL/Fl.H>
#include "Slideshow.h"
#include "XBox.h"

Slideshow::Slideshow() {
    _slideDelay = 10;
}

void Slideshow::start(int current) {

    _window->load_current();
    if (_prefs)
        _prefs->get(SLIDESHOW_DELAY, _slideDelay, 10); // TODO see forceDelay for issue
    setTimer();

    // TODO options from prefs
}

void Slideshow::stop() {
    clearTimer();
}

// command line override
void Slideshow::forceDelay(int val)
{
    stop();
    _slideDelay = val;
    setTimer();        // TODO this is just wrong
}

static void secondsCallback(void* who) {
    static_cast<Slideshow *>(who)->countdown();
}

void Slideshow::countdown()
{
    _timeout--;
    if (_timeout <= 0)
    {
        slideNext();
        return;
    }
    Fl::repeat_timeout(1.0, secondsCallback, this);
}

void Slideshow::clearTimer() {
    Fl::remove_timeout((Fl_Timeout_Handler)secondsCallback);
}

void Slideshow::setTimer(){
    _timeout = _slideDelay;
    Fl::add_timeout(1.0, (Fl_Timeout_Handler)secondsCallback, this);
}

void Slideshow::slideNext() {
    clearTimer();
    _window->next_image();
    setTimer();
}
