//
// Created by kevin on 8/10/21.
//

#ifndef YAIV_FILELIST_H
#define YAIV_FILELIST_H

#include <FL/filename.H>

class filelist {

private:
    int current_index;
    char folder_name[FL_PATH_MAX] {'\0'};
    char file_name[FL_PATH_MAX] {'\0'};

    char fullpath[FL_PATH_MAX<<2] {'\0'};

    dirent** file_list{};
    int file_count;

    void step(int delta);
    bool skip();
    int find_file(const char *n);
    static int filename_path(const char* buf, char *to);

public:
    filelist();
    void load_filelist(const char *);
    void load_file(const char *);

    const char *getCurrentFilePath();
    char *currentFilename();

    bool any() { return file_list && file_count > 0; }
    bool canNext();
    bool canPrev();
    void next();
    void prev();
    void home();
    void end();
    void setCurrent(int val);

    int currentIndex() const;
    int fileCount();

    void randomize();
    void hide();
    bool ishidden();

    static filelist* initFilelist(const char *n);
};


#endif //YAIV_FILELIST_H
