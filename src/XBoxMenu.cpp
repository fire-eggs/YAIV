#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif

//
// Created by kevin on 6/21/21.
//

#include <locale>

#include "XBox.h"
#include "FL/fl_ask.H"
#include "FL/Fl_File_Chooser.H"
#include "mediator.h"
#include "menuids.h"
#include "filelist.h"

extern filelist* box_filelist; // TODO member

extern void showOptionsDlg(); // TODO


// Sorting for fl_file_chooser which is UTF-8 aware. I.e. ".." sorts *before* Kanji.
// NOTE: may not be numeric aware as is fl_numericsort.
std::locale _loc;
const std::collate<char>& coll2 = std::use_facet<std::collate<char>>(_loc);

int blah(struct dirent **A, struct dirent **B)
{
  const char* a = (*A)->d_name;
  const char* b = (*B)->d_name;
  return coll2.compare(a, a + strlen(a), b, b + strlen(b)) > 0;
}

void XBox::load_request() {

    // 20220530 start with the last path used
    char** mru = _mru->getAll(); // TODO return a single path
    auto last = mru[0];
    
    //Fl_File_Chooser::sort = fl_numericsort;
    Fl_File_Chooser::sort = blah;
    const char *fname =
            fl_file_chooser("Image file?","*.{bm,bmp,gif,jpg,jpeg,apng,png,webp"
                                          #ifdef FLTK_USE_SVG
                                          ",svg"
                                          #ifdef HAVE_LIBZ
                                          ",svgz"
                                          #endif // HAVE_LIBZ
                                          #endif // FLTK_USE_SVG
                                          "}", last);

    if (!fname)
        return;
    load_file(fname);
}

void XBox::goto_request() {

    if (!box_filelist || box_filelist->fileCount() == 0)
        return; // no images
        
    int dex = box_filelist->currentIndex() + 1;
    char def[256];
    sprintf(def, "%d", dex);
    const char* res = fl_input("Goto image:", def);
    if (res)
    {
        try
        {
            int val = std::stoi(res);
            box_filelist->setCurrent(val - 1);
            load_current();
            //dynamic_cast<XBox *>(window_p)->load_current(); // TODO hack
        }
        catch (std::exception& e)
        {
        }
    }
}

struct menucall {XBox *who; int menu;};
static void xbox_menucb(Fl_Widget *window_p, void *userdata) {

    struct menucall *mc = static_cast<struct menucall *>(userdata);
    if (mc == nullptr) return;
    mc->who->MenuCB(window_p, mc->menu);
}

void XBox::MenuCB(Fl_Widget *window_p, int menuid) {

    size_t ndata = menuid;

    switch( ndata )
    {
        case MI_LOAD: // TODO ACT_OPEN
            load_request();
            this->take_focus();
            break;

        case MI_COPYPATH:
            {
            if (!box_filelist || box_filelist->fileCount() == 0)
                break; // no images loaded
            const char *fullpath = box_filelist->getCurrentFilePath();
            if (!fullpath)
                break;
            Fl::copy(fullpath, (int)strlen(fullpath), 1);
            }
            break;

        case MI_GOTO: // TODO ACT_GOTO
            goto_request();
            break;

        case MI_OPTIONS:
            showOptionsDlg();
            _prefs = new Prefs(); // gets new values. TODO memory leak
            break;

#ifdef DANBOORU
        case MI_DANBOORU:
            Mediator::danbooru(_prefs);
            break;
#endif
            
#ifdef METADATA
        case MI_METADATA:
            Mediator::metadata(_prefs);
            break;
#endif

        case MI_FAV0: case MI_FAV1: case MI_FAV2:
        case MI_FAV3: case MI_FAV4: case MI_FAV5:
        case MI_FAV6: case MI_FAV7: case MI_FAV8: case MI_FAV9:
        {
            size_t path = ndata - MI_FAV0;
            char** mru = _mru->getAll(); // TODO return a single path
            load_file(mru[path]);
        }
            break;

        case MI_THEME_BLUE:
        case MI_THEME_CLASSIC:
        case MI_THEME_DARK:
        case MI_THEME_GREYBIRD:
        case MI_THEME_HIGHCONTRAST:
        case MI_THEME_NATIVE:
        case MI_THEME_OCEAN:
        case MI_THEME_OLIVE:
        case MI_THEME_ROSEGOLD:
        case MI_THEME_TAN:
            Mediator::setTheme(ndata);
            break;

        default:
            break;
    }
}

void XBox::do_menu(int xloc, int yloc, bool title) {

    // 1. find the submenu in the "master" menu
    int submenuNdx;
    for (submenuNdx = 0; submenuNdx < right_click_menu->size(); submenuNdx++)
    {
        if (!right_click_menu[submenuNdx].text)
            continue;
        if (strcmp(right_click_menu[submenuNdx].text, "Last Used") != 0)
            continue;
        break;
    }

    int numfavs = _mru->getCount();

    // create a new menu
    unsigned long newCount = right_click_menu->size() + numfavs;
    Fl_Menu_Item* dyn_menu = new Fl_Menu_Item[newCount];

    // make sure the rest of the allocated menu is clear
    for (unsigned int j = 0; j < newCount; j++)
        memset(&(dyn_menu[j]), 0, sizeof(Fl_Menu_Item));

    std::vector<menucall *> *totoss = new std::vector<menucall*>();

    // initialize it with the static menu contents
    for (size_t j = 0; j <= (size_t)submenuNdx; j++)
    {
        dyn_menu[j] = right_click_menu[j];
        size_t menuparam = (size_t)dyn_menu[j].user_data();
        //size_t menuparam = (size_t)MI_LOAD + j; // TODO use actual value, not menu index?
        menucall *hold = new menucall;
        hold->who = this;
        hold->menu = static_cast<int>(menuparam);
        dyn_menu[j].callback(xbox_menucb, (void*)hold );
        totoss->push_back(hold);
    }

    // TODO if numfavs == 0 disable the "last used"

    char** favs = _mru->getAll();
    for (long j = 0; j < numfavs; j++)
    {
        dyn_menu[submenuNdx + 1 + j].label(favs[j]);
        menucall *hold = new menucall;
        hold->who = this;
        hold->menu = MI_FAV0 + j;
        dyn_menu[submenuNdx + 1 + j].callback(xbox_menucb, (void*)hold);
        totoss->push_back(hold);
    }

    // show the menu
    const Fl_Menu_Item *m = dyn_menu->popup(xloc, yloc, title ? "YAIV" : nullptr,
                                            nullptr, nullptr);
    if (m && m->callback())
        m->do_callback(this, m->user_data());

    // Memory cleanup
    for (long unsigned int i=0; i<totoss->size();i++)
        delete totoss->at(i);
    delete totoss;
    delete [] dyn_menu;
}

void XBox::hideCurrent() {

    if (!box_filelist || box_filelist->fileCount() == 0) // TODO initialize box_filelist
        return; // no images
    
    char buff[FL_PATH_MAX<<2];
    char *currfilename = box_filelist->currentFilename();
    sprintf(buff, "Are you sure you want to hide '%s'?", currfilename);
    int res = fl_choice(buff, "No", "Yes", 0L, "continue");
    if (!res) return;

    bool res2 = box_filelist->hide();
    if (!res2)
        fl_alert("Failure to open 'hidden' file.");
    load_current();
}

void XBox::favCurrent() {

    if (!box_filelist || box_filelist->fileCount() == 0)
        return; // no images

    bool res2 = box_filelist->addToFavs();
    if (!res2)
        fl_alert("Failure to open 'favs' file.");
}

#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
