//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_XBOX_H
#define CLION_TEST2_XBOX_H

#include <FL/Fl_Group.H>
#include <Fl_Anim_GIF_Image.h>

#include <string> // stoi
#include <climits> // INT_MAX, INT_MIN
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

    char *humanScale(ScaleMode val, char *buff, int buffsize)
    {
        const char *strs[] = {"None","Auto","Fit","Wide","High"};
        strncpy(buff, strs[(int)val], buffsize);
        return buff;
    }

    char *humanZScale(int val, char *buff, int buffsize)
    {
        const char *strs[] = {"None","Box","Bilinear","Bicubic","Lanczos","Bspline","CatMull"};
        strncpy(buff, strs[(int)val], buffsize);
        return buff;
    }

    Fl_Image *_img;
    Fl_Anim_GIF_Image *_anim;

    bool draw_check;
    ScaleMode draw_scale;
    bool draw_center;
    double _zoom;

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


    Fl_Menu_Item right_click_menu[5] =
            {
                    {"Load",            0, MenuCB, (void *)MI_LOAD},
                    {"Copy image path", 0, MenuCB, (void *)MI_COPYPATH},
                    {"Goto Image",      0, MenuCB, (void *)MI_GOTO, FL_MENU_DIVIDER},
                    {"Options",         0, MenuCB, (void *)MI_OPTIONS, FL_MENU_DIVIDER},

                    {0} // end of menu
            };

