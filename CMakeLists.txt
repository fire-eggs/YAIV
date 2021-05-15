cmake_minimum_required(VERSION 3.19)
project(clion_test2)

# Warnings disabled pending cleanup
# "-Wall" "-Wpedantic" "-Wextra"
add_compile_options( "-fexceptions" "-fopenmp" )

# optimize and strip appropriately
add_link_options( "$<$<CONFIG:MINSIZEREL>:-O2;-s>" "$<$<CONFIG:RELEASE>:-O2;>" )

set(CMAKE_CXX_STANDARD 14)
set(LINK_FLAGS -fopenmp -lX11 -lXext -lm -ldl -lXinerama -lXcursor -lXi -lXrender -lXfixes -ljpeg -lpng -lz -lXft -lfontconfig -pthread -lpthread -lwebp -lwebpdemux)

include_directories(. /home/kevin/proj/fltk/fltk)


add_executable(clion_test2 main.cpp Fl_Anim_GIF_Image.cpp Fl_Anim_GIF_Image.h humansize.cpp humansize.h list_rand.cpp list_rand.h rescaler.cpp rescaler.h rotate.cpp rotate.h MyW.h XBox.h Webp.cpp Webp.h checker.cpp checker.h XBox.cpp prefs.h MostRecentPaths.h MostRecentPaths.cpp)

find_library(FLTK fltk /home/kevin/proj/fltk/fltk/lib)
find_library(FLTK_IMG fltk_images /home/kevin/proj/fltk/fltk/lib)

target_link_libraries(clion_test2 LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${LINK_FLAGS})