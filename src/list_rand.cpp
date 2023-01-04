//
// Created by kevin on 5/11/21.
//

#include <cstdlib> // srand, rand
#include <ctime> // time

#include <FL/Fl_File_Chooser.H>  // dirent
#ifdef FLTK_USE_SVG
#include <FL/Fl_SVG_Image.H>
#endif

#include "list_rand.h"
#include "Webp.h"
#include "apng.h"

// NOTE borrowed from feh gib_list.c gib_list_randomize
// TODO needs to go in list container
struct dirent ** list_randomize(struct dirent ** list, int count)
{
    if (!list)
        return (NULL);
    if (count <= 1)
        return (list);

    static bool first_rand = true;

    if (first_rand)
    {
        srand(time(NULL));
        first_rand = false;
    }

    struct dirent **farray = (struct dirent **) malloc(sizeof(struct dirent *) * count);
    for (int i = 0; i < count; i++)
        farray[i] = list[i];

    for (int i = 0; i < count - 1; i++)
    {
        int r = i + (rand() % (count - i));
        struct dirent *t = farray[r];
        farray[r] = farray[i];
        farray[i] = t;
    }

    return farray;
}

#include <FL/Fl_GIF_Image.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_PNM_Image.H>

#ifndef HAVE_LIBPNG
#define HAVE_LIBPNG
#endif
#include <FL/Fl_PNG_Image.H>
#ifndef HAVE_LIBJPEG
#define HAVE_LIBJPEG
#endif
#include <FL/Fl_JPEG_Image.H>

enum ImageFormat
{
    FAIL = 0,
    GIF,
    BMP,
    PNM,
    PNG,
    JPG,
    SVG,
    APNG,
    WEBP
};

ImageFormat readImageHeader(uchar *header, int headerlen)
{
    if (headerlen < 6) // not a valid image
        return FAIL;

    if (memcmp(header, "GIF87a", 6) == 0 ||
        memcmp(header, "GIF89a", 6) == 0) // GIF file
        return GIF;

    if (memcmp(header, "BM", 2) == 0)     // BMP file
        return BMP;

    if (header[0] == 'P' && header[1] >= '1' && header[1] <= '7')
        // Portable anymap
        return PNM;

#ifdef HAVE_LIBPNG
    if (memcmp(header, "\211PNG", 4) == 0)// PNG file
    return PNG;
#endif // HAVE_LIBPNG

#ifdef HAVE_LIBJPEG
    if (memcmp(header, "\377\330\377", 3) == 0 && // Start-of-Image
      header[3] >= 0xc0 && header[3] <= 0xfe)   // APPn .. comment for JPEG file
    return JPG;
#endif // HAVE_LIBJPEG

    bool riff = header[0] == 'R' &&
                header[1] == 'I' &&
                header[2] == 'F' &&
                header[3] == 'F';
    bool webp = header[8] == 'W' &&
                header[9] == 'E' &&
                header[10] == 'B' &&
                header[11] == 'P';
    if (riff & webp)
        return WEBP;

#ifdef FLTK_USE_SVG
  // KBR handle BOM
  if (memcmp(header, "<svg", 4) == 0 ||
      memcmp(&(header[3]), "<svg", 4) == 0 ||
      memcmp(header, "<?xml", 5) == 0 ||
      memcmp(&(header[3]), "<?xml", 5) == 0 )
  {
    return SVG;
  }
  
  #  if defined(HAVE_LIBZ)
    if (header[0] == 0x1f && header[1] == 0x8b) { // denotes gzip'ed data
        int fd = fl_open_ext(name, 1, 0);
        if (fd < 0) 
            return FAIL;
        gzFile gzf =  gzdopen(fd, "r");
        if (gzf) {
        gzread(gzf, header, headerlen);
        gzclose(gzf);
        }
    }
  #  endif // HAVE_LIBZ

#endif // FLTK_USE_SVG
    
    return FAIL;
}

#include <fcntl.h> // O_RDONLY
#include <unistd.h>  // close()

ImageFormat getImageFormatAt(int dirfd, const char *filename)
{
    uchar header[64];
    int fd = openat(dirfd, filename, O_DIRECT | O_NOATIME | O_NONBLOCK | O_RDONLY);
    int count = read(fd, header, 63);
    close(fd);

    return readImageHeader(header, count);
}


