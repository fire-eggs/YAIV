#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

/* fltk includes */
#include <FL/Fl.H>

#include "toolgrp.h"
#include "toolwin.h"
#include "dock_gp.h"

// function to handle the dock actions
void toolgrp::dock_grp(void* v)
{ // dock CB
	toolgrp *gp = (toolgrp *)v;
	dockgroup *dock = gp->get_dock();

	// we can only dock a group that's not already docked... 
	// and only if a dock exists for it
	if((!gp->docked()) && (dock))
	{	//re-dock the group
		toolwin *cur_parent = static_cast<toolwin *>(gp->parent());
		dock->add(gp); // move the toolgroup into the dock
		dock->redraw();
		gp->docked(-1);    // toolgroup is docked...
		// so we no longer need the tool window.
		cur_parent->hide();
		delete cur_parent;
	}
}

// static CB to handle the undock actions
void toolgrp::undock_grp(void* v) 
{ // undock CB
	toolgrp *gp = (toolgrp *)v;
	dockgroup *dock = gp->get_dock();
	
	if(gp->docked())
	{	// undock the group into its own non-modal tool window
		int w = gp->w();
		int h = gp->h();
		Fl_Group::current(0);
		toolwin *new_parent = new toolwin(Fl::event_x_root() - 10, Fl::event_y_root() - 35, w + 3, h + 3);
		new_parent->end();
		dock->remove(gp);
		new_parent->add(gp);// move the tool group into the floating window
		new_parent->set_inner((void *)gp);
		new_parent->resizable(gp); // KBR for a resizable toolbar

		gp->position(1, 1); // align group in floating window
		new_parent->show(); // show floating window
		gp->docked(0);      // toolgroup is no longer docked
		dock->redraw();     // update the dock, to show the group has gone...
	}
}

// static CB to handle the dismiss action
void toolgrp::cb_dismiss(Fl_Button*, void* v) 
{
	toolgrp *gp = (toolgrp *)v;
	dockgroup *dock = gp->get_dock();
	
	if(gp->docked())
	{	// remove the group from the dock
		dock->remove(gp);
		gp->docked(0);
		dock->redraw();     // update the dock, to show the group has gone...
		Fl::delete_widget(gp);
	}
	else
	{	// remove the group from the floating window, 
		// and remove the floating window
		toolwin *cur_parent = (toolwin *)gp->parent();
		cur_parent->remove(gp);
		//delete cur_parent; // we no longer need the tool window.
		Fl::delete_widget(cur_parent);
		Fl::delete_widget(gp);
	}
}

toolgrp::toolgrp(int w, int h, const char *lbl) : Fl_Group(1,1,w,h,lbl) {}

// Constructors for docked/floating window
// WITH x, y co-ordinates
toolgrp::toolgrp(dockgroup *dk, int floater, int x, int y, int w, int h, const char *lbl)
  : Fl_Group(1, 1, w, h, lbl)
{
  if (floater && !dk)
  {
    create_floating(nullptr,0,0,0,w,h,lbl); // floating and not dockable
  }
	else if((floater) && (dk)) // create floating
	{
		create_floating(dk, 1, x, y, w, h, lbl);
	}
	else if(dk) // create docked
	{
		create_docked(dk);
	}
//	else //do nothing...
}

// WITHOUT x, y co-ordinates
toolgrp::toolgrp(dockgroup *dk, int floater, int w, int h, const char *lbl)
  : Fl_Group(1, 1, w, h, lbl)
{
  if (floater && !dk)
  {
    create_floating(nullptr,0,0,0,w,h,lbl); // floating and not dockable
  }
	else if((floater) && (dk)) // create floating
	{
		create_floating(dk, 0, 0, 0, w, h, lbl);
	}
	else if(dk) // create docked
	{
		create_docked(dk);
	}
//	else //do nothing...
}

// construction function
void toolgrp::create_dockable_group()
{
//	begin();
	dismiss = new Fl_Button(3, 3, 11, 11, "@-31+");
//	dismiss->box(FL_THIN_UP_BOX);
	dismiss->box(FL_BORDER_BOX);
	dismiss->tooltip("Dismiss");
	dismiss->clear_visible_focus();
	dismiss->callback((Fl_Callback*)cb_dismiss, (void *)this);

	dragger = new drag_btn(3, 16, 11, h() - 15);
	dragger->type(FL_TOGGLE_BUTTON);
	dragger->box(FL_ENGRAVED_BOX);
//	dragger->box(FL_BORDER_BOX);
	dragger->tooltip("Drag Box");
	dragger->clear_visible_focus();
	dragger->when(FL_WHEN_CHANGED);
	
	inner_group = new Fl_Group(17, 3, w() - 17, h() - 3);
	inner_group->box(FL_NO_BOX);
	//inner_group->box(FL_ENGRAVED_FRAME);
}

void toolgrp::create_docked(dockgroup *dk)
{
	// create the group itself
	create_dockable_group();
	// place it in the dock
	dk->add(this);
	set_dock(dk); // define where the toolgroup is allowed to dock
	docked(1);	// docked
	dk->redraw();
}

void toolgrp::create_floating(dockgroup *dk, int full, int x, int y, int w, int h, const char *lbl)
{
	toolwin *tw;
	// create the group itself
	create_dockable_group();
	// create a floating toolbar window
	// Ensure the window is not created as a child of its own inner group!
	Fl_Group::current(0);
	if(full)
		tw = new toolwin(x, y, w + 3, h + 3, lbl);
	else
		tw = new toolwin(w + 3, h + 3, lbl);
	//tw->border(2);
	tw->end();
	tw->add(this);  // move the tool group into the floating window
	docked(0);		// NOT docked
	set_dock(dk);	// define where the toolgroup is allowed to dock [nullptr: not dockable]
	tw->set_inner((void *)this);
	tw->resizable(this); // TODO tb
	tw->show();
	Fl_Group::current(inner_group); // leave this group open when we leave the constructor...
}

// function for setting the docked state and checkbox
void toolgrp::docked(short r)
{ 
	_docked = r; 
}

// methods for hiding/showing *all* the floating windows
// show all the active floating windows
void toolgrp::show_all()
{
	toolwin::show_all();
}

// hide all the active floating windows
void toolgrp::hide_all()
{
	toolwin::hide_all();
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
