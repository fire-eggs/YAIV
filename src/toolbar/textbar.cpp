//
// Created by kevin on 7/4/21.
//

#include "textbar.h"
#include "toolgrp.h"
#include <FL/Fl_Text_Editor.H>

void add_text_bar(Fl_Widget*, void*)
{
    // TODO tb get main window position
    //int x = win_main->x() + 100; // KBR position relative to main
    //int y = win_main->y() + 100; // KBR works except if floater added before main appears

    Fl_Text_Buffer *textbuf;
    textbuf = new Fl_Text_Buffer;

    textbuf->text("This is a test of the emergency broadcast system."
                  "\n\nThis is only a test.\n\n...and now for something completely different.");

    //toolgrp *txtgrp = new toolgrp(nullptr, 1, x, y, 200, 500);
    toolgrp *txtgrp = new toolgrp(nullptr, 1, 0, 0, 200, 500);
    Fl_Text_Display *txtout = new Fl_Text_Display(24,10,166,480);
    txtout->color(FL_GRAY);
    txtout->box(FL_NO_BOX);
    txtout->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    txtout->textfont(FL_TIMES);
    txtout->textsize(14);
    txtout->buffer(textbuf);

    txtgrp->end();
    txtgrp->resizable(txtout);
    //txtgrp->box(FL_THIN_UP_BOX);
}