ImageFormat getImageFormat(const char *filename)
{
    FILE *fp;
    uchar header[64];

    if ((fp = fl_fopen(filename, "rb")) == NULL)
        return FAIL;
    setvbuf(fp, (char *)header, _IONBF, 63);
    int count = fread(header, 1, sizeof(header), fp);
    fclose(fp);

    return readImageHeader(header, count);
}

Fl_Image *                                      // O - Image, if found
fl_check_images(const char *name,               // I - Filename
                uchar      *header,             // I - Header data from file
                int headerlen) {                // I - Amount of data

    ImageFormat itype = readImageHeader(header,headerlen);
    switch (itype)
    {
        case GIF:
            return new Fl_GIF_Image(name);
        case BMP:
            return new Fl_BMP_Image(name);
        case PNM:
            return new Fl_PNM_Image(name);
        case PNG:
            return new Fl_PNG_Image(name);
        case JPG:
            return new Fl_JPEG_Image(name);
        case SVG:
            return new Fl_SVG_Image(name);
        case FAIL:
        default:
            return nullptr;
    }
#if 0                    
    if (headerlen < 6) // not a valid image
        return nullptr;

    if (memcmp(header, "GIF87a", 6) == 0 ||
        memcmp(header, "GIF89a", 6) == 0) // GIF file
        return new Fl_GIF_Image(name);

    if (memcmp(header, "BM", 2) == 0)     // BMP file
        return new Fl_BMP_Image(name);

    if (header[0] == 'P' && header[1] >= '1' && header[1] <= '7')
        // Portable anymap
        return new Fl_PNM_Image(name);

#ifdef HAVE_LIBPNG
    if (memcmp(header, "\211PNG", 4) == 0)// PNG file
    return new Fl_PNG_Image(name);
#endif // HAVE_LIBPNG

#ifdef HAVE_LIBJPEG
    if (memcmp(header, "\377\330\377", 3) == 0 && // Start-of-Image
      header[3] >= 0xc0 && header[3] <= 0xfe)   // APPn .. comment for JPEG file
    return new Fl_JPEG_Image(name);
#endif // HAVE_LIBJPEG

#ifdef FLTK_USE_SVG
    #  if defined(HAVE_LIBZ)
  if (header[0] == 0x1f && header[1] == 0x8b) { // denotes gzip'ed data
    int fd = fl_open_ext(name, 1, 0);
    if (fd < 0) return NULL;
    gzFile gzf =  gzdopen(fd, "r");
    if (gzf) {
      gzread(gzf, header, headerlen);
      gzclose(gzf);
    }
  }
#  endif // HAVE_LIBZ

// KBR handle BOM
if (memcmp(header, "<svg", 4) == 0 ||
    memcmp(&(header[3]), "<svg", 4) == 0 ||
    memcmp(header, "<?xml", 5) == 0 ||
    memcmp(&(header[3]), "<?xml", 5) == 0 )
{
    return new Fl_SVG_Image(name);
}
#endif // FLTK_USE_SVG

    return 0;
#endif    
}

// TODO go into "file loader" module
Fl_Image *loadFile(char *filename, XBox *owner)
{
    // 1. Try to open as webp [animated or not]
    Fl_Image *img = LoadWebp(filename, owner);
    if (img)
        return img;

    // 2. Try to open as animated PNG
    img = LoadAPNG(filename, owner);
    if (img)
        return img;

    // 3. Try to open as animated gif
    Fl_Anim_GIF_Image *animgif = new Fl_Anim_GIF_Image(filename, nullptr, Fl_Anim_GIF_Image::Start);
    if (animgif && animgif->valid() && animgif->is_animated()) // TODO can use fail() ?
    {
        animgif->canvas(owner, Fl_Anim_GIF_Image::Flags::DontResizeCanvas |
                             Fl_Anim_GIF_Image::Flags::DontSetAsImage);
        return animgif;
    }

    delete animgif;

    FILE *fp;
    uchar header[64];
    int count=0;

    if ((fp = fl_fopen(filename, "rb")) != NULL) {
        count = fread(header, 1, sizeof(header), fp);
        fclose(fp);
        if (count == 0)
            return nullptr;
    } else {
        return nullptr;
    }

    return fl_check_images(filename, header, count);
}
