#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
//
// Created by kevin on 5/27/21.
//
/*
Copied from APNG Disassembler 2.9
http://apngdis.sourceforge.net

Copyright (c) 2010-2017 Max Stepin
maxst at users.sourceforge.net

zlib license
*/

// Adapted to provide a Fl_RGB_Image* or a Fl_Anim_GIF_Image*, as necessary

#include <cstdio>
#include <cstring>
#include <vector>
#include <Fl_Anim_GIF_Image.h>
#include "apng.h"
#include "png.h" // unpatched libpng is OK

#define id_IHDR 0x52444849
#define id_acTL 0x4C546361
#define id_fcTL 0x4C546366
#define id_IDAT 0x54414449
#define id_fdAT 0x54416466
#define id_IEND 0x444E4549

const unsigned long cMaxPNGSize = 16384UL;

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

struct CHUNK { unsigned char * p; unsigned int size; };
struct Image
{
    typedef unsigned char * ROW;
    unsigned int w, h, bpp, delay_num, delay_den;
    unsigned char * p;
    ROW * rows;
    Image() : w(0), h(0), bpp(0), delay_num(1), delay_den(10), p(nullptr), rows(nullptr) { }
    ~Image() = default;
    void init(unsigned int w1, unsigned int h1, unsigned int bpp1)
    {
        w = w1; h = h1; bpp = bpp1;
        int rowbytes = w * bpp;
        rows = new ROW[h];
        rows[0] = p = new unsigned char[h * rowbytes];
        for (unsigned int j=1; j<h; j++)
            rows[j] = rows[j-1] + rowbytes;
    }
    void free() //const
    {
        if (rows) delete[] rows;
        if (p)    delete[] p;
        rows = nullptr;
        p = nullptr;
    }
};

void compose_frame(unsigned char ** rows_dst, unsigned char ** rows_src,
                   unsigned char bop, unsigned int x, unsigned int y,
                   unsigned int w, unsigned int h)
{
    for (unsigned int j=0; j<h; j++)
    {
        unsigned char * sp = rows_src[j];
        unsigned char * dp = rows_dst[j+y] + x*4;

        if (bop == 0)
            memcpy(dp, sp, w*4);
        else
            for (unsigned int i=0; i<w; i++, sp+=4, dp+=4)
            {
                if (sp[3] == 255)
                    memcpy(dp, sp, 4);
                else
                if (sp[3] != 0)
                {
                    if (dp[3] != 0)
                    {
                        int u = sp[3]*255;
                        int v = (255-sp[3])*dp[3];
                        int al = u + v;
                        dp[0] = (sp[0]*u + dp[0]*v)/al;
                        dp[1] = (sp[1]*u + dp[1]*v)/al;
                        dp[2] = (sp[2]*u + dp[2]*v)/al;
                        dp[3] = al/255;
                    }
                    else
                        memcpy(dp, sp, 4);
                }
            }
    }
}

void info_fn(png_structp png_ptr, png_infop info_ptr)
{
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
    png_set_gray_to_rgb(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
    (void)png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
}

void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
    Image * image = (Image *)png_get_progressive_ptr(png_ptr);
    png_progressive_combine_row(png_ptr, image->rows[row_num], new_row);
}

int processing_start(png_structp & png_ptr, png_infop & info_ptr, void * frame_ptr, bool hasInfo, CHUNK & chunkIHDR, std::vector<CHUNK>& chunksInfo)
{
    unsigned char header[8] = {137, 80, 78, 71, 13, 10, 26, 10};

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    if (!png_ptr || !info_ptr)
        return 1;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return 1;
    }

    png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
    png_set_progressive_read_fn(png_ptr, frame_ptr, info_fn, row_fn, NULL);

    png_process_data(png_ptr, info_ptr, header, 8);
    png_process_data(png_ptr, info_ptr, chunkIHDR.p, chunkIHDR.size);

    if (hasInfo)
        for (unsigned int i=0; i<chunksInfo.size(); i++)
            png_process_data(png_ptr, info_ptr, chunksInfo[i].p, chunksInfo[i].size);

    return 0;
}

int processing_data(png_structp png_ptr, png_infop info_ptr, unsigned char * p, unsigned int size)
{
    if (!png_ptr || !info_ptr)
        return 1;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return 1;
    }

    png_process_data(png_ptr, info_ptr, p, size);
    return 0;
}

