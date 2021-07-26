//
// Created by kevin on 7/6/21.
//
#include <FL/Fl.H>
#include "mediator.h"
#include "XBox.h"
#include "toolbar/toolgrp.h"
#include "toolbar/buttonBar.h"

extern XBox *b2;
extern dockgroup* dock;
extern ButtonBar* tb;

#ifdef DANBOORU
#include "danbooru.h"
toolgrp *_danbooru;
#endif

namespace Mediator {

    class message {
    public:
        int msg;
        int data;
    };

    void mediator(void *msg) {
        if (!msg) return;

        message *msg2 = static_cast<message *>(msg); // TODO how to use dynamic_cast?
        if (!msg2) {
            printf("Invalid msg\n");
            return;
        }

        switch (msg2->msg)
        {
            case MSG_TB:
                if (msg2->data == ACT_EXIT)
                    exit(0);
                if (msg2->data == ACT_MENU)
                    // TODO full menu, not popup menu
                    b2->do_menu(Fl::event_x(),Fl::event_y(), false);
// TODO refactor state management: 1. mediator holds state; 2. mediator inits state; 3. mediator sends state update to targets; 4. keyboard handling not in xbox
                if (msg2->data == ACT_CHK)
                    tb->setState(ACT_CHK, !b2->getCheck());
                if (msg2->data == ACT_SLID)
                    tb->setState(ACT_SLID, !b2->inSlide());
                b2->action(msg2->data);
                break;
            case MSG_NEWFILE:
#ifdef DANBOORU
                // update for new file
                if (_danbooru)
                    update_danbooru(b2->currentFilename());
#endif
                // TODO if data is negative, no file
                // TODO update b2 image
                // TODO update toolbar state

                break;
            case MSG_KEY:
                b2->key(msg2->data); // TODO do key handling in mediator: lookup & send
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

#ifdef DANBOORU
    // TODO should this be a message? action?
    void danbooru(Prefs *prefs) {
        if (!_danbooru || !dock->contains(_danbooru))
            _danbooru = new toolgrp(nullptr, 1, 0, 0, 200, 500);
        view_danbooru(prefs, _danbooru->in_group());
        update_danbooru(b2->currentFilename());
    }
#endif
}
