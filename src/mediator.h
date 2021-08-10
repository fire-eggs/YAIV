//
// Created by kevin on 7/6/21.
//

#ifndef YAIV_MEDIATOR_H
#define YAIV_MEDIATOR_H

#include "prefs.h"

namespace Mediator {

    enum MSGS {
        MSG_KEY = 0,
        MSG_TB = 1,
        MSG_NEWFILE=2,
    };

    enum ACTIONS {
        ACT_INVALID = -1,
        ACT_PREV = 0,
        ACT_NEXT = 1,
        ACT_ROTR = 2,
        ACT_CHK  = 3,
        ACT_GOTO = 4,
        ACT_SLID = 5,
        ACT_ZMI  = 6,
        ACT_ZMO  = 7,
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
    };
    void send_message(int msg, int data);

    void handle_key();

    void danbooru(Prefs *);

    void setTheme(int);

}

#endif //YAIV_MEDIATOR_H
