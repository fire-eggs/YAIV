
#include <FL/Fl_Button.H>
#include "toolwin.h"
#include "mediator.h"
#include "prefs.h"

#  define FX_DROP_EVENT	(FL_DND_RELEASE + 100)  // TODO tb hack
#  define DROP_REGION_HEIGHT 42 //39

#define NTW (toolwin*)0  // Null Tool Window

// TODO HACK:: This just stores the toolwindows in a static array. I'm too lazy
//        to make a proper linked list to store these in...
toolwin* toolwin::active_list[TW_MAX_FLOATERS]; // list of active toolwins
short toolwin::active = 0; // count of active tool windows

// Dummy close button callback
static void cb_ignore()
{
	// Just shrug off the close callback...
}

// constructors
toolwin::toolwin(int x, int y, int w, int h, const char *l) 
  : Fl_Double_Window(x, y, w, h, l) 
{
	create_dockable_window();
}

toolwin::toolwin(int w, int h, const char *l) 
  : Fl_Double_Window(w, h, l) 
{
	create_dockable_window();
}

toolwin::~toolwin()
{
	active_list[idx] = NTW;
	active --;
}

// construction function
void toolwin::create_dockable_window() 
{
	static int first_window = 1;
	tool_group = nullptr; //(void *)0;
	// window list intialisation...
	// this is a nasty hack, should make a proper list
	if(first_window)
	{
		first_window = 0;
		for(short i = 0; i < TW_MAX_FLOATERS; i++)
			active_list[i] = NTW;
	}
	// find an empty index
	for(short i = 0; i < TW_MAX_FLOATERS; i++)
	{
		if(!active_list[i])
		{
			idx = i;
			active_list[idx] = this;
			active ++;
			// TODO tb need to start w/ border and turn it off later? KBR TEST
			clear_border();
			set_non_modal();
			callback((Fl_Callback *)cb_ignore);
			return;
		}
	}
	// if we get here, the list is probably full, what a hack.
	// FIX THIS:: At present, we will get a non-modal window with
	// decorations as a default instead...
	set_non_modal();
}

// show all the active floating windows
void toolwin::show_all()
{
	if (active)
	{
		for(short i = 0; i < TW_MAX_FLOATERS; i++)
		{
			if(active_list[i])
				active_list[i]->show();
		}
	}
}

// hide all the active floating windows
void toolwin::hide_all()
{
	if (active)
	{
		for(short i = 0; i < TW_MAX_FLOATERS; i++)
		{
			if(active_list[i])
				active_list[i]->hide();
		}
	}
}

int toolwin::handle(int event)
{
  int res = Fl_Double_Window::handle(event);

  if (event == FL_KEYDOWN)
  {
    Mediator::handle_key();
    return 1;
  }

  if (event == FL_FOCUS)
      return 0;

  if (event == FL_SHOW)
  {
      border(0); // can't be done until show?
  }

  return res;
}

void toolwin::setPrefs(void *p, const char *pval)
{
    _prefs = p;
    _prefsVal = pval;
}

void toolwin::saveWindowPos()
{
    Prefs *p = static_cast<Prefs *>(_prefs);
    p->setWinRect(_prefsVal, x(), y(), w(), h());
}

// end of file
