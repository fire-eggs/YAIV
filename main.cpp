
#include <cstring>
#include <clocale>     // setlocale()..

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/fl_message.H>
#include <FL/Fl_Image.H>

#include "Fl_Anim_GIF_Image.h"
#include "rotate.h"
#include "rescaler.h"
#include "Webp.h"
#include "list_rand.h"
#include "checker.h"

// TODO these go to some file list container
int current_index;
char fold[FL_PATH_MAX];
struct dirent **file_list;
int file_count;

void button_cb(Fl_Widget *,void *); // hack
void next_image(); // hack
void prev_image(); // hack
void load_current(); // hack

#define snprintf_nowarn(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

Fl_Image *img;  // TODO move inside somebody

#include "MyW.h"
MyW *_w;
#include "XBox.h"

#include <algorithm> // min

XBox *_b2;

static char name[1024];

int filename_path(const char* buf, char *to) { // TODO hack pending adding to FLTK
    const char *p = buf + strlen(buf) - 1;
    for (; *p != '/' && p != buf; --p) // TODO slash is possible '\' under windows
        ;
    if (p == buf) return 0;
    strncpy(to, buf, (p-buf)+1);
    return 1;
}

void MyW::updateLabel() {
    char lbl[1000];
    lbl[0] = 0;
    label(::_b2->getLabel(filename, lbl, sizeof(lbl)));
}

char * XBox::getLabel(char *n, char *buff, int buffsize)
{
    int outzoom = (int)(_zoom * 100.0 + 0.5);
    int w = _img ? _img->w() : 0;
    int h = _img ? _img->h() : 0;
    int depth = _img ? _img->d() : 0;

    char scaletxt[10];
    char * res = humanScale(draw_scale, scaletxt, sizeof(scaletxt)-1);

    if (res == NULL || img == NULL)
        strcpy(buff, "huh?");
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

int XBox::handle(int msg) {

    if (msg == FL_FOCUS || msg == FL_UNFOCUS)
    {
        //printf("box:focus %d\n", msg);
        return 1;
    }

    Fl_Group::handle(msg);

    if (msg == FL_KEYDOWN) {

        //printf("keyboard '%s'\n", Fl::event_text());

        switch (Fl::event_key())
        {
            case 'q':
                exit(0);
                break;

            case 'c':
                draw_check = !draw_check; redraw(); return 1;
                break;

            case 's':
                draw_scale = (ScaleMode)((int)draw_scale + 1);
                if (draw_scale == ScaleMode::MAX)
                    draw_scale = ScaleMode::None;
                redraw();
                return 1;
                break;

            case 'n':
                draw_center = !draw_center;
                if (!draw_center)
                    deltax = deltay = 0;
                redraw();
                return 1;
                break;

//			case 'f':
//				draw_fit = !draw_fit; redraw(); return 1;  // TODO draw_scale is mutually exclusive: need "fit mode"
//				break;

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
                break;

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
                break;

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
                break;

            case FL_Up:
                //printf("Box: Up arrow, state:%d\n", Fl::event_state());
                if (Fl::event_state() & FL_CTRL)
                {
                    deltay += 10; // direction matches FEH
                }
                redraw();
                return 1;
                break;

            case FL_Down:
                if (Fl::event_state() & FL_CTRL)
                {
                    deltay -= 10; // direction matches FEH
                }
                redraw();
                return 1;
                break;

            case FL_Page_Down:
            case ' ':
                next_image();
                return 1;
                break;

            case FL_Page_Up:
            case FL_BackSpace:
                prev_image();
                return 1;
                break;

            case FL_Home:
                current_index = INT_MIN + 1;
                prev_image();
                return 1;
                break;

            case FL_End:
                current_index = INT_MAX - 1;
                next_image();
                return 1;
                break;

            case 't':
                rotation++;
                redraw();
                return 1;
                break;

            case 'z':
                imgtkScale++;
                if (imgtkScale > 6) imgtkScale = 0;
                redraw();
                return 1;
                break;

            case FL_Escape: // escape to NOT close app
                return 1;
                break;

            case 'b':
                ::_w->toggle_border();
                break;

            case FL_PUSH:
//                printf("Box:push\n");
//                ::w->push();
//                return 1;
                break;
            case FL_DRAG:
//                printf("Box:drag\n");
//                ::w->drag();
//                return 1;
                break;
            case FL_RELEASE:
//                printf("Box:release\n");
//                ::w->rel();
//                return 1;
                break;
        }
    }

    if (msg == FL_PUSH) {
        if (Fl::event_button() == FL_RIGHT_MOUSE) {
            do_menu();
            return 1;
        }
    }

    return 0;
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
    }
}

