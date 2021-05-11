//
// Created by kevin on 5/11/21.
//
#include "checker.h"
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Pixmap.H>

// A grey+white pixmap to use as a checker board background
static const char *const checker_xpm[] = {"20 20 2 1",
                                          " 	c #CCCCCCCCCCCC",
                                          ".	c #FFFFFFFFFFFF",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "          ..........",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          ",
                                          "..........          "};
Fl_Tiled_Image *checkerBoard;

void makeChecker()
{
    checkerBoard = new Fl_Tiled_Image(new Fl_Pixmap((const char *const *)checker_xpm));
}

void drawChecker(int X, int Y, int W, int H) {
// TODO checkerboard position/size needs to be recalculated for rotation
// TODO FL_Tiled_Image is drawing incorrectly when using params 5 and 6 (cx,cy)
//::checkerBoard->draw(drawx, drawy, outw-2, outh-2, deltax, deltay); // see -2 comment below
    checkerBoard->draw(X,Y,W,H);
}