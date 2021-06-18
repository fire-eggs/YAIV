#ifndef __FL_TRANSBOX_H__
#define __FL_TRANSBOX_H__

//
// ----------------------------------------------------------------------
// A simple Transparency background Fl_Box implementation.
// ----------------------------------------------------------------------
// (C)2017 Raphael Kim, rageworx@gmail.com
//

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Image.H>

class Fl_TransBox : public Fl_Box
{
    public:
        Fl_TransBox(int x,int y,int w,int h, const char* l = nullptr);
        ~Fl_TransBox() override;

    public:
        void color(unsigned int c); // hides base function
        void set_alpha(unsigned char a);

    protected:
        void resize(int x, int y, int w, int h) override;
        void draw() override;

    private:
        void fill_buffer();

    private:
        unsigned char*  buffer;
        unsigned char   r;
        unsigned char   g;
        unsigned char   b;
        unsigned char   alpha;
        Fl_RGB_Image*   img;
        static bool     isInResize;
};

#endif /// __FL_TRANSBOX_H__
