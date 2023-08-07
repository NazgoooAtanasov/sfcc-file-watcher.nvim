#include "event_handler.h"
#include "watched_folders.h"

void log_entry(LOG_TYPE type, const char* full_file_path) {
    fprintf(stdout, "%d %s\n", (int)type, full_file_path);
    fflush(stdout);
}

void handle_events(int notify_fd, WatchedFolders* wfds) {
    char buffer[BUF_LEN];
    ssize_t read_len;
    struct inotify_event* event;

    for (;;) {
        read_len = read(notify_fd, buffer, BUF_LEN);

        if (read_len == -1) {
            fprintf(stderr, "There was an error trying to read events from fd\n");
            exit(1);
        }

        if (read_len == 0) {
            break;
        }

        for (char* ptr = buffer; ptr < buffer + read_len; ptr += EVENT_SIZE + event->len) {
            event = (struct inotify_event*) ptr;
            const WatchedFolder* working_folder = get_watched_folder(wfds, event->wd);
            char full_file_path[FILE_PATH_LEN] = {0};
            sprintf(full_file_path, "%s/%s", working_folder->name, event->name);

            /* IN_CREATE | IN_MODIFY | IN_DELETE */
            if (event->mask & IN_CREATE) {
                if (event->mask & IN_ISDIR) {
                    if (working_folder == NULL) {
                        fprintf(stderr, "This should never happen\n");
                        exit(1);
                    }

                    int new_wd = inotify_add_watch(notify_fd, full_file_path, IN_CREATE | IN_DELETE | IN_MODIFY);

                    if (new_wd == -1) {
                        fprintf(stderr, "There was an error opening the new directory %s: %s\n", full_file_path, strerror(errno));
                        exit(1);
                    }

                    set_watched_folder(wfds, new_wd, full_file_path);
                }

                log_entry(CREATED, full_file_path);
            }

            if (event->mask & IN_MODIFY) {
                log_entry(MODIFIED, full_file_path);
            }

            if (event->mask & IN_DELETE) {
                if (event->mask & IN_ISDIR) {
                    const WatchedFolder* folder_to_rm = get_watched_folder(wfds, event->wd);
                    inotify_rm_watch(notify_fd, folder_to_rm->wd);
                }
                log_entry(DELETED, full_file_path);
            }
        }
    }
}
