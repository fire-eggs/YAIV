//
// Created by kevin on 8/10/21.
//

#include <algorithm> // min,max
#include <cstring> // strlen, strncpy
#include <cstdio> // sprintf
#include <iostream>
#include <string>
#include <fstream>
#include <random>

#include "filelist.h"
#include "list_rand.h"

filelist::filelist() {
    current_index = -1;
    file_list = nullptr;
    file_count = -1;
    file_name[0] = '\0';
}

const char *filelist::getFolderName() { return folder_name; }

extern int getImageFormat(const char *);
extern void fire_scan_thread();

// Determine the directory portion of a full filename
// E.g. given <path>/filename.ext, return <path>
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

    // Get a clean folder path from a folder or filename
    folder_name[0] = '\0';
    if (!fl_filename_isdir(n))
        filename_path(n, folder_name);
    else
        fl_filename_absolute(folder_name, FL_PATH_MAX, n);

    if (folder_name[strlen(folder_name) - 1] == '/')
        folder_name[strlen(folder_name) - 1] = 0x0;

    // Get the list of all filenames in the specified folder
    file_count = fl_filename_list((const char *)folder_name, &file_list, fl_numericsort);
    
    // Let the file scanner iterate over the file list and filter out non-image files
    resetReal();
    fire_scan_thread();
}

void filelist::load_file(const char *n) {
    load_filelist(n);
    current_index = 0;
    if (!fl_filename_isdir(n))
        current_index = find_file(n);
}

char *filelist::currentFilename() {
    return realCount() > 0 ? file_name : nullptr;
}

int filelist::find_file(const char *n) {
    
    // determine the index in the file list of the given filename
    const char *outfn = fl_filename_name(n);
    int rc = realCount();
    //logit("file_file:|%s|", (char *)outfn);
    for (int i=0; i < rc; i++)
    {
        if (strcmp(RealFileList[i].c_str(), outfn) == 0)
            return i;
    }
    return 0;
}

bool filelist::canNext() {
    // Is it possible to move to the "next" image?
    return realCount() > 0 && current_index < (realCount()-1);
}

bool filelist::canPrev() {
    // Is it possible to move to the "previous" image?
    return realCount() > 0 && current_index > 0;
}

bool filelist::skip() {
//    return false;

    bool isdir = fl_filename_isdir(fullpath);
#ifndef _MSC_VER
    bool ishide = file_name[0] == '.'; // Linux files are hidden with leading period
#else
    bool ishide = false; // TODO windows hidden attribute
#endif
    return isdir || ishide; // || isHide(); // TODO driven by prefs
}

const char *filelist::getCurrentFilePath() {

    int rc = realCount();
    // NOTE: initialize first for canPrev/canNext
    current_index = std::min(std::max(current_index,0), rc-1);

    if (!rc) 
        return nullptr;

    strncpy(file_name, RealFileList[current_index].c_str(), FL_PATH_MAX);

    sprintf(fullpath, "%s/%s", folder_name, file_name);

    return fullpath;
}

void filelist::step(int delta) {
    int rc = realCount();
    if (!rc)
        return;

    int curr = current_index; // in case there are nothing but hidden files in this direction
    while (true) {
        current_index += delta;
        if (current_index < 0 || current_index >= rc) {
            current_index = curr; // reached the end, don't change image
            return;
        }
                
        //getCurrentFilePath(); // required for skip()
        //if (!skip())
        //    return; // non-skippable, proceed
        return; // TODO skip
    }
}

void filelist::next() { step(+1); }

void filelist::prev() { step(-1); }

void filelist::randomize() {
    
    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(RealFileList), std::end(RealFileList), rng);
    return;
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
    return realCount();
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

//static 
filelist *_filelist; // Hack for scanner: TODO pass as parameter

filelist* filelist::initFilelist(const char *n) {
    delete _filelist;
    _filelist = new filelist();
    _filelist->load_file(n);
    _filelist->loadFavs(); // TODO don't reload each time?
    _filelist->loadHidden(); // TODO don't reload each time?
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

void filelist::addToReal(const char* filename)
{
    RealFileList.push_back(std::string(filename));
}

void filelist::resetReal()
{
    RealFileList.clear();
}

int filelist::realCount()
{
    return RealFileList.size();
}

int filelist::oldFileCount()
{
    return file_count;
}

dirent *filelist::get_entry(int i)
{ return file_list[i]; }    
