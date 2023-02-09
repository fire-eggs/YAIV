//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_XBOX_H
#define CLION_TEST2_XBOX_H

#include <string>
#include <cstring>
#include <climits> // INT_MAX, INT_MIN

#include <FL/Fl_Group.H>
#include <Fl_Anim_GIF_Image.h>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_draw.H>
#include <FL/filename.H>

//#include "XBoxDisplayInfoEvent.h"
#include "ScaleMode.h"
#include "tkScaleMode.h"
#include "overlayMode.h"

#if (FL_MINOR_VERSION<4)
    #error "Error, required FLTK 1.4 or later"
#endif

#include "SmoothResizeGroup.h"
#include "humansize.h"
#include "checker.h"

#include "MostRecentPaths.h"
#include "menuids.h"

class Slideshow;
class XBoxDisplayInfoEvent;

class XBox : public SmoothResizeGroup
{
public:
    void MenuCB(Fl_Widget *window_p, int menuid);

private:
    Fl_Image *_img{}; // the original image loaded from disk
    Fl_RGB_Image *_showImg{}; // the "back-buffer" image: rotated, scaled, checkered
    Fl_Anim_GIF_Image *_anim{}; // an original animation loaded from disk

    bool draw_check{true};
    ScaleMode draw_scale{Noscale};
    OverlayMode draw_overlay{OverlayNone};
    double _zoom{1.0};
    int _zoom_step = 0;
    int _scroll_speed = 20;

    // Location of center of image
    int centerX{0};
    int centerY{0};

    int rotation; // cycle through clockwise rotations of 90 degrees
    ZScaleMode imgtkScale{ZScaleMode::None}; // cycle through fl_imgtk scale values

    char _message[128];  // HACK hard-coded message length

// TODO consider building the menu in code
#ifdef DANBOORU
  #ifdef METADATA
    #define MNU_COUNT 20
  #else
    #define MNU_COUNT 19
  #endif
#else
  #ifdef METADATA
    #define MNU_COUNT 19
  #else
    #define MNU_COUNT 18
  #endif
#endif

    Fl_Menu_Item right_click_menu[MNU_COUNT] =
    {
        {"Load",            0, nullptr, (void *)(fl_intptr_t) MI_LOAD},
        {"Copy image path", 0, nullptr, (void *)MI_COPYPATH},
        {"Goto Image",      0, nullptr, (void *)MI_GOTO},
#ifdef DANBOORU
        {"Show Danbooru",    0, nullptr, (void *)MI_DANBOORU, FL_MENU_DIVIDER},
#endif
#ifdef METADATA
        {"Show Metadata",    0, nullptr, (void *)MI_METADATA, FL_MENU_DIVIDER},
#endif

        {"Theme", 0, nullptr, nullptr, FL_SUBMENU},
            {"Blue", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_BLUE},
            {"Classic", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_CLASSIC},
            {"Dark", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_DARK},
            {"Grey Bird", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_GREYBIRD},
            {"High Contrast", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_HIGHCONTRAST},
            {"Native", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_NATIVE},
            {"Ocean", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_OCEAN},
            {"Olive", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_OLIVE},
            {"Rose Gold", 0, nullptr, (void *)(fl_intptr_t)MI_THEME_ROSEGOLD},
            {nullptr}, // end of sub menu
        {"Options",         0, nullptr, (void *)MI_OPTIONS, FL_MENU_DIVIDER},

        {"Last Used",       0, nullptr, nullptr, FL_SUBMENU},
            {nullptr}, // end of sub menu
        {nullptr} // end of menu
    };

public:
    XBox(int x, int y, int w, int h, Prefs*);

    void forceSlideshow(); // command line override
    void forceQuitAtEnd() { _quitAtEnd = true; }; // command line
    void forceScale(const char *);
    void forceDither(const char *);

    int handle(int) override;
    const char * getLabel(bool include_filepath, char *buff, int buffsize);

    void change_zoom(int delta) {_zoom_step += delta; updateImage(); updateLabel();}

    void draw() override;

    void do_menu(int, int, bool);
    void image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg);

    void load_current(); // exposed for slideshow
    void next_image();   // exposed for slideshow

    void load_file(const char *n); // exposed for argv processing

    void action(int val); // action from someplace (e.g. toolbar button)

    void hideCurrent();
    void favCurrent();

    void updateLabel();
    void updateDisplay();

    void wipeShowImage(bool force = false);
    
private:

    bool _quitAtEnd {false};

    void load_request(); // 'load' processing
    void goto_request(); // 'goto' processing

    void prev_image();
    void next_scale();
    void nextTkScale();
    void nextRotation();
    void updateImage();

    void drawMinimap();
    void drawOverlay();
    void drawCenter();
    void drawMessage();

    void toggleSlideshow();
    void toggleMinimap();
    void toggleOverlay();
    
    void showUserMessage(const char *);
    void clearUserMessage();
    void clearUserMesssageLater();
    static void cb_UserMsgTimeout(void *);

    bool determineTargetSize(int imgW, int imgH, int winW, int winH, int& targetW, int& targetH);
    
private:
    bool _pan_with_mouse;
    int dragStartX; // mouse panning state
    int dragStartY; // mouse panning state
    bool dragging;  // while mouse dragging
    int mousePan(int);

    bool _inSlideshow;
    Slideshow *_slideShow;

    bool _minimap;
    static Fl_Color _mmoc;
    static Fl_Color _mmic;
    static int _miniMapSize;

    MostRecentPaths* _mru;
    Prefs* _prefs;

    void safe_resize() override;

private:
    std::vector<XBoxDisplayInfoEvent *> *_dispevents {nullptr};

    void notifyActivate(bool val);
    void notifyBorder();
    void notifyDisplayInfo(const char *);
    void notifyDisplayLabel(const char *);

public:
    bool getCheck() const { return draw_check; }
    bool inSlide() const {return _inSlideshow; }

    void displayEventHandler(XBoxDisplayInfoEvent *hand)
    {
        if (!_dispevents)
            _dispevents = new std::vector<XBoxDisplayInfoEvent *>();
        _dispevents->push_back(hand);
    }
    
private:
    void activate_working(const char *);
    void deactivate_working();
    static void timeoutCallback(void *, const char *);
};

#endif //CLION_TEST2_XBOX_H
