#define SOCK_PATH "/var/run/devd.pipe"
#define REGEX "subsystem=CDEV type=(CREATE|DESTROY) cdev=(d[a-z]{1}[0-9]{1}[a-z]{1}[0-9]{1}.*)"
#define WITH_SYSLOG
