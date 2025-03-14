//
// Created by kevin on 5/15/21.
//

#ifndef CLION_TEST2_PREFS_H
#define CLION_TEST2_PREFS_H

#include <string>
#include <FL/Fl_Preferences.H>

#define APPLICATION "yaiv"
#define ORGANIZATION "yaiv.org"

#define MAIN_PREFIX "main"
#define MRU_GROUP "MRU"

// Colors for the image drawing area
#define CANVAS_COLOR "CanvasColor"
#define CANVAS_LABEL_COLOR "CanvasLabelColor"

#define MAIN_COLOR "MainColor"
#define MAIN_LABEL_COLOR "MainLabelColor"

#define SLIDESHOW_DELAY "SlideShow_Delay"
#define SLIDE_SHUFFLE   "SlideShow_Shuffle"
#define SLIDE_BORDER    "SlideShow_Border"
#define SLIDE_ERRORS    "SlideShow_SkipErrorFiles"

#define BORDER_FLAG "BORDER"

#define MOUSE_PAN "PanWithMouse"

#define SCALE_MODE "DefaultScaling"
#define DITHER_MODE "DefaultDither"

#define OVERLAY "Overlay"
#define CHECKER "Checkerboard"
#define MINIMAP "MiniMap"
#define LOAD_SHUFFLE "ShuffleOnLoad"

class Prefs : public Fl_Preferences
{
public:
    Prefs() : Fl_Preferences(Fl_Preferences::USER, ORGANIZATION, APPLICATION)
    {}

    void getWinRect(const char* prefix, int& x, int& y, int& w, int& h)
    {
        std::string n = prefix;
        Fl_Preferences::get((n + "_x").c_str(), x,  50);
        Fl_Preferences::get((n + "_y").c_str(), y,  50);
        Fl_Preferences::get((n + "_w").c_str(), w, 400);
        Fl_Preferences::get((n + "_h").c_str(), h, 400);
    }
    
    void getWinRect(const char* prefix, int& x, int& y, int& w, int& h, 
                    int prefx, int prefy, int prefw, int prefh )
    {
        std::string n = prefix;
        Fl_Preferences::get((n + "_x").c_str(), x, prefx);
        Fl_Preferences::get((n + "_y").c_str(), y, prefy);
        Fl_Preferences::get((n + "_w").c_str(), w, prefw);
        Fl_Preferences::get((n + "_h").c_str(), h, prefh);
    }

    void setWinRect(const char* prefix, int x, int y, int w, int h)
    {
        std::string n = prefix;
        set((n + "_x").c_str(), x);
        set((n + "_y").c_str(), y);
        set((n + "_w").c_str(), w);
        set((n + "_h").c_str(), h);
        flush();
    }

    void set2(const char* n, int val)
    {
        Fl_Preferences::set(n, val);
        flush();
    }

    void getS(const char *val, std::string& str, const std::string& defaultVal)
    {
        char *text;
        Fl_Preferences::get(val, text, defaultVal.c_str());
        str = text;
        free(text);
    }

    void getHex(const char *n, unsigned int& val, unsigned int def)
    {
        char *text;
        Fl_Preferences::get(n, text, "");
        if (text[0] == '\0')
            val = def;
        else {
            val = strtoul(text, nullptr, 16);
        }
        free(text);
    }

    void setHex(const char *n, unsigned int val)
    {
        // FLTK colors are in the format RGBA
        char buf[10];
        sprintf(buf, "%X", val);
        Fl_Preferences::set(n, buf);
    }
};
#endif //CLION_TEST2_PREFS_H
