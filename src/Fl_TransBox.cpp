#include "Fl_TransBox.h"
#include <new>

Fl_TransBox::Fl_TransBox(int x, int y, int w, int h, const char* l)
 : Fl_Box(x, y, w, h, l),
   alpha(0x40) // TODO from prefs
 {
    box(FL_NO_BOX);
    align( FL_ALIGN_CENTER ); // TODO from prefs
    buffer = new (std::nothrow) unsigned char[4*w*h];
    img = new (std::nothrow) Fl_RGB_Image(buffer, w, h, 4);
    color(-51130624);        // TODO from prefs
    labelcolor(FL_DARK_GREEN); // TODO from prefs
}

Fl_TransBox::~Fl_TransBox()
{
    delete img;
    delete[] buffer;
}

bool Fl_TransBox::isInResize = false;

void Fl_TransBox::color(unsigned int c)
{
    r = (c >> 24);
    g = (c >> 16);
    b = (c >> 8);

    fill_buffer();
    img->uncache();
}

void Fl_TransBox::set_alpha(unsigned char a)
{
    alpha = a;
    fill_buffer();
    img->uncache();
}

void Fl_TransBox::resize(int x, int y, int w, int h)
{
    Fl_Box::resize(x,y,w,h);

    if ( isInResize )
        return;

    isInResize = true;

    delete[] buffer;
    delete img;

    buffer = new (std::nothrow) unsigned char[4*w*h];
    if ( buffer != nullptr )
    {
        img = new (std::nothrow) Fl_RGB_Image(buffer, w, h, 4);
        if ( img != nullptr )
        {
            fill_buffer();
            img->uncache();
        }
        else
            delete buffer; // cleanup buffer after img alloc failure
    }

    isInResize = false;
}

void Fl_TransBox::fill_buffer()
{
    unsigned char *p = buffer;
    for (int i = 0; i < 4*w()*h(); i+=4)
    {
        *p++ = r;
        *p++ = g;
        *p++ = b;
        *p++ = alpha;
    }
}

void Fl_TransBox::draw()
{
    fl_push_clip( x(), y(), w(), h() );

    if ( img && !isInResize )
    {
        int putX = ( w() - img->w() ) / 2;
        int putY = ( h() - img->h() ) / 2;

        img->draw(x() + putX, y() + putY);
    }

    if ( image() )
    {
        Fl_Image* dispimg = image();

        int putX = ( w() - dispimg->w() ) / 2;
        int putY = ( h() - dispimg->h() ) / 2;

        dispimg->draw(x() + putX, y() + putY);
    }

    if( label() != nullptr )
    {
        draw_label();
    }

    fl_pop_clip();
}
