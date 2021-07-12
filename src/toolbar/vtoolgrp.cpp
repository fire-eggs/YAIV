//
// Created by kevin on 7/12/21.
//

#include "vtoolgrp.h"

vtoolgrp::vtoolgrp(dockgroup *dk, int floater, int w, int h, const char *lbl)
        : toolgrp(w, h, lbl)
{
    if (floater && !dk)
    {
        create_floating(nullptr,0,0,0,w,h,lbl); // floating and not dockable
    }
    else if (floater && dk) // create floating
    {
        create_floating(dk, 0, 0, 0, w, h, lbl);
    }
    else if (dk) // create docked
    {
        create_docked(dk);
    }

}

void vtoolgrp::create_dockable_group()
{
    dismiss = new Fl_Button(3, 3, 11, 11, "@-31+");
    dismiss->box(FL_BORDER_BOX);
    dismiss->tooltip("Dismiss");
    dismiss->clear_visible_focus();
    dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);

    dragger = new drag_btn(17, 3, w()-20, 11);
    dragger->type(FL_TOGGLE_BUTTON);
    dragger->box(FL_ENGRAVED_BOX);
    dragger->tooltip("Drag Box");
    dragger->clear_visible_focus();
    dragger->when(FL_WHEN_CHANGED);

    inner_group = new Fl_Group(3, 17, w() - 6, h() - 20);
    inner_group->box(FL_ENGRAVED_FRAME);
}
