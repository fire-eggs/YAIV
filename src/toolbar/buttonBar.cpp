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
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_SVG_Image.H>
#include <whereami.h>
#include "themes.h"

#define BTNSIZE 40 // TODO from options
#define HANDWID 17
#define NOHANDWID 7
#define BTNDOWN  5 // inner_group down 3
#define BTNSTEP (BTNSIZE + 3)

#define TB_HEIGHT (BTNSIZE + BTNDOWN + 2)

static void setImage(char *exepath, Fl_Widget *btn, char *imgn, int sz=25)
{
    char buff[500];
    sprintf(buff, "%s/icons/%s.svg", exepath, imgn); // TODO find sub-folder? hard-coded in code?

    btn->align(FL_ALIGN_CENTER);
    btn->visible_focus(0);

    Fl_SVG_Image *img = new Fl_SVG_Image(buff,0);
    if (img->fail()) {
        img->release();
        return;
    }

    if (!OS::is_dark_theme(OS::current_theme())) // TODO use isdark
        img->color_average(FL_BLACK, 0.0);

    btn->image(img->copy(sz,sz));

    if (OS::is_dark_theme(OS::current_theme())) // TODO use isdark
        img->color_average(FL_BLACK, 0.0);
    else
        img->color_average(FL_WHITE, 0.0);
    btn->deimage(img->copy(sz,sz));

    img->release();
}

void ButtonBar::updateColor(bool isDark) {
    int chils = _tgroup->in_group()->children();
    for (int i=0; i< chils; i++) {
        Fl_Button *btn = dynamic_cast<Fl_Button*>(_tgroup->in_group()->child(i));
        if (btn) {
            if (isDark) {
                btn->image()->color_average(FL_WHITE, 0.0);
                btn->deimage()->color_average(FL_BLACK, 0.0);
            }
            else {
                btn->image()->color_average(FL_BLACK, 0.0);
                btn->deimage()->color_average(FL_WHITE, 0.0);
            }
        }
    }
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
        Mediator::ACT_SCALE,
        Mediator::ACT_MENU,
        Mediator::ACT_EXIT,
};

Fl_Menu_Item scale_menu[6] =
        {
                {"No scale",        0, nullptr, (void *)(fl_intptr_t) Mediator::ACT_SCALE_NONE},
                {"Auto scale",      0, nullptr, (void *)(fl_intptr_t) Mediator::ACT_SCALE_AUTO},
                {"Scale to Fit",    0, nullptr, (void *)(fl_intptr_t) Mediator::ACT_SCALE_FIT},
                {"Scale to Width",  0, nullptr, (void *)(fl_intptr_t) Mediator::ACT_SCALE_WIDE},
                {"Scale to Height", 0, nullptr, (void *)(fl_intptr_t) Mediator::ACT_SCALE_HIGH},
                {nullptr} // end of menu
        };

Mediator::ACTIONS scaleMenu()
{
    int xloc = Fl::event_x();
    int yloc = Fl::event_y();
    const Fl_Menu_Item *m = scale_menu->popup(xloc, yloc, nullptr, nullptr, nullptr);
    if (m)
        return (Mediator::ACTIONS)(fl_intptr_t)m->user_data();
    return Mediator::ACTIONS::ACT_INVALID;
}

static void btnCb(Fl_Widget *w, void *data)
{
    int val = (int)(fl_intptr_t)data;
    switch (acts[val])
    {
        case Mediator::ACT_SCALE:
            {
            Mediator::ACTIONS newscale = scaleMenu();
            if (newscale != Mediator::ACT_INVALID)
                send_message(Mediator::MSG_TB, newscale);
            }
            break;
        default:
            send_message(Mediator::MSG_TB, acts[val]);
            break;
    }
}

static void makeBtn(char *exepath, bool draggable, toolgrp* tg, int i, char *name, bool vert, bool toggle)
{
    int x = (draggable ? HANDWID : NOHANDWID) + (BTNSTEP * i);
    int y = BTNDOWN;
    if (vert) {
        x = BTNDOWN;
        y = (draggable ? HANDWID : NOHANDWID) + (BTNSTEP * i);
    }

    Fl_Button *btn1 = toggle ? new Fl_Toggle_Button(x,y,BTNSIZE,BTNSIZE) :
                               new Fl_Button(x, y, BTNSIZE, BTNSIZE);
    btn1->callback(btnCb, (void *)(fl_intptr_t)i);
    //btn1->box(FL_THIN_UP_BOX);
    btn1->box(FL_UP_BOX);
    setImage(exepath, btn1, name);
    btn1->tooltip(name);
    tg->add(btn1);
}

