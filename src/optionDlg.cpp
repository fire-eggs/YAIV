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

template <typename T> class EnumSetting
{
public:
    Fl_Choice *combo;
    std::string setting;
    T def_value;

    // TODO typedefs
    std::string (*convertToString)(T);
    T (*convertFromString)(const std::string& s);
    
    EnumSetting(const char *setstr, T val, std::string (*cvtTo)(T), T (*cvtFrom)(const std::string& s))
    {
        setting = setstr;
        def_value = val;
        
        convertToString = cvtTo;
        convertFromString = cvtFrom;
    }

    void init(Fl_Choice *w)
    {
        combo = w;
        for (int i=0; i < T::LAST; i++) // NOTE enum must define LAST, first value must be 0
        {   
            std::string textVal = convertToString(static_cast<T>(i));
            combo->add(textVal.c_str());
        }
    }
    
    void load()
    {
        std::string textVal;
        std::string defVal = convertToString(def_value);
        _prefs->getS(setting.c_str(), textVal, defVal.c_str());
        T enumVal = convertFromString(textVal);
        combo->value(enumVal);
    }
    
    void save()
    {
        int selValue = combo->value();
        T enumVal = static_cast<T>(selValue);
        std::string enumText = convertToString(enumVal);
        _prefs->set(setting.c_str(), enumText.c_str());
    };
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

CheckSetting _showMinimap(MINIMAP, 1);
CheckSetting _showOverlay(OVERLAY, 1);

Fl_Choice *_scaleChoice;
EnumSetting<ZScaleMode> _zScaleMode(DITHER_MODE, ZScaleMode::Bilinear, zScaleModeToName, nameToZScaleMode);

void makeGeneralTab(int w, int h)
{
    Fl_Group *o = new Fl_Group(10, TAB_Y, w-20, h-TAB_Y, "General");
    
    Fl_Check_Button *cb1 = new Fl_Check_Button(15, 65, 250, 30, "Show checker background");
    _checker.init(cb1);

    // TODO choices: Off, On-Variant1, On-Variant2
    int val;
    Fl_Check_Button *cb2 = new Fl_Check_Button(15, 100, 250, 30, "Show overlay");
    _showOverlay.init(cb2);
    
    // TODO choices: Off, On, On-when-zoom
    Fl_Check_Button *cb3 = new Fl_Check_Button(15, 135, 250, 30, "Show minimap");
    _showMinimap.init(cb3);

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
    
    std::string scaleName;
    
    _scaleChoice = new Fl_Choice(170, 65, 150, 30, "Default Scale:");
    _scaleChoice->menu(scale_menu);
    _prefs->getS(SCALE_MODE, scaleName, scaleModeToName(ScaleMode::Noscale));
    ScaleMode sm = nameToScaleMode(scaleName);
    _scaleChoice->value(sm);

    Fl_Choice *ditherChoice = new Fl_Choice(170, 100, 150, 30, "Default Dither:");
    _zScaleMode.init(ditherChoice);
    _zScaleMode.load();

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

    _showMinimap.save();
    _showOverlay.save();
    
    // TODO how to encapsulate this using an enum TYPE
    int scaleValue = _scaleChoice->value();
    ScaleMode sm = static_cast<ScaleMode>(scaleValue);
    std::string scaleName = scaleModeToName(sm);
    _prefs->set(SCALE_MODE, scaleName.c_str());

    _zScaleMode.save();

    _prefs->flush();
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
