//
// Created by kevin on 5/12/21.
//
#include <string.h>

#include <FL/filename.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Image_Surface.H>

#include "XBox.h"
#include "MyW.h"
#include "list_rand.h"
#include "prefs.h"
#include "MostRecentPaths.h"

#define snprintf_nowarn(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

extern XBox *_b2;
extern MyW *_w;
extern Prefs *_prefs;
MostRecentPaths *_mru; // TODO consider member variable

Fl_Image *img;  // Original image TODO consider member variable
Fl_Image *showImg; // rotated/scaled image TODO consider member variable

static char name[1024]; // filename load TODO consider member variable

// TODO these go to some file list container
int current_index;
char fold[FL_PATH_MAX];
struct dirent **file_list;
int file_count;

int filename_path(const char* buf, char *to) { // TODO hack pending adding to FLTK
    const char *p = buf + strlen(buf) - 1;
    for (; *p != '/' && p != buf; --p) // TODO slash is possible '\' under windows
        ;
    if (p == buf) return 0;
    strncpy(to, buf, (p-buf)+1);
    to[(p-buf)] = '\0';
    return 1;
}

int removeFolders(struct dirent *entry) {
    // TODO this is a hack, we're not provided the base path
    const char * out = fl_filename_ext(entry->d_name);
    bool val = out != nullptr && *out != 0 && (out[1] != 0);
    return val;
}

void logit(const char *format, char *arg) // TODO varargs
{
    FILE *f = fopen("yaiv.log", "a+");
    fprintf(f, format, arg);
    fputs("\n", f);
    fclose(f);
}

void load_current() {
    if (!file_list || file_count < 1)
        return;

    //current_index = std::min(std::max(current_index,2), file_count-1);
    current_index = std::min(std::max(current_index,0), file_count-1);

    char n[FL_PATH_MAX<<2];
    if (fold[strlen(fold)-1] == '/')
        fold[strlen(fold)-1] = 0x0;
    sprintf(n, "%s/%s", fold, file_list[current_index]->d_name);
    //puts(n);

    logit("LC-file:%s", n);

    strcpy(::_w->filename, n);

// TODO consider re-enabling or doing something rather than pass folder name to image processing

    if (fl_filename_isdir(n)) {
        _b2->align(FL_ALIGN_CENTER);
        _b2->label("@fileopen"); // show a generic folder
        _b2->labelsize(64);
        _b2->labelcolor(FL_LIGHT2);
        _b2->image(nullptr,nullptr);
        _b2->redraw();
        goto dolabel;
    }

    {
        // TODO done in end of file chooser
        //_b2->take_focus();

        Fl_Anim_GIF_Image::min_delay = 0.01;
        Fl_Anim_GIF_Image::animate = true;

        img = loadFile(n, _b2);

        if (!img)
        {
            // failed to load
            _b2->align(FL_ALIGN_CENTER);
            _b2->label("@filenew");
            _b2->labelsize(64);
            _b2->labelcolor(FL_RED);
            _b2->image(nullptr,nullptr);
            _b2->redraw();
            goto dolabel;
        }

        _b2->label(nullptr);
        auto* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(img);
        _b2->image(img, animgif);
        _b2->redraw();

#if false
        bool anim = animgif != nullptr;

        // this is a hack to force the box to resize which forces the animated image to center in the box & clear background
        // TODO how to make this happen cleanly

//        if (anim)
//            _w->size(_w->w()+1, _w->h()+1);
#endif
    }
dolabel:
    char lbl[1000];
    lbl[0] = 0;
    _w->label(_b2->getLabel(n, lbl, sizeof(lbl)));

    _b2->rotation = 0; // TODO keep rotation?
}

int find_file(const char *n) {
    // determine the index in the file_list of the given filename
    const char *outfn = fl_filename_name(n);
    for (int i=0; i < file_count; i++)
        if (strcmp(file_list[i]->d_name, outfn) == 0)
            return i;
    return 0;
}

