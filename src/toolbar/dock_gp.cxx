#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

#include "dock_gp.h"
#include "dropwin.h"

// basic fltk constructors
dockgroup::dockgroup(bool vertical, int x, int y, int w, int h, const char *l)
  : Fl_Group(x, y, w, h, l) 
{
    holder = new Fl_Group(x, y, w, h);
	children = 0;
	vis_h = h;
	vis_w = w;
	win = nullptr;
	_vertical = vertical;
}

void dockgroup::openDock()
{
    if (_vertical) {
        int wi = w();
        if (wi < vis_w)
        {
            dropwin *dw = (dropwin *)win;
            size(vis_w, h());
            holder->size(vis_w, h());
            dw->dock_resizeV(wi - vis_w);
        }
    }
    else {
        int ht = h();
        if (ht < vis_h)
        {
            dropwin *dw = (dropwin *)win;
            size(w(), vis_h);
            holder->size(w(), vis_h);
            dw->dock_resize(ht - vis_h);
        }
    }
}

void dockgroup::closeDock()
{
    if (children > 0)
        return;
    children = 0;
    if (_vertical) {
        if (w() >= vis_w) {
            dropwin *dw = (dropwin *) win;
            size(3, h());
            dw->dock_resizeV(vis_w - 3);
        }
    }
    else {
        if (h() >= vis_h ) {
            dropwin *dw = (dropwin *) win;
            size(w(), 3);
            dw->dock_resize(vis_h - 3);
        }
    }
}

void dockgroup::add(Fl_Widget *grp)
{
#if true
	openDock();
#else
	int wd = w();
	int ht = h();
	// if the dock is "closed", open it back up
	if (ht < vis_h)
	{
		dropwin *dw = (dropwin *)win;
		size(wd, vis_h);
		holder->size(wd, vis_h);
		dw->dock_resize(ht - vis_h);
	}
#endif
	holder->add(grp);
	children++;
}

void dockgroup::remove(Fl_Widget *grp)
{
	holder->remove(grp);
	children--;
#if true
	closeDock();
#else
	int wd = w();
	// If the dock is empty, close it down
	if (children <= 0)
	{
		dropwin *dw = (dropwin *)win;
		children = 0; // just in case...!
		size(wd, 3);
		dw->dock_resize(vis_h - 3);
	}
#endif
}

bool dockgroup::contains(Fl_Widget *grp)
{
    int res = find(grp);
    return res < children;
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
