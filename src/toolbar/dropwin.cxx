//#include <stdio.h>
#include <FL/Fl.H>

#include "dropwin.h"
//#include "event_header.h"
#  define FX_DROP_EVENT	(FL_DND_RELEASE + 100)  // TODO tb hack
#  define FX_DRAG_EVENT	(FL_DND_DRAG + 100)  // TODO tb hack
#  define DROP_REGION_HEIGHT 40 //39

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

void dropwin::init_dropwin()
{
	dock = (dockgroup *)0;
	workspace = (Fl_Group *)0;
}

void dropwin::dock_resize(int delta_h)
{
    //printf("DW: dockrsz %d\n", delta_h);
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

void dropwin::dock_resizeV(int delta_w)
{
    //printf("DW: dockrsz %d\n", delta_h);
    int xo = workspace->x();
    int yo = workspace->y();
    int wo = workspace->w();
    int ho = workspace->h();

    xo = xo - delta_w;
    wo = wo + delta_w;
    workspace->resize(xo, yo, wo, ho);
    workspace->redraw();
    redraw();
}

int dropwin::handle(int event)
{
    if (event == FL_FOCUS) return 0;

    int res = Fl_Double_Window::handle(event);

    if (dock)
    {
        if (event == FX_DROP_EVENT || event == FX_DRAG_EVENT)
        {
            // Get our co-ordinates
            int ex = x_root() + dock->x();
            int ey = y_root() + dock->y();

            int ew = dock->target_w();
            int eh = dock->target_h();
//            int ew = dock->w();
//		int eh = dock->h(); // for non-resizing dock
//            int eh = DROP_REGION_HEIGHT; // fixed drop-zone for re-sizing dock
            // get the drop event co-ordinates
            int cx = Fl::event_x_root();
            int cy = Fl::event_y_root();

            //printf("Got Drag/Drop Event - (%d,%d)->(%d,%d,%d,%d)",cx,cy,ex,ey,(ex+ew),(ey+eh));

            if(visible() && (cx > ex) && (cy > ey) && (cx < (ew + ex)) && (cy < (eh + ey)))
            {
                if (event == FX_DRAG_EVENT)
                    dock->openDock();
//                if (event == FX_DROP_EVENT)
//                  printf("Drop Accepted\n");
//                else {
//                    printf("Drag hit\n");
//                    dock->openDock();
//                }
                res = 1;
            }
            else
            {
                if (event == FX_DRAG_EVENT)
                    dock->closeDock();
//                if (event == FX_DROP_EVENT)
//                    printf("Drop Rejected\n");
//                else {
//                    printf("Drag miss\n");
//                    dock->closeDock();
//                }
                res = 0;
            }
            fflush(stdout);

        }
    }
	return res;
}
// end of file
