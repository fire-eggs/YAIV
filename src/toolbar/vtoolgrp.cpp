//
// Created by kevin on 7/12/21.
//

#include "vtoolgrp.h"

vtoolgrp::vtoolgrp(dockgroup *d, bool floating, bool draggable, int w, int h, const char *l)
    : toolgrp(w, h, l) {

    initialize(d, floating, draggable, w, h, l, nullptr);
}

void vtoolgrp::create_dockable_group()
{
    dismiss = new Fl_Button(2, 2, 11, 11, "@-31+");
    dismiss->box(FL_BORDER_BOX);
    dismiss->tooltip("Dismiss");
    dismiss->clear_visible_focus();
    dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);

    dragger = new drag_btn(15, 2, w()-17, 11);
    dragger->type(FL_TOGGLE_BUTTON);
    dragger->box(FL_ENGRAVED_BOX);
    dragger->tooltip("Drag Box");
    dragger->clear_visible_focus();
    dragger->when(FL_WHEN_CHANGED);

    inner_group = new Fl_Group(2, 15, w() - 4, h() - 17);
    inner_group->box(FL_NO_BOX);
}

void vtoolgrp::create_fixed_group() {
    inner_group = new Fl_Group(2, 15, w() - 4, h() - 17);
    inner_group->box(FL_NO_BOX);
}
