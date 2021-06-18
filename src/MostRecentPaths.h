//
// Created by kevin on 5/15/21.
//

#ifndef CLION_TEST2_MOSTRECENTPATHS_H
#define CLION_TEST2_MOSTRECENTPATHS_H

#include <vector>
#include <FL/Fl_Preferences.H>
#include "prefs.h"

class MostRecentPaths
{
private:
    std::vector<char*> _pathList;
    Fl_Preferences* _prefs;
    void ShuffleDown(int);
    MostRecentPaths(const MostRecentPaths&) = delete; // no copying!
    MostRecentPaths& operator=(const MostRecentPaths&) = delete;

public:
    explicit MostRecentPaths(Prefs* prefs);
    ~MostRecentPaths();
    void Add(const char* newPath);
    char** getAll();
    void Save();
    int getCount();
};

#endif //CLION_TEST2_MOSTRECENTPATHS_H
