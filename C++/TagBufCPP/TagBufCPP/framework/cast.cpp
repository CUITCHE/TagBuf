//
//  cast.cpp
//  TagBufCPP
//
//  Created by hejunqiu on 16/9/16.
//  Copyright © 2016年 CHE. All rights reserved.
//

#include "cast.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <libproc.h>
#include <errno.h>

const uintptr_t MAX_CONSTANT_ADDRESS = [](){
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE] = {0};
    if (realpath("./", pathbuf) != nullptr) {
        int ret;
        pid_t pid;

        pid = getpid();
        ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
        if ( ret <= 0 ) {
//            fprintf(stderr, "PID %d: proc_pidpath ();\n", pid);
//            fprintf(stderr, "    %s\n", strerror(errno));
        } else {
//            printf("proc %d: %s\n", pid, pathbuf);
        }
    }
    FILE *fp = fopen(pathbuf, "r");
    fseek(fp, 0, SEEK_END);
    uint64_t length = ftell(fp);
    length |= 0x100000000;
    fclose(fp);

    return length;
}();