// TODO problem when navigating through file chooser - use folder chooser instead?

int removeFolders(struct dirent *entry) {
    // TODO this is a hack, we're not provided the base path
    const char * out = fl_filename_ext(entry->d_name);
    bool val = out != nullptr && *out != 0 && (out[1] != 0);
    return val;
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

void next_image() {
    current_index = std::min(current_index+1, file_count-1);
    load_current();
}

void prev_image() {
    current_index = std::max(current_index-1, 0); // 2);
    load_current();
}

void load_file(const char *n) {

    load_filelist(n); // TODO background process
    current_index = 0;
    if (!fl_filename_isdir(n))
        current_index = find_file(n);
    load_current();
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
/*
        Fl_Image *img2 = nullptr;
        bool anim = false;

        // 1. try to open as (animated) webp
        Fl_Image *img00 = LoadWebp(n, _b2);
        Fl_Anim_GIF_Image* animgif = nullptr;

        if (img00)
        {
            animgif = dynamic_cast<Fl_Anim_GIF_Image*>(img00);
            if (animgif)
            {
                anim = animgif->is_animated();
                //size *= animgif->frames() * 4; // TODO HACK the gif was d=1 but has been promoted to 32
            }
        }

        if (!img00)
        {
            // 2. try to open as (animated) gif
            animgif = new Fl_Anim_GIF_Image(n, nullptr, Fl_Anim_GIF_Image::Start);
            if (animgif && animgif->valid() && animgif->is_animated()) // TODO can use fail() ?
            {
                animgif->canvas(_b2, Fl_Anim_GIF_Image::Flags::DontResizeCanvas |
                                     Fl_Anim_GIF_Image::Flags::DontSetAsImage);
                img00 = animgif;
            }
            else
            {
                // 2a. try to open as (regular) gif
                delete animgif;
                animgif = nullptr;

                // 3. try to open as Fl_Shared_Image
                img2 = loadFile(n);
                //img2 = Fl_Shared_Image::get(n);
            }
        }
*/
        img = loadFile(n, _b2);

//        if (!img00 && !img2)
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
/*
        if (!img2) // && !animgif)
        {
            //img2 = Fl_Shared_Image::get((Fl_RGB_Image *) img00, true);
            img2 = (Fl_RGB_Image *)img00;
        }

        img = img2;
*/
        _b2->label(nullptr);
        Fl_Anim_GIF_Image* animgif = dynamic_cast<Fl_Anim_GIF_Image*>(img);
        _b2->image(img, animgif);
        _b2->redraw();

        bool anim = animgif != nullptr;

        // this is a hack to force the box to resize which forces the animated image to center in the box & clear background
        // TODO how to make this happen cleanly
        if (anim)
            _w->size(_w->w()+1, _w->h()+1);
    }
dolabel:
    char lbl[1000];
    lbl[0] = 0;
    _w->label(_b2->getLabel(n, lbl, sizeof(lbl)));

    _b2->rotation = 0; // TODO keep rotation?
}

void file_cb(const char *n) {
    if (!strcmp(name,n)) return;
    load_file(n);
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

int dvisual = 0;
int arg(int, char **argv, int &i) {
    if (argv[i][1] == '8') {dvisual = 1; i++; return 1;}
    return 0;
}

int main(int argc, char **argv) {
    int i = 1;

    Fl::scheme("gtk+"); // TODO ability to change - see unittests
    setlocale(LC_ALL, "");    // enable multilanguage errors in file chooser
    makeChecker();

    Fl_Image::RGB_scaling(FL_RGB_SCALING_BILINEAR);

    Fl::args(argc,argv,i,arg);
    if (!dvisual) Fl::visual(FL_RGB);

    _w = new MyW(400,450);
    _b2 = new XBox(5,5,390,440);
    _w->child(_b2);
    _w->resizable(_b2);
    _w->end();

    if (argv[1]) load_file(argv[1]); // TODO add more options

    _w->show(argc,argv);
    return Fl::run();
}