void load_filelist(const char *n) {

    if (file_list)
        fl_filename_free_list(&file_list, file_count);

    if (!fl_filename_isdir(n))
        filename_path(n, fold);
    else
        fl_filename_absolute(fold, FL_PATH_MAX, n);

    file_count = fl_filename_list(fold, &file_list, fl_numericsort, removeFolders); // TODO how to filter for images?
}

void load_file(const char *n) {

    load_filelist(n); // TODO background process
    current_index = 0;
    if (!fl_filename_isdir(n))
        current_index = find_file(n);
    load_current();

    // TODO don't add to MRU if unsuccessful load
    // Update the MRU list
    _mru->Add(n);
    _mru->Save();
}

void next_image() {
    current_index = std::min(current_index+1, file_count-1);
    load_current();
}

void prev_image() {
    current_index = std::max(current_index-1, 0);
    load_current();
}

void file_cb(const char *n) {
    if (!strcmp(name,n)) return;
    load_file(n); // TODO intermediate folders are being displayed/remembered; only want selected via OK?
    strcpy(name,n);
}

void button_cb(Fl_Widget *,void *) {
    fl_file_chooser_callback(file_cb);
    Fl_File_Chooser::sort = fl_numericsort;
    //const char *fname =
    fl_file_chooser("Image file?","*.{bm,bmp,gif,jpg,png,webp"
                                  #ifdef FLTK_USE_SVG
                                  ",svg"
#ifdef HAVE_LIBZ
                                      ",svgz"
#endif // HAVE_LIBZ
                                  #endif // FLTK_USE_SVG
                                  "}\tGIF Files (*.gif)", name);
    //puts(fname ? fname : "(null)"); fflush(stdout);
    fl_file_chooser_callback(nullptr);
    _b2->take_focus();
}

int XBox::handle(int msg) {

    if (msg == FL_FOCUS || msg == FL_UNFOCUS) // TODO must this go before Fl_Group::handle?
    {
        //printf("box:focus %d\n", msg);
        return 1;
    }

    int ret = Fl_Group::handle(msg);

    switch (msg) {
        case FL_DND_ENTER:
        case FL_DND_DRAG:
        case FL_DND_LEAVE:
        case FL_DND_RELEASE:
            return 1; // accept drag-and-drop
            break;

        case FL_PASTE: {
            // input will be a URL of form "file://<path>\n" OR "file://<path>\r\n"
            // THERE MAY BE MORE THAN ONE URL! taking only the first
            // TODO confirm that windows DND includes the "file://" prefix!
            const char *urls = Fl::event_text();
            if (strncmp(urls, "file://", 7) != 0)
                return 1; // not a local file, do nothing

            int len = strlen(urls);
            int i = 0;
            while (urls[i] != '\r' && urls[i] != '\n' && i < len) i++;
            char *fpath = new char[i + 1 - 7];
            strncpy(fpath, urls + 7, i + 1 - 7);
            fpath[i - 7] = '\0';
            load_file(fpath);
            delete[] fpath;
            take_focus();
            return 1;
        }
            break;

        case FL_PUSH:
            if (Fl::event_button() == FL_RIGHT_MOUSE) {
                do_menu();
                return 1;
            }
            break;
    }

    if (msg == FL_KEYDOWN) {

        //printf("keyboard '%s'\n", Fl::event_text());

        switch (Fl::event_key())
        {
            case 'q':
                exit(0);

            case 'c':
                draw_check = !draw_check;
                redraw();
                return 1;

            case 's':
                next_scale();
                return 1;

            case 'n':
                draw_center = !draw_center;
                if (!draw_center)
                    deltax = deltay = 0;
                redraw();
                return 1;

            case 'r':
            {
                if (!file_list || file_count<=1)
                    break;

                struct dirent **newlist = list_randomize(file_list, file_count);

                for (int i=0; i<file_count; i++)
                    file_list[i] = newlist[i];

                free(newlist);
                load_current();
            }
            return 1;

            case FL_Right:
                //printf("Box: Right arrow, state:%d\n", Fl::event_state());
                if (Fl::event_state() & FL_CTRL)
                {
                    deltax -= 10; // direction matches FEH
                }
                else
                {
                    next_image();
                    deltax = deltay = 0; // TODO allow unchanged?
                }
                redraw();
                return 1;

            case FL_Left: // TODO consider for pan
                if (Fl::event_state() & FL_CTRL)
                {
                    deltax += 10; // direction matches FEH
                }
                else
                {
                    prev_image();
                    deltax = deltay = 0; // TODO allow unchanged?
                }
                redraw();
                return 1;

            case FL_Up:
                //printf("Box: Up arrow, state:%d\n", Fl::event_state());
                if (Fl::event_state() & FL_CTRL)
                {
                    deltay += 10; // direction matches FEH
                }
                redraw();
                return 1;

            case FL_Down:
                if (Fl::event_state() & FL_CTRL)
                {
                    deltay -= 10; // direction matches FEH
                }
                redraw();
                return 1;

            case FL_Page_Down:
            case ' ':
                next_image();
                return 1;

            case FL_Page_Up:
            case FL_BackSpace:
                prev_image();
                return 1;

            case FL_Home:
                current_index = INT_MIN + 1;
                prev_image();
                return 1;

            case FL_End:
                current_index = INT_MAX - 1;
                next_image();
                return 1;

            case 't':
                nextRotation();
                return 1;

            case 'z':
                nextTkScale();
                return 1;

            case FL_Escape: // escape to NOT close app
                return 1;

            case 'b':
                ::_w->toggle_border();
                return 1;
        }
    }


    return ret;
}


