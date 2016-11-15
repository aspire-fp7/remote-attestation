//
// Created by alessio on 18/03/16.
//

#ifndef SRC_RA_ANDROID_FEATURES_H
#define SRC_RA_ANDROID_FEATURES_H

#include <stdio.h>

    #ifdef DEBUG

        #ifdef __ANDROID

            #include <android/log.h>

            #define ra_fprintf  real_ra_fprintf
            #define ra_printf   real_ra_printf
            #define ra_fflush   real_ra_fflush

            int real_ra_fprintf(FILE *__restrict __stream, const char *__restrict __format, ...);
            int real_ra_printf(const char *__restrict __format, ...);
            int real_ra_fflush(FILE *__stream);

        #else

            #define ra_fprintf  fprintf
            #define ra_printf   printf
            #define ra_fflush   fflush

        #endif

    #else

        #define ra_fprintf(...)
        #define ra_printf(...)
        #define ra_fflush(...)

    #endif


#endif //SRC_RA_ANDROID_FEATURES_H
