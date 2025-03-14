#ifndef _HAVE_TOOL_GROUP_HDR_
#define _HAVE_TOOL_GROUP_HDR_

/* fltk includes */
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

#include "dock_gp.h"
#include "drag_btn.h"

class toolgrp : public Fl_Group
{
protected:
	// control variables
	short _docked;
	dockgroup *dock;

    // constructor helper function
	virtual void create_dockable_group();
	virtual void create_fixed_group();

	void create_docked(dockgroup *d);
	void create_floating(dockgroup *d, int state, int x, int y, int w, int h, const char *l, const char *xclass);
    void create_fixed_docked(dockgroup *d);

    void initialize(dockgroup *dk, bool floater, bool draggable, int w, int h, const char *lbl, const char *xclass);
    void initialize(dockgroup *dk, bool floater, bool draggable, int x, int y, int w, int h, const char *lbl, const char *xclass);

protected:
	// Widgets used by the toolbar
	Fl_Button *dismiss;
	drag_btn *dragger;
	Fl_Group *inner_group;

	// Sets whether window is docked or not.
	void docked(short r);
	
	// Defines which dock the group can dock into
	void set_dock(dockgroup *w) {dock = w;}

public:
	// Constructors for docked/floating window
	toolgrp(int w, int h, const char *l = nullptr);
	toolgrp(dockgroup *d, bool floating, bool draggable, int w, int h, const char *l = nullptr);
	toolgrp(dockgroup *d, int f, int x, int y, int w, int h, const char *l = nullptr, const char *xclass=nullptr);

	// methods for hiding/showing *all* the floating windows
	static void show_all();
	static void hide_all();

	// Tests whether window is docked or not.
	short docked() const { return _docked; }

	// generic callback function for the dock/undock checkbox
	void dock_grp(void* v);
	void undock_grp(void* v);

  Fl_Group *in_group() {return inner_group;} // KBR hack
  bool shown();

	// wrap some basic Fl_Group functions to access the enclosed inner_group
	inline void begin() {inner_group->begin(); }
	inline void end() {inner_group->end(); Fl_Group::end(); }
	inline void resizable(Fl_Widget *box) {inner_group->resizable(box); }
	inline void resizable(Fl_Widget &box) {inner_group->resizable(box); }
	inline Fl_Widget *resizable() const { return inner_group->resizable(); }
	inline void add( Fl_Widget &w ) { inner_group->add( w ); }
	inline void add( Fl_Widget *w ) { inner_group->add( w ); }
	inline void insert( Fl_Widget &w, int n ) { inner_group->insert( w, n ); }
	inline void insert( Fl_Widget &w, Fl_Widget* beforethis ) { inner_group->insert( w, beforethis ); }
	inline void remove( Fl_Widget &w ) { inner_group->remove( w ); }
	inline void remove( Fl_Widget *w ) { inner_group->remove( w ); }
//	inline void add_resizable( Fl_Widget &box ) { inner_group->add_resizable( box ); }

  //int handle(int evt) override;
// get the dock group ID
dockgroup *get_dock() {return dock;}

    // generic callback function for the dismiss button
	static void cb_dismiss(Fl_Button*, void* v);

	void setPrefs(void *, const char *magic);
    
};

#endif // _HAVE_TOOL_GROUP_HDR_

// End of file //