int processing_finish(png_structp png_ptr, png_infop info_ptr)
{
    unsigned char footer[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};

    if (!png_ptr || !info_ptr)
        return 1;

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        return 1;
    }

    png_process_data(png_ptr, info_ptr, footer, 12);
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    return 0;
}

inline unsigned int read_chunk(FILE * f, CHUNK * pChunk)
{
    unsigned char len[4];
    pChunk->size = 0;
    pChunk->p = 0;
    if (fread(&len, 4, 1, f) != 1)
        return 0;

    pChunk->size = png_get_uint_32(len);
    if (pChunk->size > PNG_USER_CHUNK_MALLOC_MAX)
        return 0;
    pChunk->size += 12;
    pChunk->p = new unsigned char[pChunk->size];
    memcpy(pChunk->p, len, 4);
    if (fread(pChunk->p + 4, pChunk->size - 4, 1, f) == 1)
        return *(unsigned int *)(pChunk->p + 4);
    return 0;
}

int load_apng(const char * szIn, std::vector<Image>& img)
{
    FILE * f;
    unsigned int j, w, h, w0, h0, x0, y0;
    unsigned int delay_num, delay_den, dop, bop, imagesize;
    unsigned char sig[8];
    png_structp png_ptr;
    png_infop info_ptr;
    CHUNK chunk;
    CHUNK chunkIHDR;
    std::vector<CHUNK> chunksInfo;
    bool isAnimated = false;
    bool skipFirst = false;
    bool hasInfo = false;
    Image frameRaw;
    Image frameCur;
    Image frameNext;
    int res = -1;

    if ((f = fopen(szIn, "rb")) == nullptr)
        return -1;

    if (fread(sig, 1, 8, f) != 8 || png_sig_cmp(sig, 0, 8) != 0) {
        fclose(f);
        return -1;
    }

    unsigned int id = read_chunk(f, &chunkIHDR);
    if (!id)
    {
        fclose(f);
        return -1;
    }

    if (id == id_IHDR && chunkIHDR.size == 25)
    {
        w0 = w = png_get_uint_32(chunkIHDR.p + 8);
        h0 = h = png_get_uint_32(chunkIHDR.p + 12);

        if (!w || w > cMaxPNGSize || !h || h > cMaxPNGSize)
        {
            fclose(f);
            return res;
        }

        x0 = 0;
        y0 = 0;
        delay_num = 1;
        delay_den = 10;
        dop = 0;
        bop = 0;
        imagesize = w * h * 4;

        frameRaw.init(w, h, 4);

        if (!processing_start(png_ptr, info_ptr, (void *)&frameRaw, hasInfo, chunkIHDR, chunksInfo))
        {
            frameCur.init(w, h, 4);

            while ( !feof(f) )
            {
                id = read_chunk(f, &chunk);
                if (!id)
                    break;

                if (id == id_acTL && !hasInfo && !isAnimated)
                {
                    isAnimated = true;
                    skipFirst = true;
                }
                else
                if (id == id_fcTL && (!hasInfo || isAnimated))
                {
                    if (hasInfo)
                    {
                        if (!processing_finish(png_ptr, info_ptr))
                        {
                            frameNext.init(w, h, 4);

                            if (dop == 2)
                                memcpy(frameNext.p, frameCur.p, imagesize);

                            compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                            frameCur.delay_num = delay_num;
                            frameCur.delay_den = delay_den;
                            img.push_back(frameCur);

                            if (dop != 2)
                            {
                                memcpy(frameNext.p, frameCur.p, imagesize);
                                if (dop == 1)
                                    for (j=0; j<h0; j++)
                                        memset(frameNext.rows[y0 + j] + x0*4, 0, w0*4);
                            }
                            frameCur.p = frameNext.p;
                            frameCur.rows = frameNext.rows;
                        }
                        else
                        {
                            frameCur.free();
                            delete[] chunk.p;
                            break;
                        }
                    }

                    // At this point the old frame is done. Let's start a new one.
                    w0 = png_get_uint_32(chunk.p + 12);
                    h0 = png_get_uint_32(chunk.p + 16);
                    x0 = png_get_uint_32(chunk.p + 20);
                    y0 = png_get_uint_32(chunk.p + 24);
                    delay_num = png_get_uint_16(chunk.p + 28);
                    delay_den = png_get_uint_16(chunk.p + 30);
                    dop = chunk.p[32];
                    bop = chunk.p[33];

                    if (!w0 || w0 > cMaxPNGSize || !h0 || h0 > cMaxPNGSize
                        || x0 + w0 > w || y0 + h0 > h || dop > 2 || bop > 1)
                    {
                        frameCur.free();
                        delete[] chunk.p;
                        break;
                    }

                    if (hasInfo)
                    {
                        memcpy(chunkIHDR.p + 8, chunk.p + 12, 8);
                        if (processing_start(png_ptr, info_ptr, (void *)&frameRaw, hasInfo, chunkIHDR, chunksInfo))
                        {
                            frameCur.free();
                            delete[] chunk.p;
                            break;
                        }
                    }
                    else
                        skipFirst = false;

                    if (img.size() == (skipFirst ? 1 : 0))
                    {
                        bop = 0;
                        if (dop == 2)
                            dop = 1;
                    }
                }
                else
                if (id == id_IDAT)
                {
                    hasInfo = true;
                    if (processing_data(png_ptr, info_ptr, chunk.p, chunk.size))
                    {
                        frameCur.free();
                        delete[] chunk.p;
                        break;
                    }
                }
                else
                if (id == id_fdAT && isAnimated)
                {
                    png_save_uint_32(chunk.p + 4, chunk.size - 16);
                    memcpy(chunk.p + 8, "IDAT", 4);
                    if (processing_data(png_ptr, info_ptr, chunk.p + 4, chunk.size - 4))
                    {
                        frameCur.free();
                        delete[] chunk.p;
                        break;
                    }
                }
                else
                if (id == id_IEND)
                {
                    if (hasInfo && !processing_finish(png_ptr, info_ptr))
                    {
                        compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                        frameCur.delay_num = delay_num;
                        frameCur.delay_den = delay_den;
                        img.push_back(frameCur);
                    }
                    else
                        frameCur.free();

                    delete[] chunk.p;
                    break;
                }
                else
                if (notabc(chunk.p[4]) || notabc(chunk.p[5]) || notabc(chunk.p[6]) || notabc(chunk.p[7]))
                {
                    delete[] chunk.p;
                    break;
                }
                else
                if (!hasInfo)
                {
                    if (processing_data(png_ptr, info_ptr, chunk.p, chunk.size))
                    {
                        frameCur.free();
                        delete[] chunk.p;
                        break;
                    }
                    chunksInfo.push_back(chunk);
                    continue;
                }
                delete[] chunk.p;
            }
        }
        frameRaw.free();

        if (!img.empty())
            res = (skipFirst) ? 0 : 1;
    }

    for (unsigned int i=0; i<chunksInfo.size(); i++)
        delete[] chunksInfo[i].p;

    chunksInfo.clear();
    delete[] chunkIHDR.p;

    fclose(f);

    return res;
}

