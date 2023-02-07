#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Check_Button.H>

// TODO not modal

Fl_Double_Window *_optDlg;

#define TAB_Y 40

void makeGeneralTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "General");
    
    Fl_Check_Button *cb1 = new Fl_Check_Button(15, 65, 250, 30, "Show checker background");
    
    Fl_Check_Button *cb2 = new Fl_Check_Button(15, 100, 250, 30, "Show overlay");
    
    Fl_Check_Button *cb3 = new Fl_Check_Button(15, 135, 250, 30, "Show minimap");
    
    Fl_Box *filler = new Fl_Box(15, h, 0, 0);
    o->end();
    o->resizable(filler);
}

void makeSlideshowTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "Slideshow");
    
//    Fl_Flex flex1(15, 15, 200, 30, Fl_Flex::HORIZONTAL); 
    
    Fl_Box* b = new Fl_Box(15, 65, 100, 30, "Timing:");
    b->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP_RIGHT);
    
    // TODO spin button
    Fl_Int_Input* ii = new Fl_Int_Input(116, 65, 100, 30);
    
    Fl_Box* b2 = new Fl_Box(217, 65, 100, 30, "seconds");

    Fl_Check_Button *cb1 = new Fl_Check_Button(15, 100, 100, 30, "Shuffle");

    Fl_Check_Button *cb2 = new Fl_Check_Button(15, 135, 100, 30, "Borderless");

    Fl_Check_Button *cb3 = new Fl_Check_Button(15, 170, 100, 30, "Skip error files");
    
    o->end();
    //o->resizable();
}


void makeOptionsDlg()
{
    
  Fl_Group *previous_group = Fl_Group::current();
  if (previous_group)
    Fl_Group::current(0);
    
    
    // TODO size, location from options
    _optDlg = new Fl_Double_Window(600,600);
    int h = _optDlg->h();
    int w = _optDlg->w();
    
    Fl_Tabs *tabs = new Fl_Tabs(10, 10, w-30, h-20);
    
    makeGeneralTab(w, h);
    makeSlideshowTab(w, h);
    
    tabs->end();
    
    _optDlg->resizable(tabs);
    _optDlg->end();
    _optDlg->set_modal();
    
    Fl_Group::current(previous_group);
}

void initSettings()
{
    // set state of controls from preferences
}

void showOptionsDlg()
{
    if (!_optDlg)
        makeOptionsDlg();
    
    initSettings();
    
    Fl_Window *g = Fl::grab();
    if (g)
        Fl::grab(0);
    Fl_Group *current_group = Fl_Group::current(); // make sure the dialog does not interfere with any active group
    Fl_Group::current(0);
    _optDlg->show();
    Fl_Group::current(current_group);
    while (_optDlg->shown())
        Fl::wait();
    if (g) // regrab the previous popup menu, if there was one
        Fl::grab(g);
}
