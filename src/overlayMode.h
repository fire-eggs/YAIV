//
// Created by kevin on 6/15/21.
//

#ifndef YAIV_OVERLAYMODE_H
#define YAIV_OVERLAYMODE_H

//enum OverlayMode { OM_None=0, Text, TBox, OM_MAX };

#define OverModes(X) \
X(OverlayNone),       \
X(OverlayText),       \
X(OverlayBox)

#define X(e) e
enum OverlayMode { OverModes(X), OverlayModeMAX };
#undef X

OverlayMode nameToOverlayMode( const std::string& s );
std::string overlayModeToName(OverlayMode mode);
OverlayMode nextOverlay(OverlayMode val);

#endif //YAIV_OVERLAYMODE_H
