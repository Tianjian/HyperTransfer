#ifndef _COMMON_INCLUDE_H_
#define _COMMON_INCLUDE_H_

/** public head file */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <event.h>
#include <pthread.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <queue>
#include <map>
#include <string>
#include <iostream>
#include <cctype>
#include <openssl/md5.h>
#include "ConfigFile.h"

#include "common_def.h"

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/configurator.h>


#include "logger.h"

using namespace std;

#endif