void XBox::MenuCB(Fl_Widget *window_p, void *userdata) { // TODO make dynamic

    switch((long int)userdata) {
        case MI_LOAD:
            button_cb(window_p, (void *)userdata);
            break;

        case MI_COPYPATH:
        {
            if (!file_list || file_count < 1)
                break;

            char n[FL_PATH_MAX<<2];
            sprintf(n, "%s/%s", fold, file_list[current_index]->d_name);
            Fl::copy(n, strlen(n), 1);
        }
            break;

        case MI_GOTO:		    // TODO hacky
        {
            int dex = current_index + 1;
            char def[256];
            sprintf(def, "%d", dex);
            const char* res = fl_input("Goto image:", def);
            if (res)
            {
                try
                {
                    int val = std::stoi(res);
                    current_index = val - 1;
                    load_current();
                }
                catch (std::exception& e)
                {
                }
            }
        }
            break;

        case MI_OPTIONS:		    // TODO nyi
            break;

        case MI_FAV0: case MI_FAV1: case MI_FAV2:
        case MI_FAV3: case MI_FAV4: case MI_FAV5:
        case MI_FAV6: case MI_FAV7: case MI_FAV8: case MI_FAV9:
            {
            int path = (long)userdata - MI_FAV0;
            char** mru = _mru->getAll(); // TODO return a single path
            printf("MRU: %s\n", mru[path]);
            load_file(mru[path]);
            }
            break;
    }
}

