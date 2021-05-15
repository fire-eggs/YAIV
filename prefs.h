//
// Created by kevin on 5/15/21.
//

#ifndef CLION_TEST2_PREFS_H
#define CLION_TEST2_PREFS_H

#include <string>
#include <FL/Fl_Preferences.H>

#define APPLICATION "yaiv"
#define ORGANIZATION "iglite.com"

#define MAIN_PREFIX "main"
#define MRU_GROUP "MRU"

class Prefs : public Fl_Preferences
{
public:
    Prefs() : Fl_Preferences(Fl_Preferences::USER, ORGANIZATION, APPLICATION)
    {}

    void getWinRect(const char* prefix, int& x, int& y, int& w, int& h)
    {
        std::string n = prefix;
        get((n + "_x").c_str(), x,  50);
        get((n + "_y").c_str(), y,  50);
        get((n + "_w").c_str(), w, 400);
        get((n + "_h").c_str(), h, 400);
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


};
#endif //CLION_TEST2_PREFS_H
