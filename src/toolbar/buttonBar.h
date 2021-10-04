//
// Created by kevin on 7/4/21.
//

#ifndef YAIV_BUTTONBAR_H
#define YAIV_BUTTONBAR_H

#include "dock_gp.h"
#include "toolgrp.h"
#include "dropwin.h"
#include "mediator.h"

class ButtonBar
{
private:
    ButtonBar();
    char *exepath;
    toolgrp* _tgroup;
    Mediator::ACTIONS* _acts;
    bool _vert;

    void setActions(Mediator::ACTIONS*acts) {_acts = acts;}

public:
    void setState(Mediator::ACTIONS, int val);
    void setScaleImage(Mediator::ACTIONS who);

    static ButtonBar* add_btn_bar(char *exePath, dockgroup *dock, bool vertical, bool floating);

    int getXoffset() {return _vert ? _tgroup->w() : 1 ;}
    int getYoffset() {return _vert ? 1 : _tgroup->h() ;}


    void activate(Mediator::ACTIONS who);
    void deactivate(Mediator::ACTIONS who);
    void updateColor(bool isDarkTheme);
};

ButtonBar* makeToolbar(dropwin* win); // TODO static method

#endif //YAIV_BUTTONBAR_H
