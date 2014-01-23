#ifndef _SN_FILE_TRACK_H
#define _SN_FILE_TRACK_H

	/* INCLUDES */
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/inotify.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <signal.h>
	#include <time.h>
	
	/* CONSTANTS */
	#define APP_MAME "FileTrack"
	#define APP_USAGE "USAGE: ./filetrack <monitorize dir> [log path]"
	#define AUTHOR "Security Null"
	#define VERSION_MAJOR 1
	#define VERSION_MINOR 0
	#define VERSION_RELEASE 0
	#define WEBSITE "www.SecurityNull.net"
	#define LAST_UPDATE "22/01/2014"
	
	#ifndef PATH_MAX
		#define PATH_MAX 1024 // Max file path length
	#endif
	#define MAX_EVENTS 1024 // Max number of events to process at one go
	#define LEN_NAME 16 // Assuming that the length of the filename won't exceed 16 bytes
	#define EVENT_SIZE  (sizeof (struct inotify_event)) // Size of one event
	#define BUF_LEN     (MAX_EVENTS * (EVENT_SIZE + LEN_NAME)) // Buffer to store the data of events

	/* STRUCTS */
	typedef struct {
		int wd;
		char *full_path;
	} NOTIFY;
	
	/* PROTOTYPES */
	unsigned long addNewWatch(int, char *);
	int searchIDFromWD(int);
	void sigalCallback(int);
	
	/* VARS */
	int notify_events[] = {IN_CREATE, IN_DELETE, IN_MODIFY, IN_MOVED_FROM, IN_MOVED_TO, IN_ACCESS, IN_OPEN, IN_CLOSE/*, IN_CLOSE_WRITE, IN_CLOSE_NOWRITE, IN_ATTRIB*/};
	char *notify_events_name[] = {"CREATE", "DELETE", "MODIFY", "MOVE FROM", "MOVE TO", "ACCESS", "OPEN", "OPEN & CLOSE"/*, "WRITE", "NO WRITE", "CHANGE ATTRIB"*/};
	char *months_names[] = {"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"};
	FILE *fp_log;
	NOTIFY *paths;
	unsigned int paths_count = 0;
	char basedir[PATH_MAX];
	char last_name[PATH_MAX];
	char file_log_path[PATH_MAX];
	int save_log = 0;

#endif
