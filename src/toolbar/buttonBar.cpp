//
// Created by kevin on 7/4/21.
//
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "buttonBar.h"
#include "toolgrp.h"
#include "mediator.h"

#define BTNSIZE 30
#define HANDWID 18
#define BTNDOWN  7 // inner_group down 3
#define BTNSTEP 33

#define TB_HEIGHT 40
#define TB_WIDTH HANDWID + (BTNSTEP * 11)

#include <FL/Fl_SVG_Image.H>

static void setImage(Fl_Widget *btn, char *imgn, int sz=25)
{
    char buff[500];
    sprintf(buff, "icons/%s.svg", imgn); // TODO find sub-folder? hard-coded in code?

    btn->align(FL_ALIGN_CENTER);
    btn->color(8,FL_BLACK);
    btn->visible_focus(0);

    Fl_SVG_Image *img = new Fl_SVG_Image(buff,0);
    if (img->fail())
        return;
    btn->image(img->copy(sz,sz));
}

Mediator::ACTIONS acts[] = {
        Mediator::ACT_PREV,
        Mediator::ACT_NEXT,
        Mediator::ACT_ZMI,
        Mediator::ACT_ZMO,
        Mediator::ACT_SLID,
        Mediator::ACT_ROTR,
        Mediator::ACT_OPEN,
        Mediator::ACT_GOTO,
        Mediator::ACT_CHK,
        Mediator::ACT_MENU,
        Mediator::ACT_EXIT,
};

static void btnCb(Fl_Widget *w, void *data)
{
    int val = (int)(fl_intptr_t)data;
    send_message(Mediator::MSG_TB, acts[val]);
}

static void makeBtn(toolgrp* tg, int i, char *name)
{
    Fl_Button *btn1 = new Fl_Button(HANDWID + (BTNSTEP * i), BTNDOWN, BTNSIZE, BTNSIZE);
    btn1->callback(btnCb, (void *)(fl_intptr_t)i);
    btn1->box(FL_THIN_UP_BOX);
    btn1->tooltip(name);
    setImage(btn1, name);
    tg->add(btn1);
}

void add_btn_bar(dockgroup *dock, int floating)
{
    // Create a docked toolgroup
    toolgrp *tgroup = new toolgrp(dock, floating, TB_WIDTH, TB_HEIGHT); // TODO width from # of buttons
    tgroup->box(FL_BORDER_BOX);
    tgroup->in_group()->box(FL_NO_BOX);
    tgroup->color(FL_BLACK);

    char *btns [] = {"ViewPreviousImage", "ViewNextImage", "ZoomIn",
                     "ZoomOut", "Slideshow", "RotateRight",
                     "OpenFile", "GoToImage", "Checkerboard",
                     "Menu", "exit_white"};
    int count = sizeof(btns) / sizeof(char*);
    for (int i=0; i < count; i++)
        makeBtn(tgroup, i, btns[i]);

    tgroup->end();
}