Fl_Image* LoadAPNG(const char *filename, Fl_Widget *canvas= nullptr)
{
    std::vector<Image> imgs;
    int res = load_apng(filename, imgs);
    if (res == -1)
        return nullptr;

    unsigned int num_frames = imgs.size();
    if (num_frames > 1) {
        Image img=imgs[0];
        unsigned int w = img.w;
        unsigned int h = img.h;
        auto* gif = new Fl_Anim_GIF_Image(filename, num_frames, w, h);
        for (unsigned int i = 0; i < num_frames; i++) {

            int delay_num = imgs[i].delay_num;
            int delay_den = imgs[i].delay_den;
            if (delay_den == 0) delay_den = 100;
            double delay = ((double)delay_num / (double)delay_den);
            if (delay_num == 0 < delay < 2.0)
                delay = 2.0;

            // TODO how/why are images ceasing playback?
            // TODO where is loopcount stored?
            // TODO the 20 multiplier is arbitrary but approximates the playback speed in Chrome
            // *10 would be tenths of second as per Fl_Anim_GIF_Image
            gif->add_frame(imgs[i].p, (int)delay * 20, w, h, true); // cleanup on release()
            imgs[i].p = nullptr; // cleanup
            imgs[i].free();
        }
        gif->start();
        gif->canvas(canvas, Fl_Anim_GIF_Image::Flags::DontResizeCanvas |
                                  Fl_Anim_GIF_Image::Flags::DontSetAsImage);
        return gif;
    }
    else {
        Image img=imgs[0];

        // valgrind memory leak
        Fl_RGB_Image *ours = new Fl_RGB_Image((const uchar *)img.p, img.w, img.h, img.bpp, 0);
        ours->alloc_array = 1; // cleanup on release()
        img.p = nullptr; // cleanup
        img.free();
        return ours;
    }

    // prevent compiler detects as error or warning.
    return nullptr;
}

#pragma clang diagnostic pop