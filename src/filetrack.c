/* INCLUDES */
#include "filetrack.h"

/* ENTRY POINT */
int main(int argc, char **argv) {
	// APP info
	printf("%s %i.%i.%i [%s] (%s) - %s\n\n", APP_MAME, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, AUTHOR, LAST_UPDATE, WEBSITE);
	
	char current_path[PATH_MAX];
	
	// Check APP params
	if (argc <= 1) {
		printf("%s\n", APP_USAGE);
		exit(0);
	} else {
		// Monitorize custom path
		strcpy(basedir, argv[1]);
		
		if (basedir[0] == '.') {
			// Use current path
			if (getcwd(current_path, sizeof(current_path)) != NULL)
				strcpy(basedir, current_path);
			else
				perror("getcwd() error");
		}
		
		if (basedir[strlen(basedir) - 1] != '/')
			strcat(basedir, "/");

		if (argc >= 3) {
			// Save log into file
			strcpy(file_log_path, argv[2]);
			save_log = 1;
			
			printf("Saving to log '%s'.\n", file_log_path);
		}
	}

	// Initialize array
	paths = (NOTIFY *) malloc((paths_count + 1) * sizeof(NOTIFY));
	
	// Capture signal
	signal(SIGINT, sigalCallback);
 
	// File LOG
	if (save_log) {
		fp_log = fopen(file_log_path, "a");
		if (fp_log == NULL) {
			fprintf(fp_log, "Error opening file LOG. All output will be redirected to the 'stdout'.\n");
			fp_log = stdout;
		}
	} else
		fp_log = stdout;
	
	// Start inotify
	int fd_notify;
	#ifdef IN_NONBLOCK
		fd_notify = inotify_init1(IN_NONBLOCK);
	#else
		fd_notify = inotify_init();
	#endif

	if (fd_notify < 0) {
		if (save_log)
			fprintf(fp_log, "ERROR: Couldn't initialize inotify.\n");
		else
			perror("ERROR: Couldn't initialize inotify.");
			
		exit(0);
	}

	// Add base dir to inotify watch
	unsigned long events_count = addNewWatch(fd_notify, basedir);
	printf("Total Events: %lu\n\n", events_count);
   
   	int length, i;
	char buffer[BUF_LEN];
	while (1) {
		i = 0;
		length = read(fd_notify, buffer, BUF_LEN);  

		if (length < 0)
			perror("ERROR: read().");

		/* Read the events*/
		while (i < length) {
			struct inotify_event *event = (struct inotify_event *) &buffer[i];
			if (event->len) {				
				time_t t = time(NULL);
				struct tm tm = *localtime(&t);
				char fullpath[PATH_MAX];

				int id = searchIDFromWD(event->wd);
				if (id >= 0)
					strcpy(fullpath, paths[id].full_path);
				else
					fullpath[0] = '\0';

				int e;
				for (e = 0; e < sizeof(notify_events) / sizeof(notify_events[0]); e++) {
					if (event->mask & notify_events[e]) {
						if (strcmp(last_name, event->name) != 0) {
							if (event->mask & IN_ISDIR)
								fprintf(fp_log, "[%02d:%02d:%02d %02d/%s/%d - %s] %s%s (DIR)\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, months_names[tm.tm_mon], tm.tm_year + 1900, notify_events_name[e], paths[id].full_path, event->name);
							else
								fprintf(fp_log, "[%02d:%02d:%02d %02d/%s/%d - %s] %s%s (FILE)\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, months_names[tm.tm_mon], tm.tm_year + 1900, notify_events_name[e], paths[id].full_path, event->name);    
						}
					
						strcpy(last_name, event->name);
					}
				}

				if (save_log)
					fflush(fp_log);
			}
			
			i += EVENT_SIZE + event->len;
		}
	}

	close(fd_notify);

	return 0;
}

unsigned long addNewWatch(int fd, char *path) {
	int wd;
	unsigned long events_count = 0;
	struct dirent *entry;
	DIR *dp;

	// Add backslash if necessary
	if (path[strlen(path) - 1] != '/')
		strcat(path, "/");

	dp = opendir(path);
	if (dp == NULL) {
		perror("ERROR: Opening the directory.");
		exit(0);
	}

	// Add watch
	wd = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_ACCESS | IN_MOVE | IN_ATTRIB | IN_OPEN); 
	if (wd == -1)
		fprintf(fp_log, "ERROR: Couldn't add watch to %s\n", path);
	else {
		paths = (NOTIFY *) realloc(paths, (paths_count + 1) * sizeof(NOTIFY));
		paths[paths_count].wd = wd;
		paths[paths_count].full_path = (char *) malloc((strlen(path) + 1) * sizeof(char));
		strcpy(paths[paths_count].full_path, path);	
		paths_count++;
		events_count++;
	}

	while ((entry = readdir(dp))) {
		char new_dir[1024];
		// If its a directory, add a watch recursively
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, (char *) "..") != 0 && strcmp(entry->d_name, ".") != 0) {
				strcpy((char *) new_dir, path);
				//strcat((char *) new_dir, (char *) "/");
				strcat((char *) new_dir, entry->d_name);

				events_count += addNewWatch(fd, (char *) new_dir);
			}
		}
	}

	closedir(dp);
	
	return events_count;
}

int searchIDFromWD(int wd) {
	if (wd) {
		int i;
		for (i = 0; i < paths_count; i++)
			if (paths[i].wd == wd) 
				return i;
	}
	
	return -1;
}
 
void sigalCallback(int sigal) {
	// Flush and close log file
	if (save_log) {
		fflush(fp_log);
		fclose(fp_log);
	}
	
	// Clean memory
	free(paths);

    exit(0);
}
