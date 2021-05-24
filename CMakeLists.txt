cmake_minimum_required(VERSION 3.19)
project(YAIV)

# Warnings disabled pending cleanup
# "-Wall" "-Wpedantic" "-Wextra"
add_compile_options( "-fexceptions" "-fopenmp" ) #"-DDANBOORU" )

# optimize and strip appropriately
add_link_options( "$<$<CONFIG:MINSIZEREL>:-O2;-s>" "$<$<CONFIG:RELEASE>:-O2;>" )

set(CMAKE_CXX_STANDARD 14)
set(LINK_FLAGS -fopenmp -lX11 -lXext -lm -ldl -lXinerama -lXcursor -lXi -lXrender -lXfixes -ljpeg -lz -lXft -lfontconfig -pthread -lpthread -lwebp -lwebpdemux)

include_directories(. /home/kevin/proj/fltk/fltk)


add_executable(yaiv main.cpp Fl_Anim_GIF_Image.cpp Fl_Anim_GIF_Image.h humansize.cpp humansize.h list_rand.cpp list_rand.h rescaler.cpp rescaler.h rotate.cpp rotate.h MyW.h XBox.h Webp.cpp
        Webp.h checker.cpp checker.h XBox.cpp prefs.h MostRecentPaths.h MostRecentPaths.cpp )
#        sqlite-amalgamation-3350500/sqlite3.c danbooru.cpp danbooru.h)

find_library(FLTK fltk /home/kevin/proj/fltk/fltk/lib)
find_library(FLTK_IMG fltk_images /home/kevin/proj/fltk/fltk/lib)
# Using own PNG library to prevent output to console
find_library(FLTK_PNG fltk_png /home/kevin/proj/fltk/fltk/lib)

target_link_libraries(yaiv LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${FLTK_PNG} ${LINK_FLAGS})
