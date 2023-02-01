//
// Created by kevin on 7/6/21.
//
#include <FL/Fl.H>
#include "mediator.h"
#include "toolbar/buttonBar.h"
#include "filelist.h"

#ifdef DANBOORU
#include "danbooru.h"
toolgrp* _danbooru;
#endif

#include "metadata.h"
toolgrp* _metadata;

#include "menuids.h"
#include "themes.h"
#include <assert.h>

// TODO make a class, these as members
extern filelist* box_filelist; // TODO hack
static Prefs *_prefs;          // TODO hack
static XBox *_viewer;
static ButtonBar *_toolbar;

namespace Mediator {

    void initialize(XBox *viewer, Prefs *prefs, ButtonBar *toolbar)
    {
        _prefs = prefs;
        _viewer = viewer;
        _toolbar = toolbar;
    }

    
    class message {
    public:
        int msg;
        int data;
    };

    void toolbarMsg(Mediator::ACTIONS val)
    {
// TODO refactor state management?: 1. mediator holds state; 2. mediator inits state; 3. mediator sends state update to targets

        // 20221109 perform action first, update toolbar state from result of action [e.g. slideshow status]
        _viewer->action(val); // TODO with this, MSG_VIEW, MSG_TB distinction not necessary?
              
        switch (val)
        {
            case ACT_EXIT:
                exit(0);
            case ACT_MENU:
                // TODO show the full menu, not the popup menu
                _viewer->do_menu(Fl::event_x(),Fl::event_y(), false);
                break;
            case ACT_CHK:
                _toolbar->setState(ACT_CHK, _viewer->getCheck());
                break;
            case ACT_SLID:
                {
                bool inslide = _viewer->inSlide();
                _toolbar->setState(ACT_SLID, inslide);
                }
                break;
            case ACT_SCALE_HIGH:
            case ACT_SCALE_FIT:
            case ACT_SCALE_AUTO:
            case ACT_SCALE_NONE:
            case ACT_SCALE_WIDE:
                // TODO tell the toolbar to set the scale image
                _toolbar->setScaleImage(val);
                break;
            case ACT_NONEXT:
            case ACT_NOPREV:
                _toolbar->deactivate(val);
                break;
            case ACT_ISNEXT:
            case ACT_ISPREV:
                _toolbar->activate(val);
                break;

#ifdef DANBOORU                
            case ACT_DANBOORU:
                if (!box_filelist || !box_filelist->any())
                    break;

                // TODO was this supposed to be tied to a modifier?
                //if (Fl::event_state() & CTRL_P_KEY)
                {
                    Mediator::danbooru(_prefs);
                }
                break;
#endif
#ifdef METADATA                
            case ACT_METADATA:
                Mediator::metadata(_prefs);
                break;
#endif                
            case ACT_HIDE:
                _viewer->hideCurrent(); // TODO better approach?
                break;
            case ACT_FAV:
                _viewer->favCurrent(); // TODO better approach?
                break;
                
            default:
                break;
        }
       
    }

    void mediator(void *msg) 
    {
        if (!msg) return;

        message *msg2 = static_cast<message *>(msg); // TODO how to use dynamic_cast?
        if (!msg2) {
            printf("Invalid msg\n");
            return;
        }
        
        switch (msg2->msg)
        {
            case MSG_TB:
            case MSG_VIEW: // TODO distinction no longer necessary?
                toolbarMsg(static_cast<ACTIONS>(msg2->data));
                break;

            case MSG_NEWFILE:
#ifdef DANBOORU
                // update for new file
                if (_danbooru)
                    update_danbooru(box_filelist->currentFilename());
#endif
                // TODO if data is negative, no file
                // TODO update _viewer image
                // TODO update toolbar state
#ifdef METADATA                
                if (_metadata)
                    update_metadata(box_filelist->getCurrentFilePath());
#endif
                break;
            case MSG_REALUPDATE:
                // update toolbar
                // update titlebar
                if (box_filelist->realCount() < 2)
                    _viewer->load_current();
                else
                {
                    _viewer->updateLabel();
                    _viewer->updateDisplay();
                }
                break;
        }
        delete msg2;
    }

