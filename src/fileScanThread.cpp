#define HAVE_PTHREAD
#define HAVE_PTHREAD_H

#include "threads.h"
#include "filelist.h"

extern int getImageFormat(const char *);

Fl_Thread scanThread;

// TODO hacky
extern filelist* _filelist;

void *fileScanner(void *p)
{
    const char *fn = _filelist->getFolderName();

    bool first = true;
    for (int i=0; i < _filelist->oldFileCount(); i++)
    {
        dirent *ent = _filelist->get_entry(i);
        if (ent->d_type != DT_REG)
            continue;
        
        char fullpath[FL_PATH_MAX*2];
        sprintf(fullpath, "%s/%s", fn, ent->d_name);
        
        int format = getImageFormat(fullpath); 
        if (!format)
            continue;

        if (first) printf("add to real\n");
        first = false;
        _filelist->addToReal(ent->d_name);
    }   
    
    return NULL;
}

void fire_scan_thread()
{
    fl_create_thread(scanThread, fileScanner, NULL);
}
