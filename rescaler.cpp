//
// Created by kevin on 5/11/21.
//

#include "rescaler.h"


#include <cmath>
#include <FL/Fl_RGB_Image.H>

#define OMPSIZE_T       size_t // TODO may be different for MSVC

#define MIN(a,b)    (((a)<(b))?(a):(b))
#define MAX(a,b)    (((a)>(b))?(a):(b))

#define FI_RGBA_RED             0
#define FI_RGBA_GREEN           1
#define FI_RGBA_BLUE            2
#define FI_RGBA_ALPHA           3
#define FI_GRAYA_GRAY           0
#define FI_GRAYA_ALPHA          1

/*template <class T> T CLAMP(const T &value, const T &min_value, const T &max_value)
{
    return ((value < min_value) ? min_value : (value > max_value) ? max_value : value);
}*/

int CLAMP(const int v, const int minv, const int maxv)
{
    return v < minv ? minv : v > maxv ? maxv : v ;
}

typedef enum
{
    NONE = 0,
    BILINEAR,
    BICUBIC,
    LANCZOS,
    BSPLINE,
    CATMULL
}rescaletype;

////////////////////////////////////////////////////////////////////////////////
// Filters

class GenericFilter
{
protected:

#define FILTER_PI  double (3.1415926535897932384626433832795)
#define FILTER_2PI double (2.0 * 3.1415926535897932384626433832795)
#define FILTER_4PI double (4.0 * 3.1415926535897932384626433832795)

    double  m_dWidth;

public:
    GenericFilter (double dWidth) : m_dWidth (dWidth) {}
    virtual ~GenericFilter() {}

    double GetWidth()                   { return m_dWidth; }
    void   SetWidth (double dWidth)     { m_dWidth = dWidth; }
    virtual double Filter (double dVal) = 0;
};

class BoxFilter : public GenericFilter
{
public:
    // Default fixed width = 0.5
    BoxFilter() : GenericFilter(0.5) {}
    virtual ~BoxFilter() {}

public:
    double Filter (double dVal) { return (fabs(dVal) <= m_dWidth ? 1.0 : 0.0); }
};

class BilinearFilter : public GenericFilter
{
public:
    BilinearFilter () : GenericFilter(1) {}
    virtual ~BilinearFilter() {}

public:
    double Filter (double dVal)
    {
        dVal = fabs(dVal);
        return (dVal < m_dWidth ? m_dWidth - dVal : 0.0);
    }
};

class BicubicFilter : public GenericFilter
{
protected:
    // data for parameterized Mitchell filter
    double p0, p2, p3;
    double q0, q1, q2, q3;

public:
    // Default fixed width = 2
    BicubicFilter (double b = (1/(double)3), double c = (1/(double)3)) : GenericFilter(2)
    {
        p0 = (6 - 2*b) / 6;
        p2 = (-18 + 12*b + 6*c) / 6;
        p3 = (12 - 9*b - 6*c) / 6;
        q0 = (8*b + 24*c) / 6;
        q1 = (-12*b - 48*c) / 6;
        q2 = (6*b + 30*c) / 6;
        q3 = (-b - 6*c) / 6;
    }
    virtual ~BicubicFilter() {}

public:
    double Filter(double dVal)
    {
        dVal = fabs(dVal);
        if(dVal < 1)
            return (p0 + dVal*dVal*(p2 + dVal*p3));
        if(dVal < 2)
            return (q0 + dVal*(q1 + dVal*(q2 + dVal*q3)));
        return 0;
    }
};

class CatmullRomFilter : public GenericFilter
{
public:

    // Default fixed width = 2
    CatmullRomFilter() : GenericFilter(2) {}
    virtual ~CatmullRomFilter() {}

public:
    double Filter(double dVal)
    {
        if(dVal < -2) return 0;
        if(dVal < -1) return (0.5*(4 + dVal*(8 + dVal*(5 + dVal))));
        if(dVal < 0)  return (0.5*(2 + dVal*dVal*(-5 - 3*dVal)));
        if(dVal < 1)  return (0.5*(2 + dVal*dVal*(-5 + 3*dVal)));
        if(dVal < 2)  return (0.5*(4 + dVal*(-8 + dVal*(5 - dVal))));
        return 0;
    }
};

