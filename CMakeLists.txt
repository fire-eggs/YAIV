cmake_minimum_required(VERSION 3.19)
project(clion_test2)

set(CMAKE_CXX_STANDARD 14)
set(LINK_FLAGS -fopenmp -lX11 -lXext -lm -ldl -lXinerama -lXcursor -lXi -lXrender -lXfixes -ljpeg -lpng -lz -lXft -lfontconfig -pthread -lpthread -lwebp -lwebpdemux)

include_directories(. /home/kevin/proj/fltk/fltk)


add_executable(clion_test2 main.cpp Fl_Anim_GIF_Image.cpp Fl_Anim_GIF_Image.h humansize.cpp humansize.h list_rand.cpp list_rand.h rescaler.cpp rescaler.h rotate.cpp rotate.h MyW.h XBox.h Webp.cpp Webp.h checker.cpp checker.h XBox.cpp)

find_library(FLTK fltk /home/kevin/proj/fltk/fltk/lib)
find_library(FLTK_IMG fltk_images /home/kevin/proj/fltk/fltk/lib)

target_link_libraries(clion_test2 LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${LINK_FLAGS})