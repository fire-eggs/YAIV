//
// Created by kevin on 6/21/21.
//

#ifndef YAIV_SMOOTHRESIZEGROUP_H
#define YAIV_SMOOTHRESIZEGROUP_H

#include "FL/Fl_Group.H"

class SmoothResizeGroup : public Fl_Group
{
public:
    SmoothResizeGroup(int x, int y, int w, int h, const char *foo= nullptr) : Fl_Group(x,y,w,h,foo) {}

    // callback function for when it is safe to resize
    virtual void safe_resize() = 0;

    void resizeTimerFire();

    // overload of Fl_Group resize
    void resize(int x,int y,int w,int h) override;

private:
    bool _inResize = false;

    void resizeTimer();
    void restartResizeTimer();
    static void resizeTimerCallback(void *);
};

#endif //YAIV_SMOOTHRESIZEGROUP_H
