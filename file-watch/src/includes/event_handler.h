#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_
#include <sys/inotify.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "watched_folders.h"

typedef enum {
    CREATED = 0,
    MODIFIED = 1,
    DELETED = 2
} LOG_TYPE;

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + FILE_PATH_LEN))

void handle_events(int notify_fd, WatchedFolders* wfds);
void log_entry(LOG_TYPE type, const char* full_file_path);

#endif // EVENT_HANDLER_H_
