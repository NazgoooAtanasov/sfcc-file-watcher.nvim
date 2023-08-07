#ifndef WACHED_FOLDERS_H
#define WACHED_FOLDERS_H

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/inotify.h>

#define FILE_PATH_LEN 1024 /* if file path is more than 1KB, wtf */
#define MAX_WATCHED_FOLDERS 1024 * 9

typedef struct {
    char name[FILE_PATH_LEN];
    int wd;
} WatchedFolder;


typedef struct {
    WatchedFolder wfds[MAX_WATCHED_FOLDERS];
    int count;
} WatchedFolders;


const WatchedFolder* get_watched_folder(WatchedFolders* wfds, int wd);
void set_watched_folder(WatchedFolders* wfds, int new_wd, const char* new_wd_name);
void gather_directories(const char* dir_path, int inotify_fd, WatchedFolders* watched_folders);

#endif // WACHED_FOLDERS_H