void btn_bar_common(char *exepath, toolgrp* tgroup, bool vert, bool draggable)
{
    tgroup->box(FL_BORDER_BOX);
    tgroup->in_group()->box(FL_NO_BOX);

    char *btns [] = {"ViewPreviousImage", "ViewNextImage", "ZoomIn",
                     "ZoomOut", "Slideshow", "RotateRight",
                     "OpenFile", "GoToImage", "Checkerboard",
                     "scaletofit", "Menu", "exit_white"};
    // whether the button is a toggle
    bool btntype [] = {false,false,false,
                       false,true,false,
                       false,false,true,
                       false,false,false};
    int count = sizeof(btns) / sizeof(char*);
    for (int i=0; i < count; i++)
        makeBtn(exepath, draggable, tgroup, i, btns[i], vert, btntype[i]);

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

ButtonBar* ButtonBar::add_btn_bar(char *exePath, dockgroup *dock, bool vert, bool floating) {
    ButtonBar *bbar = new ButtonBar();
    bbar->exepath = exePath;
    bbar->setActions(acts);
    int btncount = sizeof(acts) / sizeof(Mediator::ACTIONS);
    bool draggable = false;
    int tbwide = (draggable ? HANDWID : NOHANDWID) + (btncount * BTNSTEP) + 3;
    bbar->_tgroup = vert ? new vtoolgrp(dock, floating, false, TB_HEIGHT, tbwide) :
                           new  toolgrp(dock, floating, false, tbwide, TB_HEIGHT);
    btn_bar_common(exePath, bbar->_tgroup, vert, draggable);
    bbar->_vert = vert;
    return bbar;
}

ButtonBar::ButtonBar() { }

void ButtonBar::setScaleImage(Mediator::ACTIONS who) {
    Fl_Group* inner = _tgroup->in_group();
    for (int i= 0; i < inner->children(); i++) {
        Fl_Widget *ch = inner->child(i);
        Fl_Button* ch2 = dynamic_cast<Fl_Button*>(ch);
        if (ch2 && _acts[i] == Mediator::ACT_SCALE) {
            // set button image/tooltip
            switch (who)
            {
                // TODO copy-pasta
                case Mediator::ACT_SCALE_NONE:
                    setImage(exepath, ch,"scaletofit");
                    ch->tooltip("Full Size");
                    break;
                case Mediator::ACT_SCALE_AUTO:
                    setImage(exepath, ch,"autozoom");
                    ch->tooltip("Auto-Zoom");
                    break;
                case Mediator::ACT_SCALE_FIT:
                    setImage(exepath, ch,"zoomtofit");
                    ch->tooltip("Best Fit");
                    break;
                case Mediator::ACT_SCALE_WIDE:
                    setImage(exepath, ch,"scaletowidth");
                    break;
                case Mediator::ACT_SCALE_HIGH:
                    setImage(exepath, ch,"scaletoheight");
                    break;
            }
            ch2->redraw();
            break;
        }
    }

}

ButtonBar* makeToolbar(dropwin* win) {

    dockgroup* dock;
    ButtonBar *tb;

    bool vertbar = true; // TODO as an option
    if (!vertbar) {
        dock = new dockgroup(false, 1, 1,  win->w() - 2, TB_HEIGHT + 2);
    }
    else {
        dock = new dockgroup(true,1, 1,  TB_HEIGHT + 2, win->h() - 2);
    }
    dock->box(FL_THIN_DOWN_BOX);
    dock->resizable(nullptr); // prevent buttons from resizing

    dock->end();
    dock->set_window(win);

    // find the executable path
    int dirnamelen;
    int len = wai_getExecutablePath(NULL,0,&dirnamelen);
    char *exePath = (char *)malloc(len+1);
    wai_getExecutablePath(exePath, len, &dirnamelen);
    exePath[dirnamelen] = '\0';

    tb = ButtonBar::add_btn_bar(exePath, dock, vertbar, false);

    //free(exePath); // buttons can change, don't free this

    dock->redraw();
    win->set_dock(dock);
    return tb;
}

void ButtonBar::deactivate(Mediator::ACTIONS who) {
    Fl_Group* inner = _tgroup->in_group();
   if (who == Mediator::ACT_NONEXT)
       inner->child(1)->deactivate(); // TODO hard-coded toolbar button location
   if (who == Mediator::ACT_NOPREV)
       inner->child(0)->deactivate(); // TODO hard-coded toolbar button location
}
void ButtonBar::activate(Mediator::ACTIONS who) {
    Fl_Group* inner = _tgroup->in_group();
    if (who == Mediator::ACT_ISNEXT)
        inner->child(1)->activate(); // TODO hard-coded toolbar button location
    if (who == Mediator::ACT_ISPREV)
        inner->child(0)->activate(); // TODO hard-coded toolbar button location
}