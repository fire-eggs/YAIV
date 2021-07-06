//
// Created by kevin on 7/4/21.
//

#pragma GCC diagnostic ignored "-Wwrite-strings"

#include "buttonBar.h"

#include "toolgrp.h"

#define BTNSIZE 30
#define HANDWID 18
#define BTNDOWN  7 // inner_group down 3
#define BTNSTEP 33

#define TB_HEIGHT 40

#include <FL/Fl_SVG_Image.H>

static void setImage(Fl_Widget *btn, char *imgn, int sz=25)
{
    char buff[500];
    sprintf(buff, "icons/%s.svg", imgn); // TODO find sub-folder? hard-coded in code?
    Fl_SVG_Image *img = new Fl_SVG_Image(buff,0);
    if (!img || img->ld() < 0) return;
    btn->image(img->copy(sz,sz));
    btn->align(FL_ALIGN_CENTER);
    btn->color(8,FL_BLACK);
    btn->visible_focus(0);
}


static void btnCb(Fl_Widget *w, void *data)
{
    size_t val = (size_t)data; // TODO hack
    printf("Btn: %zu\n", val);
    // TODO tb : mediator
    //send_message(MSGS::TB, (int)data);
}

static void makeBtn(toolgrp* tg, int i, char *name)
{
    Fl_Button *btn1 = new Fl_Button(HANDWID + (BTNSTEP * i), BTNDOWN, BTNSIZE, BTNSIZE);
    btn1->callback(btnCb, (void *)i);
    btn1->box(FL_THIN_UP_BOX);
    btn1->tooltip(name);
    setImage(btn1, name);
    tg->add(btn1);
}

void add_btn_bar(Fl_Widget *, void*)
{
    // Create a docked toolgroup
    //toolgrp *tgroup = new toolgrp(dock, 0, 350, TB_HEIGHT); // TODO width from # of buttons
    toolgrp *tgroup = new toolgrp(nullptr, 1, 350, TB_HEIGHT); // TODO width from # of buttons
    tgroup->box(FL_BORDER_BOX);
    tgroup->in_group()->box(FL_NO_BOX);
    tgroup->color(FL_BLACK);
    tgroup->end();

    char *btns [] = {"ViewPreviousImage","ViewNextImage","ZoomIn",
                     "ZoomOut","Slideshow","RotateRight",
                     "OpenFile", "GoToImage","Checkerboard"};
    for (int i=0; i < 9; i++)
        makeBtn(tgroup, i, btns[i]);
}
