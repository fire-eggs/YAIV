//
// Created by kevin on 7/6/21.
//
#include <FL/Fl.H>
#include "mediator.h"
#include "XBox.h"

extern XBox *b2;

namespace Mediator {

    class message {
    public:
        int msg;
        int data;
    };

    void mediator(void *msg) {
        if (!msg) return;

        message *msg2 = static_cast<message *>(msg); // TODO how to use dynamic_cast?
        if (!msg2)
            printf("Invalid msg\n");
        else {
            if (msg2->msg == MSG_TB && msg2->data == ACT_EXIT)
                exit(0);

            //printf("Msg: %d - %d\n", msg2->msg, msg2->data);
            if (msg2->msg == MSG_KEY) {
                b2->key(msg2->data); // TODO key handling right here : lookup & send
            }
            if (msg2->msg == MSG_TB) {
                b2->action(msg2->data);
            }
            delete msg2;
        }
    }

    void send_message(int msg, int data) {

        // TODO primarily intended for thread activity like "load image" -> sends "image loaded" message

        message *msg2 = new message();
        msg2->msg = msg;
        msg2->data = data;
        Fl::awake(mediator, msg2);
    }

    void handle_key() {

        // TODO why does this use send_message()? when would keys happen on the non-GUI thread?

        int key = Fl::event_key();
        int keyStateShift = Fl::event_state(FL_SHIFT);
        int keyStateAlt = Fl::event_state(FL_ALT);
        int keyStateCtrl = Fl::event_state(FL_CTRL);
        int keyStateCmd = 0;
#ifdef __APPLE__
        int keyStateCmd = Fl::event_state(FL_COMMAND);
#endif
        send_message(MSGS::MSG_KEY, key | keyStateShift | keyStateAlt | keyStateCtrl | keyStateCmd);
    }
}