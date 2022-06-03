
include(toolbar/CMakeLists.txt)

add_compile_options( "-fexceptions" "-fopenmp" "-DFLTK_USE_SVG" )

# optimize and strip appropriately
add_link_options( "$<$<CONFIG:MINSIZEREL>:-O2;-s>" "$<$<CONFIG:RELEASE>:-O2;>" )

#set(CMAKE_CXX_STANDARD 14)

# Using pango provides Unicode character display support
set(LINK_FLAGS -no-pie -fopenmp -lX11 -lXext -lm -ldl -lXinerama -lXcursor -lXi 
               -lXrender -lXfixes -ljpeg -lz -lXft -lfontconfig -pthread
               -lpangocairo-1.0 -lpango-1.0 -lcairo -lpangoft2-1.0 -lgobject-2.0
               -lglib-2.0 -lharfbuzz -lpangoxft-1.0 -lpthread -lwebp -lwebpdemux)

include_directories(. /home/kevin/proj/fltk/fltk /home/kevin/proj/YAIV/fl_imgtk/inc
        ${PROJECT_SOURCE_DIR} toolbar
        )

add_executable(yaiv main.cpp yaiv_win.cpp Fl_Anim_GIF_Image.cpp humansize.cpp list_rand.cpp
               Webp.cpp checker.cpp XBox.cpp MostRecentPaths.cpp Slideshow.cpp apng.cpp
               Fl_TransBox.cpp modes.cpp XBoxMenu.cpp SmoothResizeGroup.cpp SmoothResizeGroup.h
               ${srcs} mediator.cpp mediator.h themes.cpp themes.h filelist.cpp filelist.h
        whereami.c whereami.h fileScanThread.cpp)

add_executable(yaiv_db main.cpp Fl_Anim_GIF_Image.cpp humansize.cpp list_rand.cpp
               Webp.cpp checker.cpp XBox.cpp MostRecentPaths.cpp Slideshow.cpp apng.cpp
               Fl_TransBox.cpp danbooru.cpp yaiv_win.cpp ${srcs}
               ${PROJECT_SOURCE_DIR}/sqlite-amalgamation-3350500/sqlite3.c modes.cpp XBoxMenu.cpp SmoothResizeGroup.cpp
               SmoothResizeGroup.h mediator.cpp mediator.h themes.cpp themes.h filelist.cpp filelist.h
               whereami.c whereami.h fileScanThread.cpp)

target_compile_definitions(yaiv_db PUBLIC "-DDANBOORU")

find_library(FLTK fltk /home/kevin/proj/fltk/fltk/lib)
find_library(FLTK_IMG fltk_images /home/kevin/proj/fltk/fltk/lib)
# Using own [modified] PNG library to prevent output to console
find_library(FLTK_PNG fltk_png /home/kevin/proj/fltk/fltk/lib)
find_library(FLIMGTK fl_imgtk /home/kevin/proj/YAIV/fl_imgtk/lib)

target_link_libraries(yaiv LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${FLTK_PNG} ${FLIMGTK} ${LINK_FLAGS})
target_link_libraries(yaiv_db LINK_PUBLIC ${FLTK} ${FLTK_IMG} ${FLTK_PNG} ${FLIMGTK} ${LINK_FLAGS})

