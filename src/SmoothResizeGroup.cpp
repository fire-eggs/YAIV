//
// Created by kevin on 6/21/21.
//
// Modifies the default resize behavior of Fl_Group to be less constant/ glitchy.
// The resize action is propagated to the derived class via the safe_resize callback.
// The safe_resize callback is made after 0.25 seconds of no resize activity.
//

#include "SmoothResizeGroup.h"

void SmoothResizeGroup::resizeTimerCallback(void *d)
{
    // Timer callback during resizing
    SmoothResizeGroup *who = static_cast<SmoothResizeGroup *>(d);
    if (who)
        who->resizeTimerFire();
}

void SmoothResizeGroup::resizeTimerFire()
{
    // Our timer has fired: now safe to resize
    _inResize = false;
    safe_resize();
}

void SmoothResizeGroup::resizeTimer()
{
    // Don't update the cached image until resizing has settled for a little bit
    Fl::add_timeout(0.25, resizeTimerCallback, (void *)this);
}

void SmoothResizeGroup::restartResizeTimer()
{
    // got another resize call while timer active. reset to try again.
    Fl::remove_timeout(resizeTimerCallback);
    resizeTimer();
}

void SmoothResizeGroup::resize(int x,int y,int w,int h) {
    Fl_Group::resize(x,y,w,h);

    // Prevent constant / glitchy resizing
    if (_inResize)
        restartResizeTimer();
    else
        resizeTimer();
    _inResize = true;
}
