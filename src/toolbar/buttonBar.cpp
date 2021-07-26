//
// Created by kevin on 7/4/21.
//
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include "buttonBar.h"
#include "toolgrp.h"
#include "mediator.h"
#include "vtoolgrp.h"
#include <FL/Fl_Toggle_Button.H>

#define BTNSIZE 30
#define HANDWID 17
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

// TODO must match button order
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

static void makeBtn(toolgrp* tg, int i, char *name, bool vert, bool toggle)
{
    int x = HANDWID + (BTNSTEP * i);
    int y = BTNDOWN;
    if (vert) {
        x = BTNDOWN;
        y = HANDWID + (BTNSTEP * i);
    }
    Fl_Button *btn1 = toggle ? new Fl_Toggle_Button(x,y,BTNSIZE,BTNSIZE) :
                               new Fl_Button(x, y, BTNSIZE, BTNSIZE);
    btn1->callback(btnCb, (void *)(fl_intptr_t)i);
    btn1->box(FL_THIN_UP_BOX);
    btn1->tooltip(name);
    setImage(btn1, name);
    tg->add(btn1);
}

void btn_bar_common(toolgrp* tgroup, bool vert)
{
    tgroup->box(FL_BORDER_BOX);
    tgroup->in_group()->box(FL_NO_BOX);
    tgroup->color(FL_BLACK);

    char *btns [] = {"ViewPreviousImage", "ViewNextImage", "ZoomIn",
                     "ZoomOut", "Slideshow", "RotateRight",
                     "OpenFile", "GoToImage", "Checkerboard",
                     "Menu", "exit_white"};
    // whether the button is a toggle
    bool btntype [] = {false,false,false,
                       false,true,false,
                       false,false,true,
                       false,false};
    int count = sizeof(btns) / sizeof(char*);
    for (int i=0; i < count; i++)
        makeBtn(tgroup, i, btns[i], vert, btntype[i]);

    tgroup->end();
}

void ButtonBar::setState(Mediator::ACTIONS act, int val) {
    Fl_Group* inner = _tgroup->in_group();
    for (int i= 0; i < inner->children(); i++) {
        Fl_Widget *ch = inner->child(i);
        Fl_Toggle_Button* ch2 = dynamic_cast<Fl_Toggle_Button*>(ch);
        if (ch2 && _acts[i] == act) {
            ch2->value(val);
            break;
        }
    }
}

ButtonBar* ButtonBar::add_vert_btn_bar(dockgroup* dock, bool floating)
{
    ButtonBar *bbar = new ButtonBar();
    bbar->setActions(acts);
    bbar->_tgroup = new vtoolgrp(dock, floating, TB_HEIGHT, TB_WIDTH);
    btn_bar_common(bbar->_tgroup, true);
    return bbar;
}
ButtonBar* ButtonBar::add_btn_bar(dockgroup *dock, int floating) {
    ButtonBar *bbar = new ButtonBar();
    bbar->setActions(acts);
    bbar->_tgroup = new toolgrp(dock, floating, TB_WIDTH, TB_HEIGHT); // TODO width from # of buttons
    btn_bar_common(bbar->_tgroup, false);
    return bbar;
}

ButtonBar::ButtonBar() { }
