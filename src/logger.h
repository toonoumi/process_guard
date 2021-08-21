#include <stdio.h>
#include <time.h>
//#define DEBUG

#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

extern char* time_now(){
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    char*txt=asctime (timeinfo);
    txt[strlen(txt) - 1] = 0;
    return txt;
}

#ifdef DEBUG
#define FMT_STR "%s %s:%d:%s(): "
#define FUNC_DETAIL time_now(), __FILE__, __LINE__, __func__
#else
#define FMT_STR "%s \033[0;31m[INFO]\033[0m "
#define FUNC_DETAIL time_now()
#endif
#define logger(fmt, ...) fprintf(stderr, FMT_STR fmt "\n", FUNC_DETAIL, ##__VA_ARGS__)