    void send_message(int msg, int data) 
    {

        // TODO primarily intended for thread activity like "load image" -> sends "image loaded" message

        message *msg2 = new message();
        msg2->msg = msg;
        msg2->data = data;
        Fl::awake(mediator, msg2);
    }

    void handle_key() 
    {

        int key = Fl::event_key();
        int keyStateShift = Fl::event_state(FL_SHIFT);
        int keyStateAlt = Fl::event_state(FL_ALT);
        int keyStateCtrl = Fl::event_state(FL_CTRL);
        int keyStateCmd = 0;
#ifdef __APPLE__
        int keyStateCmd = Fl::event_state(FL_COMMAND);
#endif
    
        int msg;
        int act;
        bool res = lookupKey(key, keyStateCtrl, msg, act);
        if (res)
            send_message(msg, act); // TODO invoke directly
    }

#ifdef DANBOORU
Fl_Widget_Tracker *db_track = nullptr;

    // TODO should this be a message? action?
    void danbooru(Prefs *prefs) {

        // shortcut hides window if it's displayed
        if (_danbooru && !_danbooru->docked() && _danbooru->shown())
        {
            _danbooru->cb_dismiss(nullptr, _danbooru);
            _danbooru = nullptr;
            return;
        }
                
        if (!db_track || db_track->deleted()) {
            delete db_track;
            
            int dx,dy,dw,dh;
            prefs->getWinRect("danbooru", dx, dy, dw, dh, 0, 0, 200, 500);
            
            //_danbooru = new toolgrp(nullptr, 1, 0, 0, 200, 500, nullptr, "yaivDanbooru");
            _danbooru = new toolgrp(nullptr, 1, dx, dy, dw, dh, nullptr, "yaivDanbooru");
            _danbooru->setPrefs(prefs, "danbooru");
            
            db_track = new Fl_Widget_Tracker(_danbooru);
        }
        view_danbooru(prefs, _danbooru->in_group());
        if (box_filelist)
            update_danbooru(box_filelist->currentFilename());
    }
#endif

#ifdef METADATA
    Fl_Widget_Tracker *md_track = nullptr;

    void metadata(Prefs *prefs) {
        
        // shortcut hides window if it's displayed
        if (_metadata && !_metadata->docked() && _metadata->shown())
        {
            _metadata->cb_dismiss(nullptr, _metadata);
            _metadata = nullptr;
            return;
        }
        
        if (!md_track || md_track->deleted()) {
            delete md_track;
            
            int dx,dy,dw,dh;
            prefs->getWinRect("metaview", dx, dy, dw, dh, 0, 0, 200, 500);
            
            //_metadata = new toolgrp(nullptr, 1, 0, 0, 200, 500, nullptr, "yaivMetadata");
            _metadata = new toolgrp(nullptr, 1, dx, dy, dw, dh, nullptr, "yaivDanbooru");
            _metadata->setPrefs(prefs, "metaview");
            
            md_track = new Fl_Widget_Tracker(_metadata);
        }
        init_metadata();
        create_metadata(prefs, _metadata->in_group());
        if (box_filelist)
            update_metadata(box_filelist->getCurrentFilePath());
    }
#endif

void setTheme(int menuval) {
    bool isdark = false;
    switch (menuval) {
        case MI_THEME_BLUE:
            OS::use_blue_theme();
            break;
        case MI_THEME_CLASSIC:
            OS::use_classic_theme();
            break;
        case MI_THEME_DARK:
            OS::use_dark_theme();
            isdark = true;
            break;
        case MI_THEME_GREYBIRD:
            OS::use_greybird_theme();
            break;
        case MI_THEME_HIGHCONTRAST:
            OS::use_high_contrast_theme();
            isdark = true;
            break;
        case MI_THEME_NATIVE:
            OS::use_native_theme();
            break;
        case MI_THEME_OCEAN:
            OS::use_ocean_theme();
            break;
        case MI_THEME_OLIVE:
            OS::use_olive_theme();
            break;
        case MI_THEME_ROSEGOLD:
            OS::use_rose_gold_theme();
            break;
        case MI_THEME_TAN:
            TanColormap_FLTKSUBS();
            //irushTan();
            break;
    }

    _toolbar->updateColor(isdark);
}
    /*
        static void use_classic_theme(void);
        static void use_aero_theme(void);
        static void use_metro_theme(void);
        static void use_aqua_theme(void);
        static void use_greybird_theme(void);
        static void use_ocean_theme(void);
        static void use_blue_theme(void);
        static void use_olive_theme(void);
        static void use_rose_gold_theme(void);
        static void use_dark_theme(void);
        static void use_high_contrast_theme(void);
     */


    
    struct KeyAction
    {
        int key;
        bool shift;
        bool ctrl;
        bool alt;
        enum MSGS    msg;             // TODO eliminate the 'msg' part. e.g. ask each consumer, if returns false, ask next
        enum ACTIONS act;
    };

