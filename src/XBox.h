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

#include "XBoxDisplayInfoEvent.h"

#if (FL_MINOR_VERSION<4)
    #error "Error, required FLTK 1.4 or later"
#endif

#include "humansize.h"
#include "checker.h"

#include "MostRecentPaths.h"

class YaivWin;
class Slideshow;

class XBox : public Fl_Group
{
public:
    void MenuCB(Fl_Widget *window_p, int menuid);

private:
    // 100%; Scale if larger; Scale to window; Scale to width; Scale to height
    enum ScaleMode { None=0, Auto, Fit, Wide, High, MAX };
    enum OverlayMode { OM_None=0, Text, TBox, OM_MAX };

    static char *humanScale(ScaleMode val, char *buff, int buffsize)
    {
        const char *strs[] = {"None","Auto","Fit","Wide","High"};
        strncpy(buff, strs[(int)val], buffsize);
        return buff;
    }

    static char *humanZScale(int val, char *buff, int buffsize)
    {
        static const char *strs[] = {"None","Box","Bilinear","Bicubic","Lanczos","Bspline","CatMull"};
        strncpy(buff, strs[(int)val], buffsize);
        return buff;
    }

    Fl_Image *_img{}; // the original image loaded from disk
    Fl_RGB_Image *_showImg{}; // the "back-buffer" image: rotated, scaled, checkered
    Fl_Anim_GIF_Image *_anim{}; // an original animation loaded from disk

    bool draw_check{true};
    ScaleMode draw_scale{ScaleMode::None};
    bool draw_center{false};
    OverlayMode draw_overlay{OverlayMode::OM_None};
    double _zoom{1.0};
    int _zoom_step = 0;
    int _scroll_speed = 20;

    // scrolling deltas from origin
    int deltax{0};
    int deltay{0};

    int rotation; // cycle through clockwise rotations of 90 degrees
    int imgtkScale; // cycle through fl_imgtk scale values

    enum
    {
        MI_LOAD,
        MI_COPYPATH,
        MI_GOTO,
        MI_OPTIONS,

        MI_FAVS, // Must be last before MI_FAVx
        MI_FAV0,
        MI_FAV1,
        MI_FAV2,
        MI_FAV3,
        MI_FAV4,
        MI_FAV5,
        MI_FAV6,
        MI_FAV7,
        MI_FAV8,
        MI_FAV9,
    };

    Fl_Menu_Item right_click_menu[7] =
    {
        {"Load",            0, nullptr, (void *)MI_LOAD},
        {"Copy image path", 0, nullptr, (void *)MI_COPYPATH},
        {"Goto Image",      0, nullptr, (void *)MI_GOTO, FL_MENU_DIVIDER},
        {"Options",         0, nullptr, (void *)MI_OPTIONS, FL_MENU_DIVIDER},

        {"Last Used",       0, nullptr, 0, FL_SUBMENU},
            {nullptr}, // end of sub menu
        {nullptr} // end of menu
    };

public:
    XBox(int x, int y, int w, int h, Prefs*);

    int handle(int) override;
    char * getLabel(char *n, char *buff, int buffsize);

    void resize(int,int,int,int) override;

    void change_zoom(int delta) {_zoom_step += delta; updateImage();}

    void draw() override;

    void do_menu();
    void image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg);

    void load_current(); // exposed for slideshow
    void next_image();   // exposed for slideshow

    void load_file(const char *n); // exposed for argv processing

    // exposed for static file chooser callback
    void file_cb(const char *); // process filename from file chooser

    // TODO XBox and YaivMain tightly coupled, need to fix
    void parent(YaivWin* who) {_dad=who;}

private:

    void load_filelist(const char *);
    int find_file(const char *n);
    void load_request(); // 'load' processing

    void prev_image();
    void next_scale();
    void nextTkScale();
    void nextRotation();
    void updateLabel();
    void updateImage();
    void wipeShowImage();
    void drawMinimap();
    void drawOverlay();

    void toggleSlideshow();
    void toggleMinimap();
    void toggleOverlay();

private:
    bool _pan_with_mouse;
    int dragStartX;
    int dragStartY;
    bool dragging;
    int mousePan(int);

    bool _inSlideshow;
    Slideshow *_slideShow;

    bool _minimap;
    static Fl_Color _mmoc;
    static Fl_Color _mmic;
    static int _miniMapSize;

    YaivWin* _dad;

    MostRecentPaths* _mru;
    char filecb_name[1024]; // filename load TODO dynamic?
    Prefs* _prefs;

    // TODO go into a separate 'loader' class
    int current_index;
    char fold[FL_PATH_MAX];
    dirent** file_list;
    int file_count;

private:
    XBoxDisplayInfoEvent *_dispevent;
public:
    void displayEventHandler(XBoxDisplayInfoEvent *hand) {_dispevent = hand;}
};

#endif //CLION_TEST2_XBOX_H
