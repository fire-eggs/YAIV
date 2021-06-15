//
// Created by kevin on 6/15/21.
//

#ifndef YAIV_SCALEMODE_H
#define YAIV_SCALEMODE_H

#include <string>
#include <algorithm>

//enum ScaleMode { None=0, Auto, Fit, Wide, High, MAX };
// 100%; Scale if larger; Scale to window; Scale to width; Scale to height

#define MODES(X)  \
    X(Noscale), \
    X(Auto), \
    X(Fit),  \
    X(Wide), \
    X(High)

#define X(e) e
enum ScaleMode { MODES(X), ScaleModeMAX };
#undef X

ScaleMode nameToScaleMode( std::string s );
std::string scaleModeToName(ScaleMode mode);
char *humanScale(ScaleMode val, char *buff, int buffsize);

#endif //YAIV_SCALEMODE_H
