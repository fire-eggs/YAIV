#ifndef _HAVE_DOCK_GRP_HDR_
#define _HAVE_DOCK_GRP_HDR_

#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>

class dockgroup : public Fl_Group
{
protected:
	Fl_Window *win;
	Fl_Group *holder;
	int children;
	int vis_h;
	int vis_w;
    bool _vertical;

public:
	// Normal FLTK constructors
	dockgroup(bool vertical, int x, int y, int w, int h, const char *l = 0);
	
	// point back to our parent
	void set_window(Fl_Window *w) {win = w;}

	// methods for adding or removing toolgroups from the dock
	void add(Fl_Widget *w);
	void remove(Fl_Widget *w);

	void openDock();
	void closeDock();

	bool contains(Fl_Widget*);

	int target_w() { return _vertical ? vis_w : w() ; }
    int target_h() { return _vertical ? h() : vis_h ; }
};

#endif // _HAVE_DOCK_GRP_HDR_

// End of file //
