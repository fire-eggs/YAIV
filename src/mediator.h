//
// Created by kevin on 7/6/21.
//

#ifndef YAIV_MEDIATOR_H
#define YAIV_MEDIATOR_H

namespace Mediator {

    enum MSGS {
        MSG_KEY = 0,
        MSG_TB = 1,
    };

    enum ACTIONS {
        ACT_PREV = 0,
        ACT_NEXT = 1,
        ACT_ROTR = 2,
        ACT_CHK  = 3,
        ACT_GOTO = 4,
        ACT_SLID = 5,
        ACT_ZMI  = 6,
        ACT_ZMO  = 7,
        ACT_OPEN = 8,
    };
    void send_message(int msg, int data);

    void handle_key();
}

#endif //YAIV_MEDIATOR_H
