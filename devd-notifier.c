#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <signal.h>
#include <syslog.h>
#include <regex.h>
#include <libnotify/notify.h>

#include "config.h"
#include "devd-notifier.h"

void display_usage() {
  printf("Usage: devd-notifier [OPTION]\n");
  printf("Options:\n");
  printf("%-10s %-s\n", "-f", "don't fork to background");
  printf("%-10s %-s\n",  "-h", "show this help");
  exit(EXIT_FAILURE);
}

void cleanup() {
  closelog();
  regfree(&regex);
  notify_uninit();
  exit(EXIT_SUCCESS);
}

void signal_handler(int sig) {
  if(sig == SIGTERM || sig == SIGQUIT) {
    syslog(LOG_NOTICE, "Received signal %i. Terminating", sig);
    cleanup();
  }
}

void daemonize() {
  pid_t pid, sid;
  
  pid = fork();
  if (pid < 0) {
    perror("Error");
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
     we can exit the parent process. */
  if (pid > 0) {
    syslog(LOG_NOTICE, "forked to background with PID %i", pid);
    exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();

  if (sid < 0) {
    /* Log the failure */
    perror("Error retrieving a new SID");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir(getenv("HOME"))) < 0) {
    /* Log the failure */
    perror("Error changing working directory");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  int opt = 0;
  int  socket_fd, nbytes;
  int nmatches = 3;
  int i;

  struct sockaddr_un address;
  
  char buffer[256]; // temporary buffer for socket reading
  char message[256]; // the message we output
  char *match, *p;

  // Strings for devd action types
  const char *create = "attached";
  const char *destroy = "detached";
  const char *action = NULL;

  // We will have 3 matches
  // 1st: The whole string
  // 2nd: The type (create/destroy)
  // 3rd: The device name
  regmatch_t pmatch[nmatches];

  memset(&message, 0, strlen(message));

  // by default call daemonize() and fork to background
  globalArgs.daemonize = 1;
  
  while( opt != -1 ) {
    switch(opt) {
      case 'f':
        globalArgs.daemonize = 0;
        break;
      case 'h':
      case '?':
        display_usage();
        break;
     }
    opt = getopt( argc, argv, optString );
  }

  regcomp(&regex, REGEX, REG_EXTENDED);

  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);

  openlog("devd notifier", LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "Starting...");

  if (globalArgs.daemonize == 1)
    daemonize();

  socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  
  if(socket_fd < 0) {
    perror("socket() failed");
    exit(EXIT_FAILURE);
  }

  /* start with a clean address structure */
  memset(&address, 0, sizeof(struct sockaddr_un));

  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, SOCK_PATH);

  if(connect(socket_fd, 
             (struct sockaddr *) &address, 
             sizeof(struct sockaddr_un)) != 0) {
    perror("connect() failed");
    exit(EXIT_FAILURE);
  }
  
  notify_init("devd notifier");

  while ((nbytes = read(socket_fd, buffer, 256)) > 0) {
    buffer[nbytes] = 0;

    if (regexec(&regex, buffer, nmatches, pmatch, 0) == 0) {
      p = buffer;
      
      for (i = 1; i < nmatches; i++) {
        if (pmatch[i].rm_so == -1)
          break;

        match = strndup (p + pmatch[i].rm_so, pmatch[i].rm_eo - pmatch[i].rm_so);

        if ( i == nmatches-2) {
            if ( strcmp(match, "CREATE") == 0 )
              action = create;
            else
              action = destroy;
        }
        
        if (i == nmatches-1)
          sprintf(message, "%s %s", match, action);

        free(match);
      } // for

      p += pmatch[0].rm_eo;

      if (globalArgs.daemonize == 0)
        printf("%s\n", message);

      NotifyNotification *msg = notify_notification_new("devd", message, NULL);
      notify_notification_show(msg, NULL);
     }
  }

  close(socket_fd);
  exit(EXIT_SUCCESS);
}
