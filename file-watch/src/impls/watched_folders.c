#include "watched_folders.h"
#include <dirent.h>

void set_watched_folder(WatchedFolders* wfds, int new_wd, const char* new_wd_name) {
    wfds->wfds[wfds->count].wd = new_wd;
    memcpy(wfds->wfds[wfds->count].name, new_wd_name, strlen(new_wd_name));
    wfds->count++;

#ifdef DEBUG
    printf("=== DEBUGGING ===\n");
    for (int i = 0; i < wfds->count; ++i) {
        printf("{%d} -> %s\n", wfds->wfds[i].wd, wfds->wfds[i].name);
    }
    printf("=== DEBUGGING ===\n");
    printf("\e[1;1H\e[2J");
#endif
}

const WatchedFolder* get_watched_folder(WatchedFolders* wfds, int wd) {
    for (int i = 0; i < wfds->count; ++i) {
        if (wfds->wfds[i].wd == wd) {
            return &wfds->wfds[i];
        }
    }

    return NULL;
}

void gather_directories(const char* dir_path, int inotify_fd, WatchedFolders* watched_folders) {
    DIR* directory = opendir(dir_path);
    if (directory == NULL) {
        fprintf(stderr, "Could not open folder %s: %s", dir_path, strerror(errno));
        exit(1);
    }

    int watch_fd = inotify_add_watch(inotify_fd, dir_path, IN_CREATE | IN_MODIFY | IN_DELETE);
    if (watch_fd == -1) {
        fprintf(stderr, "Could not start watching %s: %s\n", dir_path, strerror(errno));
        exit(1);
    }
    set_watched_folder(watched_folders, watch_fd, dir_path);

    char directory_holder[FILE_PATH_LEN] = {0};
    struct stat stat_buf = {0};
    while (1) {
        struct dirent* dirent = readdir(directory);
        if (dirent == NULL) {
            break;
        }

        strcat(directory_holder, dir_path);
        strcat(directory_holder, "/");
        strcat(directory_holder, dirent->d_name);

        if (lstat(directory_holder, &stat_buf) == 0) {
            if (stat_buf.st_mode & S_IFDIR && strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0 && strstr(dirent->d_name, ".git") == NULL && strstr(dirent->d_name, "node_modules") == NULL) {
                int watch_fd = inotify_add_watch(inotify_fd, directory_holder, IN_CREATE | IN_MODIFY | IN_DELETE);
                if (watch_fd == -1) {
                    fprintf(stderr, "Could not start watching %s: %s\n", dir_path, strerror(errno));
                    exit(1);
                }
                set_watched_folder(watched_folders, watch_fd, directory_holder);

                gather_directories(directory_holder, inotify_fd, watched_folders);
            }
        } else {
            fprintf(stderr, "There was an error iterating directory %s: %s", directory_holder, strerror(errno));
            break;
        }

        memset(directory_holder, 0, FILE_PATH_LEN);
    }
    closedir(directory);
}
