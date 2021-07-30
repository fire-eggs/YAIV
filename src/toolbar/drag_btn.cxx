#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include "drag_btn.h"

#  define FX_DROP_EVENT	(FL_DND_RELEASE + 100)  // TODO tb hack
#  define FX_DRAG_EVENT	(FL_DND_DRAG + 100)  // TODO tb hack

#  define DROP_REGION_HEIGHT 42 //39

#include "toolgrp.h"

#include "dock_grip_tile.xpm"

drag_btn::drag_btn(int x, int y, int w, int h, const char *l)
 : Fl_Box(x, y, w, h, l) 
{
	was_docked = 0; // Assume we have NOT just undocked...
	x1 = y1 = xoff = yoff = 0;
}

void drag_btn::draw()
{
	int xo = x();
	int yo = y();

	// Draw the button box
	draw_box(box(), color());

	// set the clip region so we only "tile" the box
	fl_push_clip(xo+1, yo+1, w()-3, h()-3);

	// tile the pixmap onto the button face... there must be a better way
	for(int i = 2; i <= w(); i += 6)
		for(int j = 2; j <= h(); j += 6)
			fl_draw_pixmap(grip_tile_xpm, (xo + i), (yo + j));

	fl_pop_clip();
} // draw

#define DRAG_MIN 10

int win_event(int event, int cx, int cy, Fl_Window **outwin)
{
    for(Fl_Window *win = Fl::first_window(); win; win = Fl::next_window(win))
    {
        // Get the co-ordinates of each window
        int ex = win->x_root();
        int ey = win->y_root();
        int ew = win->w();
        int eh = win->h();

        // Are we inside the boundary of the window?
        if(win->visible() && (cx > ex) && (cy > ey)
           && (cx < (ew + ex)) && (cy < (eh + ey)))
        {	// Send the found window a message that we want to dock with it.
            if(Fl::handle(event, win))
            {
                *outwin = win; // provide the destination window
                return true;
            }
        }
    }
    return false;
}

int drag_btn::handle(int event) 
{
 	toolgrp *tg = (toolgrp *)parent();
	int docked = tg->docked();
	int ret = 0;
	int x2 = 0, y2 = 0;

	// If we are not docked, deal with dragging the toolwin around
	if (!docked)
	{
		// get the enclosing parent widget
		Fl_Widget *tw = (Fl_Widget *)(tg->parent());
		if(!tw) return 0;
	
		switch (event) 
		{
		case FL_PUSH: // downclick in button creates cursor offsets
			x1 = Fl::event_x_root();
			y1 = Fl::event_y_root();
			xoff = tw->x() - x1;
			yoff = tw->y() - y1;
			ret = 1;
			break;
		
		case FL_DRAG: // drag the button (and its parent window) around the screen
			if(was_docked)
			{	// Need to init offsets, we probably got here following a drag 
				// from the dock, so the PUSH (above) will not have happened.
				was_docked = 0;
				x1 = Fl::event_x_root();
				y1 = Fl::event_y_root();
				xoff = tw->x() - x1;
				yoff = tw->y() - y1;
			}
			else
            {
			    // Let the window we're dragging over know, in case it wants to react
                int cx = Fl::event_x_root();
                int cy = Fl::event_y_root();
                x2 = x1 - cx;
                y2 = y1 - cy;
                x2 = (x2 > 0) ? x2 : (-x2);
                y2 = (y2 > 0) ? y2 : (-y2);
                if ((x2 > DRAG_MIN) || (y2 > DRAG_MIN))
                {
                    Fl_Window *outwin = nullptr;
                    win_event(FX_DRAG_EVENT, cx, cy, &outwin);
                }
            }
			tw->position(xoff + Fl::event_x_root(), yoff + Fl::event_y_root());
			tw->redraw();
			ret = 1;
			break;
		
		case FL_RELEASE: {
            int cx = Fl::event_x_root(); // Where did the release occur...
            int cy = Fl::event_y_root();
            x2 = x1 - cx;
            y2 = y1 - cy;
            x2 = (x2 > 0) ? x2 : (-x2);
            y2 = (y2 > 0) ? y2 : (-y2);
            if ((x2 > DRAG_MIN) || (y2 > DRAG_MIN)) {    // test for a docking event
                Fl_Window *win = nullptr;
                if (win_event(FX_DROP_EVENT, cx, cy, &win)) {
                    tg->dock_grp(tg);

                    // Position the toolbar within the dockgroup
                    tg->position(1, cy - win->y_root()); // TODO currently vertical only!
                    return 1;
                }
            }
            ret = 1;
            break;
        }
		default:
			break;
		}
		return(ret);
	}
	
	// OK, so we must be docked - are we being dragged out of the dock?
	switch(event) 
	{
	case FL_PUSH: // downclick in button creates cursor offsets
		x1 = Fl::event_x_root();
		y1 = Fl::event_y_root();
		ret = 1;
		break;

	case FL_DRAG:
		// IF the drag has moved further than the drag_min distance
		// THEN invoke an un-docking
		x2 = Fl::event_x_root() - x1;
		y2 = Fl::event_y_root() - y1;
		x2 = (x2 > 0) ? x2 : (-x2);
		y2 = (y2 > 0) ? y2 : (-y2);
		if ((x2 > DRAG_MIN) || (y2 > DRAG_MIN))
		{
			tg->undock_grp((void *)tg); // undock the window
			was_docked = -1; // note that we *just now* undocked
		}
		ret = 1;
		break;

	default:
		break;
	}
	return ret;
} // handle

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
