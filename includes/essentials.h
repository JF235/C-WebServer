#ifndef ESSENTIALS_H
#define ESSENTIALS_H

#define _GNU_SOURCE

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "httpHelper.h" // Precisa estar acima de webSpaceManager
#include "webSpaceManager.h" 
#include "ioHelper.h"
#include "main.h"
#include "cmdLinkedList.h"
#include "serverHelper.h"
#include "errorHandling.h"
#include "trace.h"

#endif
