//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_LIST_RAND_H
#define CLION_TEST2_LIST_RAND_H

#include <FL/Fl_Image.H>
#include <XBox.h>

struct dirent ** list_randomize(struct dirent ** list, int count);
Fl_Image *loadFile(char *filename, XBox *owner);

#endif //CLION_TEST2_LIST_RAND_H
