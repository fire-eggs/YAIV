//
// Created by kevin on 7/4/21.
//

#ifndef YAIV_BUTTONBAR_H
#define YAIV_BUTTONBAR_H

#include "dock_gp.h"
#include "toolgrp.h"
#include "mediator.h"

class ButtonBar
{
private:
    ButtonBar();
    toolgrp* _tgroup;
    Mediator::ACTIONS* _acts;

    void setActions(Mediator::ACTIONS*acts) {_acts = acts;}

public:
    void setState(Mediator::ACTIONS, int val);
    void setScaleImage(Mediator::ACTIONS who);

    static ButtonBar* add_btn_bar(dockgroup *dock, int floating);
    static ButtonBar* add_vert_btn_bar(dockgroup *dock, bool floating);
};

#endif //YAIV_BUTTONBAR_H
