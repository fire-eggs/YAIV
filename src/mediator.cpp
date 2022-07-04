//
// Created by kevin on 7/6/21.
//
#include <FL/Fl.H>
#include "mediator.h"
#include "XBox.h"
#include "toolbar/toolgrp.h"
#include "toolbar/buttonBar.h"
#include "filelist.h"

extern XBox *b2;
extern dockgroup* dock;
extern ButtonBar* tb;

#ifdef DANBOORU
#include "danbooru.h"
toolgrp* _danbooru;
#endif

#include "metadata.h"
toolgrp* _metadata;

#include "menuids.h"
#include "themes.h"

extern filelist* box_filelist; // TODO hack

namespace Mediator {

    class message {
    public:
        int msg;
        int data;
    };

    void toolbarMsg(Mediator::ACTIONS val)
    {
        switch (val)
        {
            case ACT_EXIT:
                exit(0);
            case ACT_MENU:
                // TODO show the full menu, not the popup menu
                b2->do_menu(Fl::event_x(),Fl::event_y(), false);
                break;
// TODO refactor state management: 1. mediator holds state; 2. mediator inits state; 3. mediator sends state update to targets; 4. keyboard handling not in xbox
            case ACT_CHK:
                tb->setState(ACT_CHK, !b2->getCheck());
                break;
            case ACT_SLID:
                tb->setState(ACT_SLID, !b2->inSlide()); // TODO should not happen if changing slideshow is invalid (e.g. no images)
                break;
            case ACT_SCALE_HIGH:
            case ACT_SCALE_FIT:
            case ACT_SCALE_AUTO:
            case ACT_SCALE_NONE:
            case ACT_SCALE_WIDE:
                // TODO tell the toolbar to set the scale image
                tb->setScaleImage(val);
                break;
            case ACT_NONEXT:
            case ACT_NOPREV:
                tb->deactivate(val);
                break;
            case ACT_ISNEXT:
            case ACT_ISPREV:
                tb->activate(val);
                break;
            default:
                break;
        }
        b2->action(val);
    }

    void mediator(void *msg) {
        if (!msg) return;

        message *msg2 = static_cast<message *>(msg); // TODO how to use dynamic_cast?
        if (!msg2) {
            printf("Invalid msg\n");
            return;
        }

        if (msg2->msg != MSG_REALUPDATE)
            printf("med: %d, %d\n", msg2->msg, msg2->data);
        
        switch (msg2->msg)
        {
            case MSG_TB:
                toolbarMsg(static_cast<ACTIONS>(msg2->data));
                break;

            case MSG_NEWFILE:
#ifdef DANBOORU
                // update for new file
                if (_danbooru)
                    update_danbooru(box_filelist->currentFilename());
#endif
                // TODO if data is negative, no file
                // TODO update b2 image
                // TODO update toolbar state
                
                if (_metadata)
                    update_metadata(box_filelist->getCurrentFilePath());

                break;
            case MSG_KEY:
                b2->key(msg2->data); // TODO do key handling in mediator: lookup & send
                break;
            case MSG_REALUPDATE:
                // update toolbar
                // update titlebar
                if (box_filelist->realCount() < 2)
                    b2->load_current();
                else
                    b2->updateLabel();
                break;
            case MSG_VIEW:
                b2->action(static_cast<ACTIONS>(msg2->data));
                break;
        }
        delete msg2;
    }

    void send_message(int msg, int data) {

        // TODO primarily intended for thread activity like "load image" -> sends "image loaded" message

        message *msg2 = new message();
        msg2->msg = msg;
        msg2->data = data;
        Fl::awake(mediator, msg2);
    }

    void handle_key() {

        //printf("med:handle_key\n");
        // TODO why does this use send_message()? when would keys happen on the non-GUI thread?

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
        else
            b2->key(key | keyStateShift | keyStateAlt | keyStateCtrl | keyStateCmd); // TODO do key handling in mediator: lookup & send

        
        //send_message(MSGS::MSG_KEY, key | keyStateShift | keyStateAlt | keyStateCtrl | keyStateCmd);
    }

#ifdef DANBOORU
Fl_Widget_Tracker *db_track = nullptr;

    // TODO should this be a message? action?
    void danbooru(Prefs *prefs) {

        if (!db_track || db_track->deleted()) {
            delete db_track;
            _danbooru = new toolgrp(nullptr, 1, 0, 0, 200, 500, nullptr, "yaivDanbooru");
            db_track = new Fl_Widget_Tracker(_danbooru);
        }
        view_danbooru(prefs, _danbooru->in_group());
        if (box_filelist)
            update_danbooru(box_filelist->currentFilename());
    }
#endif

    Fl_Widget_Tracker *md_track = nullptr;

    void metadata(Prefs *prefs) {
        if (!md_track || md_track->deleted()) {
            delete md_track;
            _metadata = new toolgrp(nullptr, 1, 0, 0, 200, 500, nullptr, "yaivMetadata");
            md_track = new Fl_Widget_Tracker(_metadata);
        }
        init_metadata();
        create_metadata(prefs, _metadata->in_group());
        if (box_filelist)
            update_metadata(box_filelist->getCurrentFilePath());
    }

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

    tb->updateColor(isdark);
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
        enum MSGS    msg;             // TODO eliminate the 'msg' part? e.g. ask each consumer, if returns false, ask next
        enum ACTIONS act;
    };

    struct KeyAction keymap[] = 
    {
        {'c',false,false,false, MSG_TB, ACT_CHK},
        {'m',false,false,false, MSG_VIEW, ACT_MINIMAP},
        {'o',false,false,false, MSG_VIEW, ACT_OVERLAY},
        {'o',false,true, false, MSG_TB, ACT_OPEN},
        {'q',false,false,false, MSG_TB, ACT_EXIT},
        {'r',false,false,false, MSG_VIEW, ACT_RANDOM}, // TODO should be toolbar msg?
        {'s',false,false,false, MSG_TB, ACT_SCALE},
        {'t',false,false,false, MSG_VIEW, ACT_ROTR},
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

