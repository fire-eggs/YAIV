#include "Fl_TransBox.h"


Fl_TransBox::Fl_TransBox(int x, int y, int w, int h, const char* l)
 : Fl_Box(x, y, w, h, l),
   alpha(0x40), // TODO from prefs
   dragEnabled(false)
 {
    box(FL_NO_BOX);
    align( FL_ALIGN_CENTER );
    buffer = new unsigned char[4*w*h];
    img = new Fl_RGB_Image(buffer, w, h, 4);
    color(-51130624);        // TODO from prefs
    labelcolor(FL_DARK_GREEN); // TODO from prefs


}

Fl_TransBox::~Fl_TransBox()
{
    if ( img != NULL )
    {
        //free_fl_rgb( img );
        delete img;
    }

    if ( buffer != NULL )
    {
        delete[] buffer;
    }
}

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

static bool isInResize = false;

void Fl_TransBox::resize(int x, int y, int w, int h)
{
    Fl_Box::resize(x,y,w,h);

    if ( isInResize == true )
        return;

    isInResize = true;

    if ( buffer != NULL )
    {
        delete[] buffer;
        buffer = NULL;
    }

    if ( img != NULL )
    {
        //free_fl_rgb( img );
        delete img;
    }

    buffer = new unsigned char[4*w*h];
    if ( buffer != NULL )
    {
        img = new Fl_RGB_Image(buffer, w, h, 4);

        if ( img != NULL )
        {
            fill_buffer();
            img->uncache();
        }
    }

    isInResize = false;
}

void Fl_TransBox::free_fl_rgb( Fl_RGB_Image* r )
{
    if ( r != NULL )
    {
#if FLTK_ABI_VERSION <= 10303
        if ( ( r->alloc_array == 0 ) && ( r->array != NULL ) )
        {
            delete[] r->array;
        }
#else
        if ( ( r->alloc_array == 1 ) && ( r->array != NULL ) )
        {
            delete[] r->array;
        }
#endif // FLTK_ABI_VERSION
        delete r;
        r = NULL;
    }
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

    if ( ( img != NULL ) && ( isInResize == false ) )
    {
        int putX = ( w() - img->w() ) / 2;
        int putY = ( h() - img->h() ) / 2;

        img->draw(x() + putX, y() + putY);
    }

    if ( image() != NULL )
    {
        Fl_Image* dispimg = image();

        int putX = ( w() - dispimg->w() ) / 2;
        int putY = ( h() - dispimg->h() ) / 2;

        dispimg->draw(x() + putX, y() + putY);
    }

    if( label() != NULL )
    {
        draw_label();
    }

    fl_pop_clip();
}
