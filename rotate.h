//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_ROTATE_H
#define CLION_TEST2_ROTATE_H

#include <FL/Fl_Image.H>

Fl_RGB_Image* rotate90( Fl_RGB_Image* img ); // TODO hack
Fl_RGB_Image* rotate180( Fl_RGB_Image* img ); // TODO hack
Fl_RGB_Image* rotate270( Fl_RGB_Image* img ); // TODO hack
void discard_user_rgb_image( Fl_RGB_Image* &img ); // TODO hack

#endif //CLION_TEST2_ROTATE_H
