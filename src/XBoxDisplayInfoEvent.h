//
// Created by kevin on 6/7/21.
//

#ifndef YAIV_XBOXDISPLAYINFOEVENT_H
#define YAIV_XBOXDISPLAYINFOEVENT_H

#include "Fl_TransBox.h"

class XBoxDisplayInfoEvent
{
public:
    virtual void OnDisplayInfo( const char* info ) = 0;
    virtual void OnActivate(bool activate) = 0;
};

// TODO goes in separate header?

class XBoxDspInfoEI : public XBoxDisplayInfoEvent
{
private:
    Fl_TransBox *_infoBox {};

public:
    XBoxDspInfoEI(Fl_TransBox* dest) { _infoBox = dest; }

    void OnActivate(bool show) override
    {
        if (show) _infoBox->show();
        else _infoBox->hide();
    }

    void OnDisplayInfo( const char* info ) override
    {
        if (!_infoBox || !info)
            return;

        float _maxf = (float)(_infoBox->h()-10);
        // let calculate label size
        // ratio ..
        float whrf = ( (float)_infoBox->w() / (float)_infoBox->h() ) / 20.f;

        if ( whrf > 1.f ) whrf = 1.f;
        else if ( whrf <= 0.f ) whrf = 0.1f;

        whrf *= _maxf;
        // if to small? make it limited.
        if ( whrf < 10.f ) whrf = 10.f;

        // TODO label font, size from prefs?
        _infoBox->labelsize( (int)whrf );
        _infoBox->copy_label( info );
        _infoBox->redraw();
    }
};

#endif //YAIV_XBOXDISPLAYINFOEVENT_H