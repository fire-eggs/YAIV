//
// A separate thread to scan the files in a folder. Given a list of file paths,
// filters out any not-supported files (i.e. non-images, not supported image formats).
//

#define HAVE_PTHREAD
#define HAVE_PTHREAD_H

#include <fcntl.h> // O_RDONLY
#include <unistd.h> // close()

#include "threads.h"
#include "filelist.h"
#include "mediator.h" // TODO define messages elsewhere?

extern int getImageFormat(const char *);
extern int getImageFormatAt(int, const char *);

Fl_Thread scanThread;

// TODO need to turn this into a class, and a friend of filelist ?

// TODO hacky
extern filelist* _filelist;

void *fileScanner(void *p)
{
    const char *fn = _filelist->getFolderName();

/* TODO This attempted optimization is often failing (Feb 1, 2023). It
 * attempts to use openat() instead of open(). Unfortunately, getImageFormatAt 
 * fails to read the image headers when processing some directories. I don't 
 * have a good idea why this is a problem, so revert to getImageFormat for now...
    
    int dirfd = open(fn, O_RDONLY|O_DIRECTORY);
    if (dirfd != -1)
    {
        int updCount = 0;
        
        // NOTE: tried to use omp but failed [memory corruption in addToReal]
        for (int i=0; i < _filelist->oldFileCount(); i++)
        {           
            dirent *ent = _filelist->get_entry(i);
            if (ent->d_type != DT_REG)
                continue;
            
            int format = getImageFormatAt(dirfd, ent->d_name);
            if (!format)
                continue;
            
            _filelist->addToReal(ent->d_name);
         
            updCount ++;
            if ((updCount % 50) == 1) // update only periodically
                send_message(Mediator::MSG_REALUPDATE, 0);
        }
        close(dirfd);
        send_message(Mediator::MSG_REALUPDATE, 0);  // final update
        return NULL;
    }
*/    
    int updCount = 0;

//    bool first = true;
    for (int i=0; i < _filelist->oldFileCount(); i++)
    {
        dirent *ent = _filelist->get_entry(i);

        // 20240525 files accessed via SAMBA are of type DT_UNKNOWN. Stop limiting to regular files, 
        // which has the benefit of handling symbolic links too. 
//        if (ent->d_type != DT_REG)
//            continue;
        if (ent->d_type == DT_DIR || ent->d_type == DT_CHR || ent->d_type == DT_BLK)
            continue;
        
        
        char fullpath[FL_PATH_MAX*2];
        strcpy(fullpath, fn);
        strcat(fullpath, "/");
        strcat(fullpath, ent->d_name);
        //sprintf(fullpath, "%s/%s", fn, ent->d_name);

        int format = getImageFormat(fullpath); 
        if (!format)
            continue;

//        if (first) printf("add to real\n");
//        first = false;
        _filelist->addToReal(ent->d_name);
        updCount ++;
        if ((updCount % 50) == 1) // update only periodically
            send_message(Mediator::MSG_REALUPDATE, 0);
    }   
    
    // final update
    send_message(Mediator::MSG_REALUPDATE, 0);
    
    return NULL;
}

void fire_scan_thread()
{
    fl_create_thread(scanThread, fileScanner, NULL);
}
