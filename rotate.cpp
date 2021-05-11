//
// Created by kevin on 5/11/21.
//

#include "rotate.h"

inline void fl_imgtk_swap_mem( uchar* a, uchar* b, size_t c )
{
    if ( c > 0 )
    {
        uchar* t = new uchar[c];
        if ( t != NULL )
        {
            memcpy( t, a, c );
            memcpy( a, b, c );
            memcpy( b, t, c );

            delete[] t;
        }
    }
}


void discard_user_rgb_image( Fl_RGB_Image* &img )
{
    if( img != NULL )
    {
        if ( ( img->array != NULL ) && ( img->alloc_array == 0 ) )
        {
            delete[] img->array;
        }
        delete img;
        img = NULL;
    }
}


#define OMPSIZE_T       size_t // TODO may be different for MSVC


Fl_RGB_Image* rotate90( Fl_RGB_Image* img )
{
    if ( img == NULL )
        return NULL;

    //OMPSIZE_T w = img->w();
    OMPSIZE_T w = img->data_w();
    //OMPSIZE_T h = img->h();
    OMPSIZE_T h = img->data_h();
    OMPSIZE_T d = img->d();

    OMPSIZE_T src_w = w;
    OMPSIZE_T src_h = h;

    if ( ( src_w > 0 ) && ( src_h > 0 ) )
    {
        const uchar* ptr = (const uchar*)img->data()[0];
        OMPSIZE_T new_w = src_h;
        OMPSIZE_T new_h = src_w;

        uchar* buff = new uchar[ new_w * new_h * d ];

        if ( buff == NULL )
            return NULL;

        OMPSIZE_T cntw = 0;
        OMPSIZE_T cnth = 0;

        for( cntw=new_w; cntw-- != 0; )
        {
#pragma omp parallel for
            for( cnth=0; cnth<new_h; cnth++ )
            {
                unsigned pos1 = ( new_w * cnth + cntw ) * d;
                unsigned pos2 = ( src_w * ( new_w - cntw - 1 ) + cnth ) * d;

                memcpy( &buff[ pos1 ], &ptr[ pos2 ], d );
            }
        }
#if defined(FLIMGTK_IMGBUFF_OWNALLOC)
        Fl_RGB_Image* newimg = new Fl_RGB_Image( buff, new_w, new_h, d );
        if ( newimg != NULL )
        {
            newimg->alloc_array = 1;
            return newimg;
        }
#else
        return new Fl_RGB_Image( buff, new_w, new_h, d );
#endif /// of #if defined(FLIMGTK_IMGBUFF_OWNALLOC)
    }

    return NULL;
}

Fl_RGB_Image* rotate180( Fl_RGB_Image* img )
{
    if ( img == NULL )
        return NULL;

    //OMPSIZE_T w = img->w();
    OMPSIZE_T w = img->data_w(); // TODO is this necessary or even a good idea?
    //OMPSIZE_T h = img->h();
    OMPSIZE_T h = img->data_h();
    OMPSIZE_T d = img->d();

    OMPSIZE_T cur_w = w;
    OMPSIZE_T cur_h = h;

    if ( ( cur_w > 0 ) && ( cur_h > 0 ) )
    {
        uchar* ptr = (uchar*)img->data()[0];
        uchar* buff = new uchar[ w * h * d ];

        if ( buff == NULL )
            return NULL;

        memcpy( buff, ptr, w * h * d );

        OMPSIZE_T imgmax = w*h;
        OMPSIZE_T cntmax = imgmax / 2;

#pragma omp parallel for
        for( OMPSIZE_T cnt=0; cnt<cntmax; cnt++ )
        {
            fl_imgtk_swap_mem( &buff[ cnt * d ],
                               &buff[ (imgmax - cnt - 1) * d ],
                               3);
        }
#if defined(FLIMGTK_IMGBUFF_OWNALLOC)
        Fl_RGB_Image* newimg = new Fl_RGB_Image( buff, w, h, d );
        if ( newimg != NULL )
        {
            newimg->alloc_array = 1;
            return newimg;
        }
#else
        return new Fl_RGB_Image( buff, w, h, d );
#endif /// of #if defined(FLIMGTK_IMGBUFF_OWNALLOC)
    }

    return NULL;
}

Fl_RGB_Image* rotate270( Fl_RGB_Image* img )
{
    if ( img == NULL )
        return NULL;

    //OMPSIZE_T w = img->w();
    OMPSIZE_T w = img->data_w();
    //OMPSIZE_T h = img->h();
    OMPSIZE_T h = img->data_h();
    OMPSIZE_T d = img->d();

    OMPSIZE_T src_w = w;
    OMPSIZE_T src_h = h;

    if ( ( src_w > 0 ) && ( src_h > 0 ) )
    {
        const uchar* ptr = (const uchar*)img->data()[0];
        OMPSIZE_T new_w = src_h;
        OMPSIZE_T new_h = src_w;

        uchar* buff = new uchar[ new_w * new_h * d ];

        if ( buff == NULL )
            return NULL;

        OMPSIZE_T cntw = 0;
        OMPSIZE_T cnth = 0;

#pragma omp parallel for private( cnth )
        for( cntw=0; cntw<new_w; cntw++ )
        {
            for( cnth=new_h; cnth-- != 0; )
            {
                OMPSIZE_T pos1 = ( new_w * cnth + cntw ) * d;
                OMPSIZE_T pos2 = ( src_w * cntw + new_h - cnth - 1 ) * d;

                memcpy( &buff[ pos1 ], &ptr[ pos2 ], d );
            }
        }
#if defined(FLIMGTK_IMGBUFF_OWNALLOC)
        Fl_RGB_Image* newimg = new Fl_RGB_Image( buff, new_w, new_h, d );
        if ( newimg != NULL )
        {
            newimg->alloc_array = 1;
            return newimg;
        }
#else
        return new Fl_RGB_Image( buff, new_w, new_h, d );
#endif /// of #if defined(FLIMGTK_IMGBUFF_OWNALLOC)
    }

    return NULL;
}