char * XBox::getLabel(char *n, char *buff, int buffsize)
{
    int outzoom = (int)(_zoom * 100.0 + 0.5);
    int w = _img ? _img->w() : 0;
    int h = _img ? _img->h() : 0;
    int depth = _img ? _img->d() : 0;

    char scaletxt[10];
    char * res = humanScale(draw_scale, scaletxt, sizeof(scaletxt)-1);

    if (res == nullptr || img == nullptr)
        sprintf(buff, "%d/%d - huh? - %s", current_index+1,file_count,n);
    else
    {
        char nicesize[10];
        humanSize(n, nicesize, sizeof(nicesize)-1);

        char Zscaletxt[10];
        humanZScale(imgtkScale, Zscaletxt, sizeof(Zscaletxt)-1);

        // for the image label currently don't have filename, don't display extra dashes
        snprintf_nowarn(buff, buffsize, "%d/%d - [%dx%dx%d%s%s] - (%d%%)[%s][%s]%s%s",
                        current_index+1, file_count, w, h, depth,
                        n[0] == '\0' ? "" : " - ",
                        nicesize, outzoom, scaletxt, Zscaletxt, n[0] == '\0' ? "" : " - ", n);
    }
    return buff;
}

void XBox::next_scale() {
    draw_scale = (ScaleMode)((int)draw_scale + 1);
    if (draw_scale == ScaleMode::MAX)
        draw_scale = ScaleMode::None;

    updateLabel();
    updateImage();
    redraw();
}

void XBox::nextTkScale() {
    imgtkScale++;
    if (imgtkScale > 6) imgtkScale = 0;

    updateLabel();
    updateImage();
    redraw();
}

void XBox::nextRotation() {
    rotation++;
    updateLabel();
    updateImage();
    redraw();
}

void XBox::updateLabel() {
    ::_w->updateLabel(); // TODO super hack
}

void XBox::image(Fl_Image *newImg, Fl_Anim_GIF_Image *animimg)
{
    wipeShowImage();

    if (_anim)
    {
        //Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(_img);
        _anim->stop();
        _anim->canvas(nullptr);
//        Fl_Anim_GIF_Image::animate = false;
        delete _anim;
    }

    else if (_img)
        _img->release();

    _img = newImg;
    _anim = animimg;

    rotation = 0; // TODO better place
    updateImage();
}

void XBox::wipeShowImage() {
    if (_showImg && _showImg != _img) {
        if (requiresDiscard) {
            auto *tmp = (Fl_RGB_Image *) _showImg;
            discard_user_rgb_image(tmp);
        } else {
            _showImg->release();
        }
    }
    _showImg = nullptr;
}

