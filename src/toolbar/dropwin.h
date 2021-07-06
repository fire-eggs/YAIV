#ifndef _HAVE_DROP_WIN_HDR_
#define _HAVE_DROP_WIN_HDR_

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>

#include "dock_gp.h"

class dropwin : public Fl_Double_Window
{
protected:
	void init_dropwin(void);
	dockgroup *dock;

public:
	// Normal FLTK constructors
	dropwin(int x, int y, int w, int h, const char *l = 0);
	dropwin(int w, int h, const char *l = 0);

	// The working area of this window
	Fl_Group *workspace;

	// override handle method to capture "drop" events
	int handle(int);

	// assign a dock widget to this window
	void set_dock(dockgroup *d) {dock = d;}
    dockgroup *get_dock() { return dock; }

	// Resize the workspace area if the dock closes/opens
	void dock_resize(int h);
};

#endif // _HAVE_DROP_WIN_HDR_

// End of file //
