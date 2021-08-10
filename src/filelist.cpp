//
// Created by kevin on 8/10/21.
//

#include <algorithm> // min,max
#include <cstring> // strlen, strncpy
#include <cstdio> // sprintf
#include "filelist.h"
#include "list_rand.h"

filelist::filelist() {
    current_index = -1;
    file_list = nullptr;
    file_count = -1;
}

//TODO a file-filter callback is not in "vanilla" FLTK
#if 0
int removeFolders(struct dirent *entry) {
    // TODO this is a hack, we're not provided the base path
    const char * out = fl_filename_ext(entry->d_name);
    bool val = out != nullptr && *out != 0 && (out[1] != 0);
    return val;
}
#endif

int filelist::filename_path(const char* buf, char *to) { // TODO hack pending adding to FLTK
    const char *p = buf + strlen(buf) - 1;
    for (; *p != '/' && p != buf; --p) // TODO slash is possible '\' under windows
        ;
    if (p == buf) return 0;
    strncpy(to, buf, (p-buf)+1);
    to[(p-buf)] = '\0';
    return 1;
}

void filelist::load_filelist(const char *n) {
    if (file_list)
        fl_filename_free_list(&file_list, file_count);

    if (!fl_filename_isdir(n))
        filename_path(n, folder_name);
    else
        fl_filename_absolute(folder_name, FL_PATH_MAX, n);

    if (folder_name[strlen(folder_name) - 1] == '/')
        folder_name[strlen(folder_name) - 1] = 0x0;

    // TODO how best to filter for images?
    //TODO a file-filter callback is not in "vanilla" FLTK
    //file_count = fl_filename_list(folder_name, &file_list, fl_numericsort, removeFolders);
    file_count = fl_filename_list(folder_name, &file_list, fl_numericsort);
}

void filelist::load_file(const char *n) {
    load_filelist(n); // TODO background process
    current_index = 0;
    if (!fl_filename_isdir(n))
        current_index = find_file(n);
}

char *filelist::currentFilename() {
    return file_name;
}

int filelist::find_file(const char *n) {
    // determine the index in the file_list of the given filename
    const char *outfn = fl_filename_name(n);
    //logit("file_file:|%s|", (char *)outfn);
    for (int i=0; i < file_count; i++)
        if (strcmp(file_list[i]->d_name, outfn) == 0)
            return i;
        return 0;
}

bool filelist::canNext() {
    return file_list && current_index < (file_count-1);
}

bool filelist::canPrev() {
    return file_list && current_index > 0;
}

bool filelist::skip() {
//    return false;

    bool isdir = fl_filename_isdir(fullpath);
#ifndef _MSC_VER
    bool ishide = file_name[0] == '.';
#else
    bool ishide = false; // TODO windows hidden attribute
#endif
    return isdir || ishide; // TODO driven by prefs
}

const char *filelist::getCurrentFilePath() {

    // NOTE: initialize first for canPrev/canNext
    current_index = std::min(std::max(current_index,0), file_count-1);

    if (!file_list || file_count < 1) return nullptr;

    strncpy(file_name, file_list[current_index]->d_name, FL_PATH_MAX);

    sprintf(fullpath, "%s/%s", folder_name, file_list[current_index]->d_name);

    return fullpath;
}

void filelist::step(int delta) {
    if (!file_list || file_count < 1)
        return;

    int curr = current_index; // in case there are nothing but hidden files in this direction
    while (true) {
        current_index += delta;
        if (current_index < 0 || current_index >= file_count) {
            current_index = curr; // reached the end, don't change image
            return;
        }
        getCurrentFilePath(); // required for skip()
        if (!skip())
            return; // non-skippable, proceed
    }
}

void filelist::next() { step(+1); }

void filelist::prev() { step(-1); }

void filelist::randomize() {
    if (!file_list || file_count<=1)
        return;

    struct dirent **newlist = list_randomize(file_list, file_count);

    for (int i=0; i<file_count; i++)
        file_list[i] = newlist[i];

    free(newlist);

    current_index -= 1;
    next();
}

void filelist::home() {
    current_index = -1;
    next();
}

void filelist::end() {
    current_index = INT_MAX-1;
    prev();
}

int filelist::currentIndex() const {
    return current_index;
}

int filelist::fileCount() {
    return file_count;
}

void filelist::setCurrent(int val) {
    current_index = val-1;
    next();
}

void filelist::hide() {
    const char *fullpath = getCurrentFilePath();

    // TODO test with utf-8 folder, filename

#ifndef _MSC_VER
    std::string newpath = std::string(folder_name) + "/." + file_name;
    if (std::rename(fullpath, newpath.c_str()))
        return; // failure

    strcpy(file_list[current_index]->d_name, (std::string(".") + file_name).c_str());
#else
    // TODO Windows change hidden attribute
#endif
    next(); // TODO probably a problem if hiding the only image in the list
}

bool filelist::ishidden() {
    return file_list && file_name[0] == '.'; // TODO windows hidden attribute
}

static filelist *_filelist;

filelist* filelist::initFilelist(const char *n) {
    if (_filelist)
        delete _filelist;
    _filelist = new filelist();
    _filelist->load_file(n);
    return _filelist;
}
