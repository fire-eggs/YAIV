#include <stdio.h>
#include <FL/Fl.H>

#include "dropwin.h"
//#include "event_header.h"
#  define FX_DROP_EVENT	(FL_DND_RELEASE + 100)  // TODO tb hack
#  define DROP_REGION_HEIGHT 40 //39

//#include "mediator.h"

// basic fltk constructors
dropwin::dropwin(int x, int y, int w, int h, const char *l) 
  : Fl_Double_Window(x, y, w, h, l) 
{
	init_dropwin();
}

dropwin::dropwin(int w, int h, const char *l) 
  : Fl_Double_Window(w, h, l) 
{
	init_dropwin();
}

void dropwin::init_dropwin(void)
{
	dock = (dockgroup *)0;
	workspace = (Fl_Group *)0;
}

void dropwin::dock_resize(int delta_h)
{
	int xo = workspace->x();
	int yo = workspace->y();
	int wo = workspace->w();
	int ho = workspace->h();
	
	yo = yo - delta_h;
	ho = ho + delta_h;
	workspace->resize(xo, yo, wo, ho);
	workspace->redraw();
	redraw();
}

int dropwin::handle(int evt)
{
	int res = Fl_Double_Window::handle(evt);
	
  if (evt == FL_KEYDOWN)
  {
    printf("DW:key %d\n", Fl::event_key());
    //handle_key(); // TODO tb mediator
    //send_message(MSGS::KEY, Fl::event_key());
    return 1;
  }
  
	// Is this a dock_drop event?
	if((evt == FX_DROP_EVENT) && (dock))
	{
		printf("Got Drop Event - ");
		// Did the drop happen on us?
		// Get our co-ordinates
		int ex = x_root() + dock->x();
		int ey = y_root() + dock->y();
		int ew = dock->w();
//		int eh = dock->h(); // for non-resizing dock 
		int eh = DROP_REGION_HEIGHT; // fixed drop-zone for re-sizing dock
		// get the drop event co-ordinates
		int cx = Fl::event_x_root();
		int cy = Fl::event_y_root();
		
		// Is the event inside the boundary of this window?
		if(visible() && (cx > ex) && (cy > ey)
		&& (cx < (ew + ex)) && (cy < (eh + ey)))
		{
			printf("Accepted\n");
			res = 1;
		}
		else
		{
			printf("Rejected\n");
			res = 0;
		}
		fflush(stdout);
	}
	return res;
}
// end of file
