//
// Created by kevin on 6/15/21.
//

#ifndef YAIV_TKSCALEMODE_H
#define YAIV_TKSCALEMODE_H

// NOTE: these must match the value and sequence in fl_imgtk.h
#define MODEStk(XX) \
    XX(None),  \
    XX(Bilinear), \
    XX(Bicubic), \
    XX(Lanczos),   \
    XX(Bspline),   \
    XX(Catmull)

#define XX(e) e
enum ZScaleMode { MODEStk(XX), ZScaleModeMAX };
#undef XX

ZScaleMode nameToZScaleMode( std::string s );
std::string zScaleModeToName(ZScaleMode mode);

char *humanZScale(ZScaleMode val, char *buff, int buffsize);

#endif //YAIV_TKSCALEMODE_H