#include "watched_folders.h"
#include "event_handler.h"

static WatchedFolders watched_folders = {0};
int main(void) {
    int notify_fd = inotify_init();
    if (notify_fd == -1) {
        fprintf(stderr, "Could not open inotify\n");
        exit(1);
    }

    const char* dir_path = "/home/ng/_Forkpoint/acne/app-project";
    gather_directories(dir_path, notify_fd, &watched_folders);
    while(1) {
        handle_events(notify_fd, &watched_folders);
    }

    return 0;
}
