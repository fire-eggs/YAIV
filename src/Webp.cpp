//
// Created by kevin on 5/11/21.
//

#include <fstream>
#include <FL/fl_utf8.h>

extern "C" {
#include <webp/decode.h>
#include <webp/demux.h>
};

#include "Fl_Anim_GIF_Image.h"
#include "Webp.h"

/*
 * Overload Fl_RGB_Image for webp. Webp uses malloc() to allocate its buffer,
 * and the Fl_RGB_Image dtor will attempt to free it using delete.
 */
class Webp_Image : public Fl_RGB_Image
{
public:
    Webp_Image(const uchar *bits, int W, int H, int D=3, int LD=0) :
            Fl_RGB_Image(bits, W, H, D, LD) {}

    ~Webp_Image() {
        // TODO fully replicates the base dtor, consider calling it anyway...
        uncache();
        free((void *)array);
    }
};

bool strendswith(const char* str, const char* suffix)
{
    uint64_t len = strlen(str); // TODO portable?
    uint64_t suffixlen = strlen(suffix);
    if (suffixlen > len)
    {
        return false;
    }

    str += (len - suffixlen);
    return strcmp(str, suffix) == 0;
}

#define W_CHAR wchar_t  // WCHAR without underscore might already be defined.
#define TO_W_CHAR(STR) (L##STR)
#define WFOPEN(ARG, OPT) _wfopen((const W_CHAR*)ARG, TO_W_CHAR(OPT))

int ImgIoUtilReadFile(const char* const file_name,
                      const uint8_t** data, size_t* data_size) {
    int ok;
    uint8_t* file_data;
    size_t file_size;
    FILE* in;

    if (data == NULL || data_size == NULL) return 0;
    *data = NULL;
    *data_size = 0;

    in = fl_fopen(file_name, "rb");
    if (in == NULL) {
        return 0;
    }
    fseek(in, 0, SEEK_END);
    file_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    // we allocate one extra byte for the \0 terminator
    file_data = (uint8_t*)malloc(file_size + 1);
    if (file_data == NULL) {
        fclose(in);
        return 0;
    }
    ok = (fread(file_data, file_size, 1, in) == 1);
    fclose(in);

    if (!ok) {
        free(file_data);
        return 0;
    }
    file_data[file_size] = '\0';  // convenient 0-terminator
    *data = file_data;
    *data_size = file_size;
    return 1;
}

int LoadWebP(const char* const in_file,
             const uint8_t** data, size_t* data_size,
             WebPBitstreamFeatures* bitstream) {

    {
        char header[13];
        FILE *in = fl_fopen(in_file, "rb");
        if (in == NULL) {
            return 0;
        }
        bool ok = fread(header, 13, 1, in) == 1;
        fclose(in);
        if (!ok)
            return 0;
        bool riff = header[0] == 'R' &&
                    header[1] == 'I' &&
                    header[2] == 'F' &&
                    header[3] == 'F';
        if (!riff) return 0;
        bool webp = header[8] == 'W' &&
                    header[9] == 'E' &&
                    header[10] == 'B' &&
                    header[11] == 'P';
        if (!webp) return 0;
    }



    VP8StatusCode status;
    WebPBitstreamFeatures local_features;
    if (!ImgIoUtilReadFile(in_file, data, data_size))
        return 0;

    if (bitstream == NULL) {
        bitstream = &local_features;
    }

    status = WebPGetFeatures(*data, *data_size, bitstream);
    if (status != VP8_STATUS_OK) {
        free((void*)*data);
        *data = NULL;
        *data_size = 0;
        return 0;
    }
    return 1;
}

static int IsWebP(const uint8_t* const data, size_t data_size)
{
    return (WebPGetInfo(data, data_size, NULL, NULL) != 0);
}

typedef struct
{
    uint8_t* rgba;         // Decoded and reconstructed full frame.
    int duration;          // Frame duration in milliseconds.
    int is_key_frame;      // True if this frame is a key-frame.
} DecodedFrame;

typedef struct
{
    uint32_t canvas_width;
    uint32_t canvas_height;
    uint32_t bgcolor;
    uint32_t loop_count;
    DecodedFrame* frames;
    uint32_t num_frames;
    void* raw_mem;
} AnimatedImage;

static int CheckSizeForOverflow(uint64_t size) { return (size == (size_t)size); }

static int AllocateFrames(AnimatedImage* const image, uint32_t num_frames)
{
    uint32_t i;
    uint8_t* mem = nullptr;
    DecodedFrame* frames = nullptr;
    const uint64_t rgba_size  =	(uint64_t)image->canvas_width * 4 * image->canvas_height;
    const uint64_t total_size = (uint64_t)num_frames * rgba_size * sizeof(*mem);
    const uint64_t total_frame_size = (uint64_t)num_frames * sizeof(*frames);
    if (!CheckSizeForOverflow(total_size) ||
        !CheckSizeForOverflow(total_frame_size)) {
        return 0;
    }

    mem = (uint8_t*)malloc((size_t)total_size);
    frames = (DecodedFrame*)malloc((size_t)total_frame_size);

    if (!mem || !frames) {
        free(mem);
        free(frames);
        return 0;
    }
    WebPFree(image->raw_mem);
    image->num_frames = num_frames;
    image->frames = frames;
    for (i = 0; i < num_frames; ++i) {
        frames[i].rgba = mem + i * rgba_size;
        frames[i].duration = 0;
        frames[i].is_key_frame = 0;
    }
    image->raw_mem = mem;
    return 1;
}

