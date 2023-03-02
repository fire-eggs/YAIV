#include <string>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>

#include "prefs.h"
#include "ScaleMode.h"
#include "tkScaleMode.h"

// TODO not modal

Fl_Double_Window *_optDlg;
static Prefs *_prefs = nullptr; 

#define TAB_Y 40


// TODO duplicated from buttonBar.cpp
static Fl_Menu_Item scale_menu[6] =
{
    {"No scale",        0, nullptr, (void *)(fl_intptr_t) ScaleMode::Noscale},
    {"Auto scale",      0, nullptr, (void *)(fl_intptr_t) ScaleMode::Auto},
    {"Scale to Fit",    0, nullptr, (void *)(fl_intptr_t) ScaleMode::Fit},
    {"Scale to Width",  0, nullptr, (void *)(fl_intptr_t) ScaleMode::Wide},
    {"Scale to Height", 0, nullptr, (void *)(fl_intptr_t) ScaleMode::High},
    {nullptr} // end of menu
};



class CheckSetting
{
public:
    Fl_Check_Button *chk;
    std::string setting;
    int def_value;
    
    CheckSetting(const char *setstr, int val)
    {
        setting = setstr;
        def_value = val;
    }

    void init(Fl_Check_Button *w)
    {
        chk = w;
        int val;
        _prefs->get(setting.c_str(), val, def_value);
        chk->value(val);
    }
    
    void save()
    {
        int val = chk->value();
        _prefs->set2(setting.c_str(), val);
    }
};

CheckSetting _checker(CHECKER, 1);
CheckSetting _panMouse(MOUSE_PAN, 1);
CheckSetting _border(BORDER_FLAG, 1);
CheckSetting _initShuffle("LOAD_SHUFFLE", 0);

CheckSetting _slideShuffle(SLIDE_SHUFFLE, 0);
CheckSetting _slideBorder(SLIDE_BORDER, 1);
CheckSetting _slideSkipError(SLIDE_ERRORS, 1);

void makeGeneralTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "General");
    
    Fl_Check_Button *cb1 = new Fl_Check_Button(15, 65, 250, 30, "Show checker background");
    _checker.init(cb1);
    
    int val;
    Fl_Check_Button *cb2 = new Fl_Check_Button(15, 100, 250, 30, "Show overlay");
    _prefs->get(OVERLAY, val, 1);
    cb2->value(val);
    
    Fl_Check_Button *cb3 = new Fl_Check_Button(15, 135, 250, 30, "Show minimap");
    _prefs->get(MINIMAP, val, 1);
    cb3->value(val);

    Fl_Check_Button *cb4 = new Fl_Check_Button(15, 170, 250, 30, "Pan with mouse");
    _panMouse.init(cb4);

    Fl_Check_Button *cb5 = new Fl_Check_Button(15, 205, 250, 30, "Show border");
    _border.init(cb5);

    Fl_Check_Button *cb6 = new Fl_Check_Button(15, 240, 250, 30, "Shuffle on load");
    _initShuffle.init(cb6);
    
    Fl_Box *filler = new Fl_Box(15, h, 0, 0);
    o->end();
    o->resizable(filler);
}

void makeSlideshowTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "Slideshow");
        
    Fl_Box* b = new Fl_Box(15, 65, 100, 30, "Timing:");
    b->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP_RIGHT);
    
    // TODO spin button
    Fl_Int_Input* ii = new Fl_Int_Input(116, 65, 100, 30);
    
    Fl_Box* b2 = new Fl_Box(217, 65, 100, 30, "seconds");

    Fl_Check_Button *cb1 = new Fl_Check_Button(15, 100, 100, 30, "Shuffle");
    _slideShuffle.init(cb1);

    Fl_Check_Button *cb2 = new Fl_Check_Button(15, 135, 100, 30, "Show Border");
    _slideBorder.init(cb2);

    Fl_Check_Button *cb3 = new Fl_Check_Button(15, 170, 100, 30, "Skip error files");
    _slideSkipError.init(cb3);
    
    o->end();
}

void makeScaleTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "Scaling");
    
    std::string defaultScale;
    
    Fl_Choice *scaleC = new Fl_Choice(170, 65, 150, 30, "Default Scale:");
    scaleC->menu(scale_menu);
    _prefs->getS(SCALE_MODE, defaultScale, scaleModeToName(ScaleMode::Noscale));
    ScaleMode sm = nameToScaleMode(defaultScale);
    scaleC->value(sm);

    Fl_Choice *ditherC = new Fl_Choice(170, 100, 150, 30, "Default Dither:");
    for (int i= ZScaleMode::None; i < ZScaleModeMAX; i++)
    {
        ZScaleMode z = static_cast<ZScaleMode>(i);
        std::string n = zScaleModeToName(z);
        ditherC->add(n.c_str());
    }
    _prefs->getS(DITHER_MODE, defaultScale, zScaleModeToName(ZScaleMode::None));
    ZScaleMode zm = nameToZScaleMode(defaultScale);
    ditherC->value(zm);
    
    o->end();
}

void makeThemeTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "Theme");
    o->end();
}


void OnCancel(Fl_Widget *,void *) { _optDlg->hide(); }

void On_OK(Fl_Widget *, void *)
{
    _checker.save(); // TODO immediate impact
    _panMouse.save();
    _border.save(); // TODO immediate impact
    _initShuffle.save();

    _slideShuffle.save();
    _slideBorder.save();
    _slideSkipError.save();
    
    _optDlg->hide();
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
    
    Fl_Tabs *tabs = new Fl_Tabs(10, 10, w-30, h-50);
    
    makeGeneralTab(w, h);
    makeSlideshowTab(w, h);
    makeScaleTab(w, h);
    makeThemeTab(w, h);
    
    tabs->end();
    
    _optDlg->resizable(tabs);
    
    Fl_Button *ok = new Fl_Button(10, h-30, 50, 25, "OK");
    ok->callback(On_OK);
    Fl_Button *cancel = new Fl_Button(90, h-30, 100, 25, "Cancel");
    cancel->callback(OnCancel);
    
    _optDlg->end();
    _optDlg->set_modal();
    
    Fl_Group::current(previous_group);
}

void showOptionsDlg()
{
    _prefs = new Prefs();
    
    if (!_optDlg)
        makeOptionsDlg();
        
    // TODO this section should be a utility function
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
