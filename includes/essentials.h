#ifndef ESSENTIALS_H
#define ESSENTIALS_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

//#include <dirent.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <time.h>


#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "httpHelper.h" // Precisa estar acima de webSpaceManager
#include "webSpaceManager.h" 
#include "ioHelper.h"
#include "main.h"
#include "cmdLinkedList.h"
#include "serverHelper.h"

#endif
