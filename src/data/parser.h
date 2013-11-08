/*************************************************************************
  > File Name: parser.h
  > Copyright (C) 2013 Yue Wu<yuewu@outlook.com>
  > Created Time: Thu 07 Nov 2013 08:16:26 PM
  > Descriptions: public funtions to parse
 ************************************************************************/

#ifndef HEADER_PARSER
#define HEADER_PARSER
#include <cstdio>
#include <cmath>

namespace SOL{

    inline bool is_space(char* p){
        return (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r');
    }

    inline char* strip_line(char* p){
        while(is_space(p) == true)
            p++;
        return p;
    }

    //The following function is a home made strtoi
    inline int parseInt(char * p, char **end) {
        *end = p;
        p = strip_line(p);

        if (*p == '\0'){
            return 0;
        }
        int s = 1;
        if (*p == '+')p++;
        if (*p == '-') {
            s = -1; p++;
        }
        int acc = 0;
        while (*p >= '0' && *p <= '9')
            acc = acc * 10 + *p++ - '0';

        int num_dec = 0;
        if (*p == '.') {
            p++;
            while (*p >= '0' && *p <= '9') {
                acc = acc *10 + *p++ - '0' ;
                num_dec++;
            }
        }
        int exp_acc = 0;
        if(*p == 'e' || *p == 'E'){
            p++;
            if (*p == '+')p++;
            while (*p >= '0' && *p <= '9')
                exp_acc = exp_acc * 10 + *p++ - '0';

        }
        if (is_space(p)== true) {//easy case succeeded.
            exp_acc -= num_dec;
            if (exp_acc < 0)
                return 0;
            else
                acc *= (int)(powf(10.f,exp_acc));

            *end = p;
            return s * acc;
        }
        else {
            return 0;
        }
    }

    //The following function is a home made strtoi
    inline unsigned int parseUint(char * p, char **end) {
        *end = p;
        p = strip_line(p);

        if (*p == '\0'){
            return 0;
        }
        unsigned int acc = 0;
        while (*p >= '0' && *p <= '9')
            acc = acc * 10 + *p++ - '0';

        int num_dec = 0;
        if (*p == '.') {
            p++;
            while (*p >= '0' && *p <= '9') {
                acc = acc *10 + *p++ - '0' ;
                num_dec++;
            }
        }
        int exp_acc = 0;
        if(*p == 'e' || *p == 'E'){
            p++;
            if (*p == '+')p++;
            while (*p >= '0' && *p <= '9')
                exp_acc = exp_acc * 10 + *p++ - '0';
        }
        if (*p == ':') {//easy case succeeded.
            if (exp_acc < num_dec)
                return 0;
            else
                acc *= (unsigned int)(powf(10.f,exp_acc - num_dec));
            *end = ++p;
            return acc;
        }
        else {
            return 0;
        }
    }
    // The following function is a home made strtof. The
    // differences are :
    //  - much faster (around 50% but depends on the string to parse)
    //  - less error control, but utilised inside a very strict parser
    //    in charge of error detection.
    inline float parseFloat(char * p, char **end) {
        *end = p;
        p = strip_line(p);

        if (*p == '\0'){
            return 0;
        }
        int s = 1;
        if (*p == '+') p++;
        if (*p == '-') {
            s = -1; p++;
        }

        unsigned int acc = 0;
        while (*p >= '0' && *p <= '9')
            acc = acc * 10 + *p++ - '0';

        int num_dec = 0;
        if (*p == '.') {
            p++;
            while (*p >= '0' && *p <= '9') {
                acc = acc *10 + *p++ - '0' ;
                num_dec++;
            }
        }

        int exp_acc = 0;
        if(*p == 'e' || *p == 'E'){
            p++;
            int exp_s = 1;
            if (*p == '+') p++;
            if (*p == '-') {
                exp_s = -1; p++;
            }
            while (*p >= '0' && *p <= '9')
                exp_acc = exp_acc * 10 + *p++ - '0';
            exp_acc *= exp_s;
        }
        if (is_space(p) == true){//easy case succeeded.
            exp_acc -= num_dec;
            *end = p;
            return s * acc* powf(10.f,(float)exp_acc);
        }
        else
            return 0;
    }

}
#endif