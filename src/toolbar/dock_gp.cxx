#include <stdio.h>

#include <FL/Fl.H>

#include "dock_gp.h"
#include "dropwin.h"

// basic fltk constructors
dockgroup::dockgroup(int x, int y, int w, int h, const char *l) 
  : Fl_Group(x, y, w, h, l) 
{
	pack = new Fl_Pack(x, y, w, h);
	pack->type(Fl_Pack::HORIZONTAL);
	children = 0;
	vis_h = h;
}

void dockgroup::add(Fl_Widget *grp)
{
	int wd = w();
	int ht = h();
	
	// if the dock is "closed", open it back up
	if (ht < vis_h)
	{
		dropwin *dw = (dropwin *)win;
		size(wd, vis_h);
		pack->size(wd, vis_h);
		dw->dock_resize(ht - vis_h);
	}
	pack->add(grp); 
	children++;
}

void dockgroup::remove(Fl_Widget *grp)
{
	int wd = w();
	pack->remove(grp);
	children--;
	// If the dock is empty, close it down
	if (children <= 0)
	{
		dropwin *dw = (dropwin *)win;
		children = 0; // just in case...!
		size(wd, 3);
		dw->dock_resize(vis_h - 3);
	}
}

void dockgroup::dock_check(void)
{
	printf("DG: %d - %dx%d (%dx%d)\n", children, pack->w(), pack->h(), w(), h());
	fflush(stdout);
}

// end of file