    static struct KeyAction keymap[] = 
    {
        {'b',false,false,false, MSG_VIEW, ACT_BORDER},
        {'c',false,false,false, MSG_TB, ACT_CHK},
        {'d',false,false,false, MSG_TB, ACT_DANBOORU},
        {'e',false,false,false, MSG_TB, ACT_METADATA},
        {'g',false,true, false, MSG_TB, ACT_GOTO},
        {'h',false,false,false, MSG_VIEW, ACT_HIDE},
        {'m',false,false,false, MSG_VIEW, ACT_MINIMAP},
        {'o',false,false,false, MSG_VIEW, ACT_OVERLAY},
        {'o',false,true, false, MSG_TB, ACT_OPEN},
        {'p',false,false,false, MSG_VIEW, ACT_MOUSEPAN},
        {'q',false,false,false, MSG_TB, ACT_EXIT},
        {'r',false,false,false, MSG_VIEW, ACT_RANDOM}, // TODO should be toolbar msg?
        {'s',false,false,false, MSG_TB, ACT_SCALE},
        {'t',false,false,false, MSG_VIEW, ACT_ROTR},
        {'v',false,false,false, MSG_TB, ACT_FAV},
        {'w',false,false,false, MSG_TB, ACT_SLID},
        {'z',false,false,false, MSG_VIEW, ACT_DITHER},
        {' ',false,false,false, MSG_VIEW, ACT_NEXT},
        
        {FL_Up,false,false,false, MSG_VIEW, ACT_ZMI},
        {FL_Up,false,true,false,  MSG_VIEW, ACT_SCROLLUP},
        {FL_Down,false,false,false, MSG_VIEW, ACT_ZMO},
        {FL_Down,false,true,false,  MSG_VIEW, ACT_SCROLLDOWN},
        {FL_Right,false,false,false, MSG_VIEW, ACT_NEXT},
        {FL_Right,false,true,false,  MSG_VIEW, ACT_SCROLLRIGHT},
        {FL_Left,false,false,false, MSG_VIEW, ACT_PREV},
        {FL_Left,false,true,false,  MSG_VIEW, ACT_SCROLLLEFT},

        {FL_Page_Down,false,false,false, MSG_VIEW, ACT_NEXT},
        {FL_Page_Up,false,false,false, MSG_VIEW, ACT_PREV},
        {FL_Home,false,false,false, MSG_VIEW, ACT_HOME},
        {FL_End,false,false,false, MSG_VIEW, ACT_END},
 
        {FL_BackSpace,false,false,false, MSG_VIEW, ACT_PREV},
    };
    
    bool lookupKey(int key, int ctrl, int& msg, int& act)
    {
        int len = sizeof(keymap) / sizeof(struct KeyAction);
        for (int i = 0; i < len; i++)
        {
            if (key != keymap[i].key)
                continue;
            if ((bool)ctrl != keymap[i].ctrl)
                continue;
            
            msg = keymap[i].msg;
            act = keymap[i].act;
            return true;
        }
        return false;
    }
}
