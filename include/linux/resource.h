#ifndef     _RESOURCE_H_
#define     _RESOURCE_H_

#define RLIMIT_CPU  0       /*  CPU time in ms */
#define RLIMIT_FSIZE    1       /*  Maximum filesize */
#define RLIMIT_DATA 2       /*  max data size */
#define RLIMIT_STACK    3       /*  max stack size */
#define RLIMIT_CORE 4       /*  max core file size */
#define RLIMIT_RSS  5       /*  max resident set size */
#define RLIMIT_NPROC    6       /*  max number of processes */
#define RLIMIT_NOFILE   7       /*  max number of open files */

#define     RLIM_NLIMITS    8

struct rlimit {
    long rlim_cur;
    long rlim_max;
};

#endif
