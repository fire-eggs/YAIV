//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_MYW_H
#define CLION_TEST2_MYW_H

class MyW : public Fl_Double_Window
{
private:
    bool _border;
    int _xoff;
    int _yoff;
    Fl_Widget *_child;

public:
    MyW(int a, int b) : Fl_Double_Window(a, b), _border(1), _xoff(0), _yoff(0) {}

    void child(Fl_Widget *c) { _child =c; }

    char filename[FL_PATH_MAX];

    void updateLabel();

    void toggle_border() { _border = !_border; border(_border); }

    void push()
    {
        _xoff = x() - Fl::event_x_root();
        _yoff = y() - Fl::event_y_root();
    }
    void drag()
    {
        position(_xoff + Fl::event_x_root(), _yoff+Fl::event_y_root());
        redraw();
    }
    void rel()
    {
        // show();
    }

    int handle(int e)
    {
        int ret;
        ret = Fl_Double_Window::handle(e);
        switch (e)
        {
            case FL_PUSH: push(); ret=1; break;

            case FL_DRAG: drag(); ret=1; break;

            case FL_RELEASE: rel(); ret=1; break;

            case FL_FOCUS:
                //printf("win:focus\n");
                //ret = 0;
                _child->take_focus();
                break;
            case FL_UNFOCUS:
                //printf("win:unfocus\n");
                ret = 1;
//          _child->take_focus();
                break;

            case FL_KEYDOWN:
                //printf("Win: keydown, state:%d\n", Fl::event_state());
                ret = 0;
                break;

            default: ret=0; break;
        }
        return ret;
    }
};

#endif //CLION_TEST2_MYW_H