class Lanczos3Filter : public GenericFilter
{
public:
    // Default fixed width = 3
    Lanczos3Filter() : GenericFilter(3) {}
    virtual ~Lanczos3Filter() {}

public:
    double Filter(double dVal)
    {
        dVal = fabs(dVal);
        if(dVal < m_dWidth)
        {
            return (sinc(dVal) * sinc(dVal / m_dWidth));
        }
        return 0;
    }

private:
    double sinc(double value)
    {
        if(value != 0)
        {
            value *= FILTER_PI;
            return (sin(value) / value);
        }
        return 1;
    }
};

class BSplineFilter : public GenericFilter
{
public:
    // Default fixed width = 2
    BSplineFilter() : GenericFilter(2) {}
    virtual ~BSplineFilter() {}

public:
    double Filter(double dVal)
    {
        dVal = fabs(dVal);
        if(dVal < 1) return (4 + dVal*dVal*(-6 + 3*dVal)) / 6;
        if(dVal < 2)
        {
            double t = 2 - dVal;
            return (t*t*t / 6);
        }
        return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Resize relations.

class WeightsTable
{
    typedef struct
    {
        double *Weights;
        unsigned Left, Right;
    }Contribution;

private:
    Contribution*   m_WeightTable;
    unsigned        m_WindowSize;
    unsigned        m_LineLength;

public:
    WeightsTable(GenericFilter *pFilter, unsigned uDstSize, unsigned uSrcSize);
    ~WeightsTable();

public:
    double getWeight(unsigned dst_pos, unsigned src_pos);
    unsigned getLeftBoundary(unsigned dst_pos);
    unsigned getRightBoundary(unsigned dst_pos);
};

WeightsTable::WeightsTable( GenericFilter *pFilter, unsigned uDstSize, unsigned uSrcSize )
{
    OMPSIZE_T       u;
    double          dWidth;
    double          dFScale         = 1.0;
    const double    dFilterWidth    = pFilter->GetWidth();

    // scale factor
    const double dScale = double(uDstSize) / double(uSrcSize);

    if(dScale < 1.0)
    {
        // minification
        dWidth  = dFilterWidth / dScale;
        dFScale = dScale;
    }
    else
    {
        // magnification
        dWidth= dFilterWidth;
    }

    // allocate a new line contributions structure
    //
    // window size is the number of sampled pixels
    m_WindowSize = 2 * (int)ceil(dWidth) + 1;
    m_LineLength = uDstSize;
    // allocate list of contributions
    m_WeightTable = new Contribution[ m_LineLength + 1 ];
    for(u = 0 ; u < m_LineLength ; u++)
    {
        // allocate contributions for every pixel
        m_WeightTable[u].Weights = new double[ m_WindowSize + 1 ];
    }

    // offset for discrete to continuous coordinate conversion
    const double dOffset = ( 0.5 / dScale) - 0.5;

#pragma omp parallel for
    for(u = 0; u < m_LineLength; u++)
    {
        // scan through line of contributions
        const double dCenter = (double)u / dScale + dOffset;   // reverse mapping

        // find the significant edge points that affect the pixel
        int iLeft  = MAX( 0, (int)floor (dCenter - dWidth) );
        int iRight = MIN( (int)ceil (dCenter + dWidth), int(uSrcSize) - 1 );

        // cut edge points to fit in filter window in case of spill-off
        if((iRight - iLeft + 1) > int(m_WindowSize))
        {
            if(iLeft < (int(uSrcSize) - 1 / 2))
            {
                iLeft++;
            }
            else
            {
                iRight--;
            }
        }

        m_WeightTable[u].Left  = iLeft;
        m_WeightTable[u].Right = iRight;

        int iSrc = 0;
        double dTotalWeight = 0;  // zero sum of weights
        for(iSrc = iLeft; iSrc <= iRight; iSrc++)
        {
            // calculate weights
            const double weight = dFScale * pFilter->Filter(dFScale * (dCenter - (double)iSrc));
            m_WeightTable[u].Weights[iSrc-iLeft] = weight;
            dTotalWeight += weight;
        }

        if((dTotalWeight > 0) && (dTotalWeight != 1))
        {
            // normalize weight of neighbouring points
            for(iSrc = iLeft; iSrc <= iRight; iSrc++)
            {
                // normalize point
                m_WeightTable[u].Weights[iSrc-iLeft] /= dTotalWeight;
            }

            // simplify the filter, discarding null weights at the right
            iSrc = iRight - iLeft;
            while(m_WeightTable[u].Weights[iSrc] == 0)
            {
                m_WeightTable[u].Right--;
                iSrc--;
                if(m_WeightTable[u].Right == m_WeightTable[u].Left)
                {
                    break;
                }
            }

        }
    }
}

WeightsTable::~WeightsTable()
{
    for(unsigned u = 0; u < m_LineLength; u++)
    {
        // free contributions for every pixel
        delete[] m_WeightTable[u].Weights;
    }
    // free list of pixels contributions
    delete[] m_WeightTable;
}

double WeightsTable::getWeight(unsigned dst_pos, unsigned src_pos)
{
    if ( dst_pos < m_LineLength )
    {
        //int sz = m_WeightTable[dst_pos].Right - m_WeightTable[dst_pos].Left;
        if ( src_pos < m_WindowSize )
        {
            return m_WeightTable[dst_pos].Weights[src_pos];
        }
    }

    return 0.0;
}

unsigned WeightsTable::getLeftBoundary(unsigned dst_pos)
{
    return m_WeightTable[dst_pos].Left;
}

unsigned WeightsTable::getRightBoundary(unsigned dst_pos)
{
    return m_WeightTable[dst_pos].Right;
}



class ResizeEngine
{
private:
    GenericFilter* m_pFilter;

public:
    ResizeEngine( GenericFilter* filter );
    virtual ~ResizeEngine() {}

public:
    Fl_RGB_Image* scale(Fl_RGB_Image *src, int dst_width, int dst_height);

public:
    // cindex : 0 = RED, 1 = GREEEN, 2 = BLUE, 3 = ALPHA
    //void useSingleChannel( bool f, char cindex );
    bool useSingleChannel() { return useSCh; }
    char refSingleChannel() { return refSCh; }

private:
    void horizontalFilter( const uchar* src, const unsigned height, const unsigned src_width,
                           const unsigned src_bpp,
                           const unsigned src_offset_x, const unsigned src_offset_y,
                           uchar* dst, const unsigned dst_width);
    void verticalFilter( const uchar* src, const unsigned width, const unsigned src_height,
                         const unsigned src_bpp,
                         const unsigned src_offset_x, const unsigned src_offset_y,
                         uchar* dst, const unsigned dst_height);

protected:
    bool useSCh;
    int  refSCh;
};

ResizeEngine::ResizeEngine( GenericFilter* filter )
        : m_pFilter(filter),
          useSCh(false),
          refSCh(0)
{
}

Fl_RGB_Image* ResizeEngine::scale( Fl_RGB_Image* src, int dst_width, int dst_height )
{
    if ( src == NULL)
        return NULL;

    if ( ( src->w() == 0 ) && ( src->h() == 0 ) )
        return NULL;

    if ( ( src->w() == dst_width) && ( src->h() == dst_height))
    {
        return (Fl_RGB_Image*)src->copy();
    }

    // allocate the dst image
    uchar* dst_buff = new uchar[ ( dst_width * dst_height * 4 ) + 1 ];
    if ( dst_buff == NULL )
    {
        return NULL;
    }

    const uchar* src_buff = (uchar*)src->data()[0];

    if ( dst_width <= src->w() )
    {
        uchar* tmp_buff = NULL;

        if ( src->w() != dst_width )
        {
            if ( src->h() != dst_height )
            {
                tmp_buff = new uchar[ ( dst_width * src->h() * src->d() ) + 1 ];
                if ( tmp_buff == NULL )
                {
                    delete[] dst_buff;
                    return NULL;
                }
            }
            else
            {
                tmp_buff = dst_buff;
            }

            horizontalFilter( src_buff, src->h(), src->w(), src->d(), 0, 0, tmp_buff, dst_width );

        }
        else
        {
            tmp_buff = (uchar*)src_buff;
        }

        if ( src->h() != dst_height )
        {
            verticalFilter( tmp_buff, dst_width, src->h(), src->d(), 0, 0,
                            dst_buff, dst_height );
        }

        if ( ( tmp_buff != src_buff ) && ( tmp_buff != dst_buff ) )
        {
            delete[] tmp_buff;
            tmp_buff = NULL;
        }

    }
    else    /// == ( dst_width > src->w() )
    {
        uchar* tmp_buff = NULL;

        if ( src->h() != dst_height )
        {
            if ( src->w() != dst_width )
            {
                tmp_buff = new uchar[ src->w() * dst_height * src->d() + 1 ];
                if ( tmp_buff == NULL )
                {
                    delete[] dst_buff;
                    return NULL;
                }
            } else {
                tmp_buff = dst_buff;
            }

            verticalFilter( src_buff, src->w(), src->h(), src->d(),
                            0, 0, tmp_buff, dst_height );

        }
        else
        {
            tmp_buff = (uchar*)src_buff;
        }

        if ( src->w() != dst_width )
        {
            horizontalFilter( tmp_buff, dst_height, src->w(), src->d(),
                              0, 0, dst_buff, dst_width );
        }

        if ( ( tmp_buff != src_buff ) && ( tmp_buff != dst_buff ) )
        {
            delete[] tmp_buff;
            tmp_buff = NULL;
        }
    }

    if ( dst_buff != NULL )
    {
        Fl_RGB_Image *dst = new Fl_RGB_Image( dst_buff, dst_width, dst_height, src->d() );
        if ( dst == NULL )
        {
            delete[] dst_buff;
            return NULL;
        }
#if defined(FLIMGTK_IMGBUFF_OWNALLOC)
        if ( dst != NULL )
        {
            dst->alloc_array = 1;
        }
#endif
        return dst;
    }

    return NULL;
}

void ResizeEngine::horizontalFilter( const uchar* src, const unsigned height, const unsigned src_width, const unsigned src_bpp, const unsigned src_offset_x, const unsigned src_offset_y, uchar* dst, const unsigned dst_width)
{
    // allocate and calculate the contributions
    WeightsTable weightsTable(m_pFilter, dst_width, src_width);

    switch ( src_bpp )
    {
        case 1:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(x)
            for ( y = 0; y < height; y++)
            {
                const
                uchar* src_bits = &src[ ( ( y + src_offset_y ) * src_width * src_bpp ) +
                                        ( src_offset_x * src_bpp ) ];
                uchar* dst_bits = &dst[ y * dst_width * src_bpp ];

                // scale each row
                for ( x = 0; x < dst_width; x++)
                {
                    // loop through row
                    const unsigned iLeft  = weightsTable.getLeftBoundary(x);            // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(x) - iLeft;   // retrieve right boundary
                    const uchar*   pixel  = src_bits + iLeft * src_bpp;
                    double v = 0;

                    // for(i = iLeft to iRight)
                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(x, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)pixel[refSCh]);
                            v += c;
                        }
                        else
                        {
                            v += (weight * (double)pixel[FI_GRAYA_GRAY]);
                        }

                        pixel += src_bpp;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int)(v + 0.5), 0, 0xFF);
                        dst_bits[FI_GRAYA_GRAY]   = cmpv;
                    }
                    else
                    {
                        dst_bits[FI_GRAYA_GRAY]   = (uchar)CLAMP((int)(v + 0.5), 0, 0xFF);
                    }
                    dst_bits += src_bpp;
                }
            }
        }
        break;

        case 2:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(x)
            for ( y = 0; y < height; y++)
            {
                const
                uchar* src_bits = &src[ ( ( y + src_offset_y ) * src_width * src_bpp ) +
                                        ( src_offset_x * src_bpp ) ];
                uchar* dst_bits = &dst[ y * dst_width * src_bpp ];

                // scale each row
                for ( x = 0; x < dst_width; x++)
                {
                    // loop through row
                    const unsigned iLeft = weightsTable.getLeftBoundary(x);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(x) - iLeft;   // retrieve right boundary
                    const uchar *pixel = src_bits + iLeft * src_bpp;
                    double v = 0, a = 0;

                    // for(i = iLeft to iRight)
                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(x, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)pixel[refSCh]);
                            v += c;
                            a += (weight * (double)pixel[FI_GRAYA_ALPHA]);
                        }
                        else
                        {
                            v += (weight * (double)pixel[FI_GRAYA_GRAY]);
                            a += (weight * (double)pixel[FI_GRAYA_ALPHA]);
                        }
                        pixel += src_bpp;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int)(v + 0.5), 0, 0xFF);

                        dst_bits[FI_GRAYA_GRAY]   = cmpv;
                        dst_bits[FI_GRAYA_ALPHA] = (uchar)CLAMP((int)(a + 0.5), 0, 0xFF);
                    }
                    else
                    {
                        dst_bits[FI_GRAYA_GRAY] = (uchar)CLAMP((int)(v + 0.5), 0, 0xFF);
                        dst_bits[FI_GRAYA_ALPHA] = (uchar)CLAMP((int)(a + 0.5), 0, 0xFF);
                    }
                    dst_bits += src_bpp;
                }
            }
        }
            break;

        case 3:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(x)
            for ( y = 0; y < height; y++)
            {
                const
                uchar* src_bits = &src[ ( ( y + src_offset_y ) * src_width * src_bpp ) +
                                        ( src_offset_x * src_bpp ) ];
                uchar* dst_bits = &dst[ y * dst_width * src_bpp ];

                // scale each row
                for ( x = 0; x < dst_width; x++)
                {
                    // loop through row
                    const unsigned iLeft  = weightsTable.getLeftBoundary(x);            // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(x) - iLeft;   // retrieve right boundary
                    const uchar*   pixel  = src_bits + iLeft * src_bpp;
                    double r = 0, g = 0, b = 0;

                    // for(i = iLeft to iRight)
                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(x, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)pixel[refSCh]);
                            r += c;
                            g += c;
                            b += c;
                        }
                        else
                        {
                            r += (weight * (double)pixel[FI_RGBA_RED]);
                            g += (weight * (double)pixel[FI_RGBA_GREEN]);
                            b += (weight * (double)pixel[FI_RGBA_BLUE]);
                        }

                        pixel += src_bpp;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int)(r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_RED]   = cmpv;
                        dst_bits[FI_RGBA_GREEN] = cmpv;
                        dst_bits[FI_RGBA_BLUE]  = cmpv;
                    }
                    else
                    {
                        dst_bits[FI_RGBA_RED]   = (uchar)CLAMP((int)(r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_GREEN] = (uchar)CLAMP((int)(g + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_BLUE]  = (uchar)CLAMP((int)(b + 0.5), 0, 0xFF);
                    }
                    dst_bits += src_bpp;
                }
            }
        }
            break;

        case 4:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(x)
            for ( y = 0; y < height; y++)
            {
                const
                uchar* src_bits = &src[ ( ( y + src_offset_y ) * src_width * src_bpp ) +
                                        ( src_offset_x * src_bpp ) ];
                uchar* dst_bits = &dst[ y * dst_width * src_bpp ];

                // scale each row
                for ( x = 0; x < dst_width; x++)
                {
                    // loop through row
                    const unsigned iLeft = weightsTable.getLeftBoundary(x);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(x) - iLeft;   // retrieve right boundary
                    const uchar *pixel = src_bits + iLeft * src_bpp;
                    double r = 0, g = 0, b = 0, a = 0;

                    // for(i = iLeft to iRight)
                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(x, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)pixel[refSCh]);
                            r += c;
                            g += c;
                            b += c;
                            a += (weight * (double)pixel[FI_RGBA_ALPHA]);
                        }
                        else
                        {
                            r += (weight * (double)pixel[FI_RGBA_RED]);
                            g += (weight * (double)pixel[FI_RGBA_GREEN]);
                            b += (weight * (double)pixel[FI_RGBA_BLUE]);
                            a += (weight * (double)pixel[FI_RGBA_ALPHA]);
                        }
                        pixel += src_bpp;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int)(r + 0.5), 0, 0xFF);

                        dst_bits[FI_RGBA_RED]   = cmpv;
                        dst_bits[FI_RGBA_GREEN] = cmpv;
                        dst_bits[FI_RGBA_BLUE]  = cmpv;
                        dst_bits[FI_RGBA_ALPHA] = (uchar)CLAMP((int)(a + 0.5), 0, 0xFF);
                    }
                    else
                    {
                        dst_bits[FI_RGBA_RED]   = (uchar)CLAMP((int)(r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_GREEN] = (uchar)CLAMP((int)(g + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_BLUE]  = (uchar)CLAMP((int)(b + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_ALPHA] = (uchar)CLAMP((int)(a + 0.5), 0, 0xFF);
                    }
                    dst_bits += src_bpp;
                }
            }
        }
            break;
        default:
            break;
    } /// of switch()
}

