//
// Created by alessio on 18/03/16.
//

#include <ra_print_wrapping.h>

#ifdef __ANDROID

#include <android/log.h>

int  real_ra_fprintf (FILE *__restrict __stream, const char *__restrict __format, ...){
#ifdef DEBUG
    va_list arglist;
    va_start(arglist, __format);
    if(__stream == stdout){
        __android_log_vprint(ANDROID_LOG_INFO,"Remote_attestation",__format,arglist);
    }else{
        __android_log_vprint(ANDROID_LOG_ERROR,"Remote_attestation",__format,arglist);
    }
    va_end( arglist );
#endif
    return 0;
}

int  real_ra_printf (const char *__restrict __format, ...){
#ifdef DEBUG

    va_list arglist;
    va_start(arglist, __format);
    __android_log_vprint(ANDROID_LOG_INFO,"Remote_attestation",__format, arglist)   ;
    va_end( arglist );
#endif
    return 0;
}

int  real_ra_fflush (FILE *__stream){
    return 0;
}
#endif
