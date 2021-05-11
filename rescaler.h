//
// Created by kevin on 5/11/21.
//

#ifndef CLION_TEST2_RESCALER_H
#define CLION_TEST2_RESCALER_H

#include <FL/Fl_Image.H>

Fl_RGB_Image* itk_rescale( Fl_RGB_Image* img, unsigned w, unsigned h, int scaletype );

#endif //CLION_TEST2_RESCALER_H
