#ifndef __METADATA_H__
#define __METADATA_H__

#include "prefs.h"
#include <FL/Fl_Group.H>

void init_metadata();
void create_metadata(Prefs *prefs, Fl_Group *container);
void update_metadata(const char *filename);

#endif // __METADATA_H__
