//
// Created by kevin on 8/10/21.
//

#ifndef YAIV_FILELIST_H
#define YAIV_FILELIST_H

#include <FL/filename.H>
#include <vector>
#include <string>

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

    std::vector<std::string> _hidden;
    std::vector<std::string> _favs;

    void loadHidden();
    void loadFavs();

    bool addToHidden();
    
public:
    static int filename_path(const char* buf, char *to);

    filelist();
    void load_filelist(const char *);
    void load_file(const char *);

    const char *getCurrentFilePath();
    char *currentFilename();

    bool any() { return realCount() > 0; }
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
    bool hide();
    bool ishidden();
    bool isFav();
    bool isHide();

    bool addToFavs();
    
    static filelist* initFilelist(const char *n);
    
    dirent *get_entry(int i);
    const char *getFolderName();
    void addToReal(const char *filename);
    void resetReal();
    int realCount();
    
    std::vector<std::string> RealFileList;
    int oldFileCount();
};


#endif //YAIV_FILELIST_H
