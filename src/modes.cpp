#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-auto"
#endif
//
// Created by kevin on 6/15/21.
//
#include "ScaleMode.h"
#include "tkScaleMode.h"
#include "overlayMode.h"

#include <cstring>

#define X(s) #s
std::string ScaleModeNames[] = {MODES(X)};
#undef X

// TODO can these be template-tized? or macro-tized?

ScaleMode nameToScaleMode( const std::string& s )
{
    auto res = std::find( ScaleModeNames, ScaleModeNames+static_cast<int>(ScaleMode::LAST), s);
    ScaleMode val = ScaleMode( res - ScaleModeNames );
    return val == ScaleMode::LAST ? ScaleMode::Noscale : val;
}

std::string scaleModeToName(ScaleMode mode)
{
    return mode < ScaleMode::LAST ? ScaleModeNames[static_cast<int>(mode)] : ScaleModeNames[0];
}

#define XX(s) #s
std::string ZScaleModeNames[] = {MODEStk(XX)};
#undef XX

ZScaleMode nameToZScaleMode( const std::string& s )
{
    auto res = std::find( ZScaleModeNames, ZScaleModeNames+static_cast<int>(ZScaleMode::LAST), s);
    ZScaleMode val = ZScaleMode( res - ZScaleModeNames );
    return val == ZScaleMode::LAST ? ZScaleMode::None : val;
}

std::string zScaleModeToName(ZScaleMode mode)
{
    return mode < ZScaleMode::LAST ? ZScaleModeNames[static_cast<int>(mode)] : ZScaleModeNames[0];
}

// TODO obsolete - use zScaleModeToName directly
char *humanZScale(ZScaleMode val, char *buff, int buffsize)
{
    strncpy(buff, zScaleModeToName(val).c_str(), buffsize);
    return buff;
}

#define X(s) #s
std::string OverlayModeNames[] = {OverModes(X)};
#undef X

OverlayMode nameToOverlayMode( const std::string& s )
{
    auto res = std::find( OverlayModeNames, OverlayModeNames+OverlayModeMAX, s);
    OverlayMode val = OverlayMode( res - OverlayModeNames );
    return val == OverlayModeMAX ? OverlayNone : val;
}

std::string overlayModeToName(OverlayMode mode)
{
    return mode < OverlayModeMAX ? OverlayModeNames[mode] : OverlayModeNames[0];
}

OverlayMode nextOverlay(OverlayMode val)
{
    OverlayMode res = (OverlayMode)((int) val + 1);
    return res >= OverlayModeMAX ? OverlayNone : res;
}

#include "themes.h"

#define Y(s) #s
std::string ThemeNames[] = {THEMES(Y)};
#undef Y


#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif
