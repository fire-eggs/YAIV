//
// Created by kevin on 8/10/21.
//

#include <algorithm> // min,max
#include <cstring> // strlen, strncpy
#include <cstdio> // sprintf
#include <iostream>
#include <string>
#include <fstream>

#include "filelist.h"
#include "list_rand.h"

filelist::filelist() {
    current_index = -1;
    file_list = nullptr;
    file_count = -1;
    file_name[0] = '\0';
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
    return file_count > 0 ? file_name : nullptr;
}

int filelist::find_file(const char *n) {
    // determine the index in the file_list of the given filename
    const char *outfn = fl_filename_name(n);
    //logit("file_file:|%s|", (char *)outfn);
    for (int i=0; i < file_count; i++)
    {
        if (strcmp(file_list[i]->d_name, outfn) == 0)
            return i;
    }
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
    return isdir || ishide; // || isHide(); // TODO driven by prefs
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

bool filelist::hide() {
    bool OK = addToHidden();
    next(); // TODO probably a problem if hiding the only image in the list
    return OK;
}

bool filelist::ishidden() {
    return file_list && (file_name[0] == '.' || isHide()); // TODO windows hidden attribute
}

static filelist *_filelist;

filelist* filelist::initFilelist(const char *n) {
    delete _filelist;
    _filelist = new filelist();
    _filelist->load_file(n);
    _filelist->loadFavs();
    _filelist->loadHidden();
    return _filelist;
}

void filelist::loadHidden()
{
    std::ifstream input("hidden.yaiv");
    std::string line;
    while (std::getline(input, line)){
        if (!line.empty())
            _hidden.push_back(line);
    }
}

void filelist::loadFavs()
{
    std::ifstream input("favs.yaiv");
    std::string line;
    while (std::getline(input, line)){
        if (!line.empty())
            _favs.push_back(line);
    }
}

bool filelist::addToHidden()
{
    _hidden.push_back(fullpath);
    // write out
    FILE *fp = fl_fopen("hidden.yaiv", "a+");
    if (!fp) // 20220530 fix crash when cannot open file
        return false;
    fputs(fullpath,fp);
    fputc('\n',fp);
    fclose(fp);
    return true;
}

bool filelist::addToFavs()
{
    _favs.push_back(fullpath);
    // write out
    FILE *fp = fl_fopen("favs.yaiv", "a+");
    if (!fp) // 20220530 fix crash when cannot open file
        return false;
    fputs(fullpath,fp);
    fputc('\n',fp);
    fclose(fp);
    return true;
}

bool filelist::isFav()
{
    return _filelist && !_favs.empty() && std::find(_favs.begin(), _favs.end(), fullpath) != _favs.end();
}

bool filelist::isHide()
{
    return _filelist && !_hidden.empty() && std::find(_hidden.begin(), _hidden.end(), fullpath) != _hidden.end();
}
