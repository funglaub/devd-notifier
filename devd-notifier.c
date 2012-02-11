/* Tue Feb  7 15:29:02 2012 */

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

regex_t regex;

void cleanup() {
#ifdef WITH_SYSLOG
  closelog();
#endif
  regfree(&regex);
  exit(EXIT_SUCCESS);
}

void signal_handler(int sig) {
  if(sig == SIGTERM || sig == SIGQUIT) {
#ifdef WITH_SYSLOG
    syslog(LOG_NOTICE, "Received signal %i. Terminating", sig);
#endif
    cleanup();
  }
}
  
int main(int argc, char *argv[])
{

  struct sockaddr_un address;
  int  socket_fd, nbytes;
  char buffer[256];

  pid_t pid, sid;
  regmatch_t pmatch[1];
  char *match;

  regcomp(&regex, REGEX, REG_EXTENDED);

  signal(SIGTERM, signal_handler);
  signal(SIGQUIT, signal_handler);

#ifdef WITH_SYSLOG
  openlog("devd notifier", LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "Starting...");
#endif
  
  pid = fork();
  if (pid < 0) {
    perror("Error");
    exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
     we can exit the parent process. */
  if (pid > 0) {
#ifdef WITH_SYSLOG
    syslog(LOG_NOTICE, "forked to background with PID %i", pid);
#endif
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
  socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if(socket_fd < 0) {
    perror("socket() failed");
    return 1;
  }

  /* start with a clean address structure */
  memset(&address, 0, sizeof(struct sockaddr_un));
 
  address.sun_family = AF_UNIX;
  strcpy(address.sun_path, SOCK_PATH);

  if(connect(socket_fd, 
             (struct sockaddr *) &address, 
             sizeof(struct sockaddr_un)) != 0) {
    perror("connect() failed");
    return 1;
  }
  notify_init("devd notifier");
  
  while ((nbytes = read(socket_fd, buffer, 256)) > 0) {
    buffer[nbytes] = 0;
    if (regexec(&regex, buffer, 1, pmatch, 0) == 0) {
      match = strndup(buffer + pmatch[0].rm_so, pmatch[0].rm_eo - pmatch[0].rm_so);
      
      NotifyNotification *msg = notify_notification_new("devd", match, NULL);
      notify_notification_show(msg, NULL);
    }
  }

 close(socket_fd);
 
return 0;
}