public:
    XBox(int x, int y, int w, int h) : Fl_Group(x,y,w,h)
    {
        align(FL_ALIGN_INSIDE|FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_CLIP);
        box(FL_BORDER_BOX);
        color(fl_rgb_color(252,243,207));
        end();
        _img = NULL;
        _anim = NULL;

        draw_check = true;
        draw_scale = ScaleMode::None;
        draw_center = false; // "centered" is really confusing. should center image at current scale

        deltax = 0;
        deltay = 0;
        rotation = 0;
        imgtkScale = 0;
    }

    int handle(int);
    char * getLabel(char *n, char *buff, int buffsize);

    void image(Fl_Image *img, Fl_Anim_GIF_Image *animimg)
    {
        if (_anim)
        {
            //Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(_img);
            _anim->stop();
            _anim->canvas(NULL);
//        Fl_Anim_GIF_Image::animate = false;
            delete _anim;
        }

        else if (_img)
            _img->release();

        _img = img;
        _anim = animimg;
    }


    void draw()
    {
        Fl_Group::draw();
        if (!_img) return;
        if (!_img->w() || !_img->h()) return; // zero dimension

//    int new_anim_w = 0; // hack TODO fix scale logic in Fl_Anim_GIF_Image
//    int new_anim_h = 0;

        //double oldzoom = _zoom;
        _zoom = 1.0;

        // TODO: needs to be initialized on change, allowing zoom to be changed
        // TODO: do using zoom value; take zoom value into account
        if (draw_scale == ScaleMode::Auto)
        {
            if (_anim) {
                _anim->scale(w(), h());
                _zoom = std::max( (double)_anim->w() / _anim->data_w(),
                                  (double)_anim->h() / _anim->data_h());
            }
            else {
                _img->scale(w(),h());
                _zoom = std::max( (double)_img->w() / _img->data_w(),
                                  (double)_img->h() / _img->data_h());
            }
        }
        else if (draw_scale == ScaleMode::Fit)
        {
            if (_anim) {
                _anim->scale(w(), h(), 1, 1);
                _zoom = std::max( (double)_anim->w() / _anim->data_w(),
                                  (double)_anim->h() / _anim->data_h());
            }
            else {
                _img->scale(w(),h(), 1, 1);
                _zoom = std::max( (double)_img->w() / _img->data_w(),
                                  (double)_img->h() / _img->data_h());
            }
        }
        else if (draw_scale == ScaleMode::None)
        {
            if (_anim)
                _anim->scale(_anim->data_w(), _anim->data_h());
            else
                _img->scale(_img->data_w(),_img->data_h());
        }
        else if (draw_scale == ScaleMode::Wide)
        {
            int new_w = w();
            int new_h = (int)((double)_img->h() * w() / (double)_img->w());
            if (_anim) {
                _anim->scale(new_w, new_h, 1, 1);
                _zoom = (double)_anim->w() / _anim->data_w();
            }
            else {
                _img->scale(new_w, new_h, 1, 1);
                _zoom = (double)_img->w() / _img->data_w();
            }
        }
        else if (draw_scale == ScaleMode::High)
        {
            int new_h = h();
            int new_w = (int)((double)_img->w() * h() / (double)_img->h());
            if (_anim) {
                _anim->scale(new_w, new_h, 1, 1);
                _zoom = (double)_anim->h() / _anim->data_h();
            }
            else {
                _img->scale(new_w, new_h, 1, 1);
                _zoom = (double)_img->h() / _img->data_h();
            }
        }

        //::_w->updateLabel(); // TODO super hack
/*
    if (_anim)
    {
//		printf("%d,%d | %d,%d\n", _anim->data_w(), _anim->data_h(), _anim->w(), _anim->h());
//		if (draw_fit) _anim->resize(w(), h());
        _anim->scale(_anim->data_w(), _anim->data_h());
		_anim->resize(new_anim_w, new_anim_h);
	}
*/
        // Note: offset to not overwrite the box outline
        int drawx = x()+1;
        int drawy = y()+1;

        if (draw_center) {
            int iw;
            int ih;
            if (_anim) {iw = _anim->w(); ih = _anim->h();}
            else {iw = _img->w(); ih = _img->h();}
            //drawx = x() +
            deltax = std::max(1, (w() - iw) / 2);
            deltay = std::max(1, (h() - ih) / 2);
            //drawy = y() +
        }

        if (draw_check)
        {
            int outw = 0;
            int outh = 0;
            if (_anim) {
                outw = std::min(w(), _anim->w());
                outh = std::min(h(), _anim->h());
            }
            else {
                outw = std::min(w(), _img->w());
                outh = std::min(h(), _img->h());
            }

            drawChecker(x() + deltax + 1, y() + deltay + 1, outw-2, outh-2);
        }

        if (rotation)
        {
            // TODO rotation of scaled image is garbage - need to rotate, then scale?
            Fl_RGB_Image *rimg = nullptr;
            switch (rotation)
            {
                case 1:
                    rimg = rotate90((Fl_RGB_Image*)_img->copy());
                    break;
                case 2:
                    rimg = rotate180((Fl_RGB_Image*)_img->copy());
                    break;
                case 3:
                    rimg = rotate270((Fl_RGB_Image*)_img->copy());
                    break;
                default:
                    rotation = 0;
                    break;
            }
            if (rimg)
            {
                rimg->draw(drawx, drawy, w()-2, h()-2, -deltax, -deltay);
                discard_user_rgb_image(rimg);
                goto draw_label;
                return;
            }
        }

        // Testing imgtk scaling

        // scale crashed on discard of 100% image [discard of copy] when using FL_Shared_Image.

        // TODO scale crashes/doesn't work on animated image (see 86000.webp; 6003.webp)
        // TODO don't need to scale if zoom is 100% ?
        if (!_anim && (int)imgtkScale)
        {
            int outw = _anim ? _anim->w() : _img->w();
            int outh = _anim ? _anim->h() : _img->h();
            if (_anim) _anim->scale(_anim->data_w(), _anim->data_h());
            _img->scale(_img->data_w(), _img->data_h());

            Fl_Image *icopy = (Fl_Image*)_img->copy();
            Fl_RGB_Image *itksimg = itk_rescale((Fl_RGB_Image *)icopy, outw, outh, imgtkScale-1);
            icopy->release();

            if (itksimg)
            {
                itksimg->draw(drawx, drawy, w()-2, h()-2, -deltax, -deltay);
                discard_user_rgb_image(itksimg);
                goto draw_label;
            }
        }

        // Note: the -2 is to prevent drawing past the right/bottom edge of the box
        // Drawing appears to be "absolute" i.e. not auto-clipped to the widget?
        if (_anim)
        {
            _anim->draw(drawx, drawy, w()-2, h()-2, -deltax, -deltay);
        }
        else if (_img)
        {
            _img->draw(drawx, drawy, w()-2, h()-2, -deltax, -deltay);
        }


        draw_label:
        {
            char hack[1000];
            char hack2[1]={'\0'};
            label(getLabel(hack2,hack,1000));
            labelsize(20);
            labelcolor(FL_DARK_GREEN);
            labeltype(FL_EMBOSSED_LABEL);
            align(FL_ALIGN_RIGHT|FL_ALIGN_BOTTOM);

        }

        if (label())
        {
            int lw,lh;
            measure_label(lw,lh);
            fl_font(labelfont(), labelsize());
            fl_color(labelcolor());
            //fl_draw(label(), x(), y(), w(), h(), align());
            draw_label(x()+w()-lw-2, y()+h()-lh-2, lw, lh, align());
        }
    }

    void do_menu() {
        const Fl_Menu_Item *m = right_click_menu->popup(Fl::event_x(), Fl::event_y(), "YAIV", 0, 0);
        if (m && m->callback())
            m->do_callback(this, m->user_data());
    }
};

#endif //CLION_TEST2_XBOX_H
