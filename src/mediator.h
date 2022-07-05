//
// Created by kevin on 7/6/21.
//

#ifndef YAIV_MEDIATOR_H
#define YAIV_MEDIATOR_H

#include "prefs.h"
#include "XBox.h"
class ButtonBar;

namespace Mediator {

    enum MSGS {
        MSG_KEY = 0,
        MSG_TB = 1,
        MSG_NEWFILE=2,
        MSG_REALUPDATE,
        MSG_VIEW,
    };

    enum ACTIONS {
        ACT_INVALID = -1,
        ACT_PREV = 0,
        ACT_NEXT = 1,
        ACT_ROTR = 2,  // rotate right
        ACT_CHK  = 3,  // checkerboard
        ACT_GOTO = 4,
        ACT_SLID = 5,  // slideshow
        ACT_ZMI  = 6,  // zoom in
        ACT_ZMO  = 7,  // zoom out
        ACT_OPEN = 8,
        ACT_EXIT = 9,
        ACT_MENU = 10,
        ACT_SCALE = 11,
        ACT_SCALE_NONE = 12,
        ACT_SCALE_AUTO = 13,
        ACT_SCALE_FIT = 14,
        ACT_SCALE_WIDE = 15,
        ACT_SCALE_HIGH = 16,

        ACT_NOPREV,
        ACT_NONEXT,
        ACT_ISPREV,
        ACT_ISNEXT,
        
        ACT_HIDE,
        ACT_FAV,
        
        ACT_RANDOM,
        ACT_OVERLAY,
        ACT_MINIMAP,
        ACT_BORDER,
        ACT_DITHER,
        ACT_MOUSEPAN,
        
        ACT_SCROLLUP,
        ACT_SCROLLDOWN,
        ACT_SCROLLLEFT,
        ACT_SCROLLRIGHT,
        
        ACT_HOME,
        ACT_END, 
        
        ACT_METADATA,
        ACT_DANBOORU,
    };
        
    void send_message(int msg, int data);

    void handle_key();

    void danbooru(Prefs *);

    void metadata(Prefs *);
    
    void setTheme(int);

    bool lookupKey(int key, int ctrl, int& msg, int& act);

    void initialize(XBox *, Prefs *, ButtonBar *);
}

#endif //YAIV_MEDIATOR_H
