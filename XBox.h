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

#include "humansize.h"
#include "checker.h"
#include "rotate.h"
#include "rescaler.h"

class XBox : public Fl_Group
{
public:
    int rotation; // TODO hack
    int imgtkScale; // TODO hack

private:
    // 100%; Scale if larger; Scale to window; Scale to width; Scale to height
    enum ScaleMode { None, Auto, Fit, Wide, High, MAX };

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
    Fl_Image *_showImg{};
    Fl_Anim_GIF_Image *_anim{};
    bool requiresDiscard{}; // true if _showImg needs discard_user_rgb_image

    bool draw_check;
    ScaleMode draw_scale;
    bool draw_center;
    double _zoom{};

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
    XBox(int x, int y, int w, int h) : Fl_Group(x,y,w,h)
    {
        align(FL_ALIGN_INSIDE|FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
        box(FL_BORDER_BOX);
        color(fl_rgb_color(252,243,207));
        end();
        _img = nullptr;
        _anim = nullptr;

        draw_check = true;
        draw_scale = ScaleMode::None;
        draw_center = false;

        deltax = 0;
        deltay = 0;
        rotation = 0;
        imgtkScale = 0;
    }

    int handle(int) override;
    char * getLabel(char *n, char *buff, int buffsize);

    void resize(int,int,int,int) override;

    void draw() override
    {
        Fl_Group::draw();
        if (!_showImg || !_showImg->w() || !_showImg->h())
            return;

        // Note: offset to not overwrite the box outline
        int drawx = x()+1;
        int drawy = y()+1;

        if (draw_center) {
            int iw;
            int ih;
            if (_anim) {iw = _anim->w(); ih = _anim->h();}
            else {iw = _showImg->w(); ih = _showImg->h();}

            deltax = std::max(1, (w() - iw) / 2);
            deltay = std::max(1, (h() - ih) / 2);
        }

        if (_anim && draw_check)
        {
            // TODO can checker be established in updateImage() instead?
            int outw;
            int outh;
            if (_anim) {
                outw = std::min(w(), _anim->w());
                outh = std::min(h(), _anim->h());
            }
            else {
                outw = std::min(w(), _showImg->w());
                outh = std::min(h(), _showImg->h());
            }
            drawChecker(drawx + deltax, drawy + deltay, outw-2, outh-2);
        }

        if (_anim) {
            auto tmp = _anim->image();
            //printf("Draw Anim %d,%d | %d,%d \n", tmp->w(),tmp->h(), tmp->data_w(), tmp->data_h());
            // TODO for some reason the frame scale() isn't "sticking"???
            // TODO 4009.webp gets this far then crashes because image() returns null
            if (tmp)
                tmp->scale(_anim->w(), _anim->h(), 1, 1);
            _anim->draw(drawx, drawy, w() - 2, h() - 2, -deltax, -deltay);
        }
        else {
            _showImg->draw(drawx, drawy, w() - 2, h() - 2, -deltax, -deltay);
        }
        goto draw_label;

#if false
        // TODO Testing imgtk scaling : move to updateImage()

        // scale crashed on discard of 100% image [discard of copy] when using FL_Shared_Image.

        // TODO scale crashes/doesn't work on animated image (see 86000.webp; 6003.webp)
        // TODO don't need to scale if zoom is 100% ?
        if (!_anim && (int)imgtkScale)
        {
            int outw = _anim ? _anim->w() : _img->w();
            int outh = _anim ? _anim->h() : _img->h();
            if (_anim) _anim->scale(_anim->data_w(), _anim->data_h());
            _img->scale(_img->data_w(), _img->data_h());

            auto *icopy = (Fl_Image*)_img->copy();
            Fl_RGB_Image *itksimg = itk_rescale((Fl_RGB_Image *)icopy, outw, outh, imgtkScale-1);
            icopy->release();

            if (itksimg)
            {
                itksimg->draw(drawx, drawy, w()-2, h()-2, -deltax, -deltay);
                discard_user_rgb_image(itksimg);
                goto draw_label;
            }
        }
#endif

draw_label:
        {
            char hack[1000];
            char hack2[1]={'\0'};
            label(getLabel(hack2,hack,1000));
            labelsize(20);
            labelcolor(FL_DARK_GREEN);    // TODO options
            labeltype(FL_EMBOSSED_LABEL); // TODO options
            align(FL_ALIGN_BOTTOM_RIGHT); // TODO options

            if (label())
            {
                int lw,lh;
                measure_label(lw,lh);
                fl_font(labelfont(), labelsize());
                fl_color(labelcolor());
                draw_label(x()+w()-lw-2, y()+h()-lh-2, lw, lh, align()); // TODO options
            }
            label(""); // TODO prevent extra label draw?
        }
    }

    void do_menu();
    void image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg);

private:
    void next_scale();
    void nextTkScale();
    void nextRotation();
    static void updateLabel();
    void updateImage();
    void wipeShowImage();
};

#endif //CLION_TEST2_XBOX_H