/// Performs vertical image filtering
void ResizeEngine::verticalFilter( const uchar* src, unsigned width, unsigned src_height, const unsigned src_bpp, unsigned src_offset_x, unsigned src_offset_y, uchar* dst, unsigned dst_height)
{
    // allocate and calculate the contributions
    WeightsTable weightsTable( m_pFilter, dst_height, src_height );

    //unsigned dst_pitch = dst_width * src_bpp;
    unsigned dst_pitch = width * src_bpp;
    uchar*   dst_base  = dst;
    unsigned src_pitch = width * src_bpp;

    switch( src_bpp )
    {
        case 1:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(y)
            for ( x = 0; x < width; x++)
            {
                // work on column x in dst
                const unsigned index = x * src_bpp;
                uchar* dst_bits = dst_base + index;

                // scale each column
                for ( y = 0; y < dst_height; y++)
                {
                    const
                    uchar* src_base  = &src[ ( src_offset_y * width * src_bpp ) +
                                             ( src_offset_y * src_pitch + src_offset_x * src_bpp ) ];
                    // loop through column
                    const unsigned iLeft = weightsTable.getLeftBoundary(y);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(y) - iLeft;   // retrieve right boundary
                    const uchar *src_bits = src_base + iLeft * src_pitch + index;
                    double v = 0;

                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(y, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)src_bits[refSCh]);
                            v += c;
                        }
                        else
                        {
                            v += (weight * (double)src_bits[FI_GRAYA_GRAY]);
                        }
                        src_bits += src_pitch;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int) (v + 0.5), 0, 0xFF);
                        dst_bits[FI_GRAYA_GRAY]   = cmpv;
                    }
                    else
                    {
                        dst_bits[FI_GRAYA_GRAY]   = (uchar)CLAMP((int) (v + 0.5), 0, 0xFF);
                    }
                    dst_bits += dst_pitch;
                }
            }
        }
            break;

        case 2:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(y)
            for ( x = 0; x < width; x++)
            {
                // work on column x in dst
                const unsigned index = x * src_bpp;
                uchar *dst_bits = dst_base + index;

                // scale each column
                for ( y = 0; y < dst_height; y++)
                {
                    const
                    uchar* src_base  = &src[ ( src_offset_y * width * src_bpp ) +
                                             ( src_offset_y * src_pitch + src_offset_x * src_bpp ) ];
                    // loop through column
                    const unsigned iLeft = weightsTable.getLeftBoundary(y);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(y) - iLeft;   // retrieve right boundary
                    const uchar *src_bits = src_base + iLeft * src_pitch + index;
                    double v = 0, a = 0;

                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(y, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)src_bits[refSCh]);
                            v += c;
                            a += (weight * (double)src_bits[1]);
                        }
                        else
                        {
                            v += (weight * (double)src_bits[FI_GRAYA_GRAY]);
                            a += (weight * (double)src_bits[FI_GRAYA_ALPHA]);
                        }
                        src_bits += src_pitch;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int) (v + 0.5), 0, 0xFF);
                        dst_bits[FI_GRAYA_GRAY]   = cmpv;
                        dst_bits[FI_GRAYA_ALPHA] = (uchar)CLAMP((int) (a + 0.5), 0, 0xFF);
                    }
                    else
                    {
                        dst_bits[FI_GRAYA_GRAY]   = (uchar)CLAMP((int) (v + 0.5), 0, 0xFF);
                        dst_bits[FI_GRAYA_ALPHA] = (uchar)CLAMP((int) (a + 0.5), 0, 0xFF);
                    }
                    dst_bits += dst_pitch;
                }
            }
        }
            break;

        case 3:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(y)
            for ( x = 0; x < width; x++)
            {
                // work on column x in dst
                const unsigned index = x * src_bpp;
                uchar* dst_bits = dst_base + index;

                // scale each column
                for ( y = 0; y < dst_height; y++)
                {
                    const
                    uchar* src_base  = &src[ ( src_offset_y * width * src_bpp ) +
                                             ( src_offset_y * src_pitch + src_offset_x * src_bpp ) ];
                    // loop through column
                    const unsigned iLeft = weightsTable.getLeftBoundary(y);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(y) - iLeft;   // retrieve right boundary
                    const uchar *src_bits = src_base + iLeft * src_pitch + index;
                    double r = 0, g = 0, b = 0;

                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(y, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)src_bits[refSCh]);
                            r += c;
                            g += c;
                            b += c;
                        }
                        else
                        {
                            r += (weight * (double)src_bits[FI_RGBA_RED]);
                            g += (weight * (double)src_bits[FI_RGBA_GREEN]);
                            b += (weight * (double)src_bits[FI_RGBA_BLUE]);
                        }
                        src_bits += src_pitch;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int) (r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_RED]   = cmpv;
                        dst_bits[FI_RGBA_GREEN] = cmpv;
                        dst_bits[FI_RGBA_BLUE]  = cmpv;
                    }
                    else
                    {
                        dst_bits[FI_RGBA_RED]   = (uchar)CLAMP((int) (r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_GREEN] = (uchar)CLAMP((int) (g + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_BLUE]  = (uchar)CLAMP((int) (b + 0.5), 0, 0xFF);
                    }
                    dst_bits += dst_pitch;
                }
            }
        }
            break;

        case 4:
        {
            OMPSIZE_T x = 0;
            OMPSIZE_T y = 0;

#pragma omp parallel for private(y)
            for ( x = 0; x < width; x++)
            {
                // work on column x in dst
                const unsigned index = x * src_bpp;
                uchar *dst_bits = dst_base + index;

                // scale each column
                for ( y = 0; y < dst_height; y++)
                {
                    const
                    uchar* src_base  = &src[ ( src_offset_y * width * src_bpp ) +
                                             ( src_offset_y * src_pitch + src_offset_x * src_bpp ) ];
                    // loop through column
                    const unsigned iLeft = weightsTable.getLeftBoundary(y);             // retrieve left boundary
                    const unsigned iLimit = weightsTable.getRightBoundary(y) - iLeft;   // retrieve right boundary
                    const uchar *src_bits = src_base + iLeft * src_pitch + index;
                    double r = 0, g = 0, b = 0, a = 0;

                    for (unsigned i = 0; i <= iLimit; i++)
                    {
                        // scan between boundaries
                        // accumulate weighted effect of each neighboring pixel
                        const double weight = weightsTable.getWeight(y, i);

                        if (useSCh)
                        {
                            double c = (weight * (double)src_bits[refSCh]);
                            r += c;
                            g += c;
                            b += c;
                            a += (weight * (double)src_bits[FI_RGBA_ALPHA]);
                        }
                        else
                        {
                            r += (weight * (double)src_bits[FI_RGBA_RED]);
                            g += (weight * (double)src_bits[FI_RGBA_GREEN]);
                            b += (weight * (double)src_bits[FI_RGBA_BLUE]);
                            a += (weight * (double)src_bits[FI_RGBA_ALPHA]);
                        }
                        src_bits += src_pitch;
                    }

                    // clamp and place result in destination pixel
                    if (useSCh)
                    {
                        uchar cmpv = (uchar)CLAMP((int) (r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_RED]   = cmpv;
                        dst_bits[FI_RGBA_GREEN] = cmpv;
                        dst_bits[FI_RGBA_BLUE]  = cmpv;
                        dst_bits[FI_RGBA_ALPHA] = (uchar)CLAMP((int) (a + 0.5), 0, 0xFF);
                    }
                    else
                    {
                        dst_bits[FI_RGBA_RED]   = (uchar)CLAMP((int) (r + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_GREEN] = (uchar)CLAMP((int) (g + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_BLUE]  = (uchar)CLAMP((int) (b + 0.5), 0, 0xFF);
                        dst_bits[FI_RGBA_ALPHA] = (uchar)CLAMP((int) (a + 0.5), 0, 0xFF);
                    }
                    dst_bits += dst_pitch;
                }
            }
        }
            break;
        default:
            break;
    }
}


Fl_RGB_Image* itk_rescale( Fl_RGB_Image* img, unsigned w, unsigned h, int _rst )
{
    auto rst = (rescaletype)_rst;

    if (img == nullptr || w <= 0 || h <= 0)
        return nullptr;

    GenericFilter* afilter = nullptr;
    Fl_RGB_Image* newimg = nullptr;

    switch( (int)rst )
    {
        case (int)NONE:
        default:
            afilter = new BoxFilter();
            break;

        case (int)BILINEAR:
            afilter = new BilinearFilter();
            break;

        case (int)BICUBIC:
            afilter = new BicubicFilter();
            break;

        case (int)LANCZOS:
            afilter = new Lanczos3Filter();
            break;

        case (int)BSPLINE:
            afilter = new BSplineFilter();
            break;

        case CATMULL:
            afilter = new CatmullRomFilter();
            break;
    }

    if ( afilter != nullptr )
    {
        ResizeEngine* rse = new ResizeEngine( afilter );
        if ( rse != nullptr )
        {
            newimg = rse->scale( img, w, h );
            delete rse;
        }

        delete afilter;
    }

    return newimg;

}
