//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_XBOX_H
#define CLION_TEST2_XBOX_H

#include <string> // stoi
#include <climits> // INT_MAX, INT_MIN

#include <FL/Fl_Group.H>
#include <Fl_Anim_GIF_Image.h>
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_draw.H>

#if (FL_MINOR_VERSION<4)
    #error "Error, required FLTK 1.4 or later"
#endif

#include "humansize.h"
#include "checker.h"
#include "rotate.h"
#include "rescaler.h"

#include "Slideshow.h"

class XBox : public Fl_Group
{
public:
    int rotation; // TODO hack
    int imgtkScale; // TODO hack

private:
    // 100%; Scale if larger; Scale to window; Scale to width; Scale to height
    enum ScaleMode { None=0, Auto, Fit, Wide, High, MAX };

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

    Fl_Image *_img{};
    Fl_RGB_Image *_showImg{};
    Fl_Anim_GIF_Image *_anim{};

    bool draw_check;
    ScaleMode draw_scale;
    bool draw_center;
    double _zoom{};
    int _zoom_step = 0;
    int _scroll_speed = 20;

    int deltax;
    int deltay;

    static void MenuCB(Fl_Widget *, void *);

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
                    {"Load",            0, MenuCB, (void *)MI_LOAD},
                    {"Copy image path", 0, MenuCB, (void *)MI_COPYPATH},
                    {"Goto Image",      0, MenuCB, (void *)MI_GOTO, FL_MENU_DIVIDER},
                    {"Options",         0, MenuCB, (void *)MI_OPTIONS, FL_MENU_DIVIDER},

                    {"Last Used",       0,      0, 0, FL_SUBMENU},
                        {nullptr}, // end of sub menu
                    {nullptr} // end of menu
            };

public:
    XBox(int x, int y, int w, int h);

    int handle(int) override;
    char * getLabel(char *n, char *buff, int buffsize);

    void resize(int,int,int,int) override;

    void change_zoom(int delta) {_zoom_step += delta; updateImage();}

    void draw() override;

    void do_menu();
    void image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg);

    void load_current();
    void next_image();
    void prev_image();
    void toggleSlideshow();
    void toggleMinimap();

private:
    void next_scale();
    void nextTkScale();
    void nextRotation();
    static void updateLabel();
    void updateImage();
    void wipeShowImage();
    void drawMinimap();
    void drawOverlay();

    bool _inSlideshow;
    Slideshow *_slideShow;

    bool _minimap;
    static Fl_Color _mmoc;
    static Fl_Color _mmic;
    static int _miniMapSize;
};

#endif //CLION_TEST2_XBOX_H