AnimatedImage* ReadAnimatedImage(const uint8_t* const data, size_t data_size)
{
    if (!IsWebP(data, data_size))
        return nullptr;

    struct WebPData webp_data;
    webp_data.bytes = data;
    webp_data.size = data_size;

    WebPAnimDecoder* dec = WebPAnimDecoderNew(&webp_data, nullptr);
    if (!dec)
        return nullptr;

    WebPAnimInfo anim_info;
    if (!WebPAnimDecoderGetInfo(dec, &anim_info))
    {
        WebPAnimDecoderDelete(dec);
        return nullptr;
        //fprintf(stderr, "Error getting global info about the animation\n");
    }

    // TODO these go into some sort of 'animated image' object - Fl_Anim_GIF_Image ?
    auto* image = new AnimatedImage();
    memset(image, 0, sizeof(*image));

    // Animation properties.
    image->canvas_width = anim_info.canvas_width;
    image->canvas_height = anim_info.canvas_height;
    image->loop_count = anim_info.loop_count;
    image->bgcolor = anim_info.bgcolor;

    // Allocate frames.
    if (!AllocateFrames(image, anim_info.frame_count))
    {
        delete image;
        WebPAnimDecoderDelete(dec);
        return nullptr;
    }

    // Decode frames.
    uint32_t frame_index = 0;
    int prev_frame_timestamp = 0;
    while (WebPAnimDecoderHasMoreFrames(dec))
    {
        DecodedFrame* curr_frame;
        uint8_t* curr_rgba;
        uint8_t* frame_rgba;
        int timestamp;

        if (!WebPAnimDecoderGetNext(dec, &frame_rgba, &timestamp))
        {
            delete image;
            WebPAnimDecoderDelete(dec);
            return nullptr;
        }

        curr_frame = &image->frames[frame_index];
        curr_rgba = curr_frame->rgba;
        curr_frame->duration = timestamp - prev_frame_timestamp;
        curr_frame->is_key_frame = 0;  // Unused.
        memcpy(curr_rgba, frame_rgba, image->canvas_width * 4 * image->canvas_height);

        //// Needed only because we may want to compare with GIF later.
        //CleanupTransparentPixels((uint32_t*)curr_rgba,
        //	image->canvas_width, image->canvas_height);

        ++frame_index;
        prev_frame_timestamp = timestamp;
    }

    WebPAnimDecoderDelete(dec);
    return image;
}

VP8StatusCode DecodeWebP(const uint8_t* const data, size_t data_size,
                         WebPDecoderConfig* const config)
{
    if (config == nullptr)
        return VP8_STATUS_INVALID_PARAM;
    return WebPDecode(data, data_size, config);
}

Fl_Image* LoadWebp(const char* filename, Fl_Widget *canvas=nullptr)
{
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config))
        return nullptr;
    WebPBitstreamFeatures* const bitstream = &config.input;

    const uint8_t* data;
    size_t data_size = 0;
    int res = LoadWebP(filename, &data, &data_size, bitstream);

    if (!res)
        return nullptr;

    if (bitstream->has_animation)
    {
        AnimatedImage *image = ReadAnimatedImage(data, data_size);
        if (!image)
            return nullptr;

        // convert to FL_Animate_GIF_Image compatible
        unsigned int W = image->canvas_width;
        unsigned int H = image->canvas_height;
        auto* gif = new Fl_Anim_GIF_Image(filename, (int)image->loop_count, W, H);
        for (unsigned int i = 0; i < image->num_frames; i++)
        {
            DecodedFrame frame = image->frames[i];
            gif->add_frame(frame.rgba, frame.duration, W, H, true);
        }
        gif->start();
        gif->canvas(canvas, Fl_Anim_GIF_Image::Flags::DontResizeCanvas |
                            Fl_Anim_GIF_Image::Flags::DontSetAsImage);

        WebPFree((void*)data); // valgrind mem leak
        return gif;
    }

    WebPDecBuffer* const output_buffer = &config.output;
    output_buffer->colorspace = bitstream->has_alpha ? MODE_RGBA : MODE_RGB;
    VP8StatusCode status = VP8_STATUS_OK;

    status = DecodeWebP(data, data_size, &config);
    if (status)
    {
        WebPFreeDecBuffer(output_buffer);
        WebPFree((void*)data);
        return nullptr;
    }

    Webp_Image* ours = new Webp_Image(output_buffer->u.RGBA.rgba,
                                      output_buffer->width, output_buffer->height,
                                      bitstream->has_alpha ? 4 : 3);

    WebPFree((void*)data);
    return ours;
}