void XBox::updateImage() {

    // 1. dispose of any existing showImg because we're building a new one
    wipeShowImage();

    requiresDiscard = false;
    if (!_img) {
        _showImg = nullptr;
        return;
    }

    // 2. rotate the original
    // TODO rotation of _anim : problematic as frames change via draw(), not here
    auto *rimg = (Fl_RGB_Image *)_img;
    if (rotation && !_anim)
    {
        switch (rotation)
        {
            case 1:
                rimg = rotate90((Fl_RGB_Image*)_img);
                break;
            case 2:
                rimg = rotate180((Fl_RGB_Image*)_img);
                break;
            case 3:
                rimg = rotate270((Fl_RGB_Image*)_img);
                break;
            default:
                rotation = 0;
                break;
        }

        if (rotation != 0) {
            requiresDiscard = true;
        }
        _showImg = rimg;
    }
    else _showImg = _img->copy();

    // 3. scale showimage

    // TODO need to keep _zoom if no scaling [user zoom]
    _zoom = 1.0;

    switch (draw_scale) {
        case ScaleMode::None:
            {
                if (_anim)
                    _anim->scale(_anim->data_w(), _anim->data_h());
                else
                    _showImg->scale(_showImg->data_w(),_showImg->data_h());
            }
            break;

        case ScaleMode::Wide:
            {
                int new_w = w();
                int new_h = (int)((double)_showImg->h() * w() / (double)_showImg->w());
                if (_anim) {
                    _anim->scale(new_w, new_h, 1, 1);
                    _zoom = (double)_anim->w() / _anim->data_w();
                }
                else {
                    _showImg->scale(new_w, new_h, 1, 1);
                    _zoom = (double)_showImg->w() / _showImg->data_w();
                }
            }
            break;

        case ScaleMode::High:
            {
                int new_h = h();
                int new_w = (int)((double)_showImg->w() * h() / (double)_showImg->h());
                if (_anim) {
                    _anim->scale(new_w, new_h, 1, 1);
                    _zoom = (double)_anim->h() / _anim->data_h();
                }
                else {
                    _showImg->scale(new_w, new_h, 1, 1);
                    _zoom = (double)_showImg->h() / _showImg->data_h();
                }
            }
            break;

        case ScaleMode::Fit:
            {
                if (_anim) {
                    _anim->scale(w(), h(), 1, 1);
                    _zoom = std::max( (double)_anim->w() / _anim->data_w(),
                                      (double)_anim->h() / _anim->data_h());
                }
                else {
                    _showImg->scale(w(),h(), 1, 1);
                    _zoom = std::max( (double)_showImg->w() / _showImg->data_w(),
                                      (double)_showImg->h() / _showImg->data_h());
                }
            }
            break;

        case ScaleMode::Auto:
            {
                if (_anim) {
                    _anim->scale(w(), h());
                    _zoom = std::max( (double)_anim->w() / _anim->data_w(),
                                      (double)_anim->h() / _anim->data_h());
                }
                else {
                    _showImg->scale(w(),h());
                    _zoom = std::max( (double)_showImg->w() / _showImg->data_w(),
                                      (double)_showImg->h() / _showImg->data_h());
                }
            }
            break;

        default:
        case MAX:
            break;
    }

    // Draw the checker and image in a surface and use the surface to draw later
    // TODO animated image frames update in draw(), not here, so skip for now
    if (!_anim) {
        Fl_Image_Surface *imgSurf = new Fl_Image_Surface(_showImg->w(), _showImg->h());
        Fl_Surface_Device::push_current(imgSurf);
        if (draw_check) {
            drawChecker(0, 0, _showImg->w(), _showImg->h());
        _showImg->draw(0,0);
        _showImg->release();
        _showImg = imgSurf->image();
        Fl_Surface_Device::pop_current();
        delete imgSurf;
        }
    }
}

void XBox::resize(int x,int y,int w,int h) {
    Fl_Group::resize(x,y,w,h);
    updateImage(); // e.g. resize to fit/wide/high
}

void XBox::do_menu() {

    // 1. find the submenu in the "master" menu
    int i;
    for (i = 0; i < right_click_menu->size(); i++)
    {
        if (strcmp(right_click_menu[i].text,"Last Used"))
            continue;
        break;
    }

    size_t numfavs = _mru->getCount();

    // create a new menu
    int newCount = right_click_menu->size() + numfavs;
    Fl_Menu_Item* dyn_menu = new Fl_Menu_Item[newCount];

    // make sure the rest of the allocated menu is clear
    for (int j = 0; j < newCount; j++)
        memset(&(dyn_menu[j]), 0, sizeof(Fl_Menu_Item));

    // initialize it with the static menu contents
    for (int j = 0; j <= i; j++)
    {
        dyn_menu[j] = right_click_menu[j];
        dyn_menu[j].callback(MenuCB, (void *)(MI_LOAD + j)); // TODO just 'j'?
    }

    // TODO if numfavs == 0 disable the "last used"

    char** favs = _mru->getAll();
    for (long unsigned int j = 0; j < numfavs; j++)
    {
        dyn_menu[i + 1 + j].label(favs[j]);
        dyn_menu[i + 1 + j].callback(MenuCB);
        dyn_menu[i + 1 + j].argument(MI_FAV0 + j);
    }

    // show the menu

    const Fl_Menu_Item *m = dyn_menu->popup(Fl::event_x(), Fl::event_y(), "YAIV", nullptr, nullptr);
    if (m && m->callback())
        m->do_callback(this, m->user_data());
}

XBox::XBox(int x, int y, int w, int h) : Fl_Group(x,y,w,h)
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

    _mru = new MostRecentPaths(_prefs); // TODO consider singleton
}
