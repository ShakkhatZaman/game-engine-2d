#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define ERROR_RETURN(R, ...) fprintf(stderr, __VA_ARGS__); return R
#define ERROR_EXIT(...) fprintf(stderr, __VA_ARGS__)
#define ERROR_EXIT_PROGRAM(...) fprintf(stderr, __VA_ARGS__); exit(1)
#define ASSERT_EXIT(exp, ...) if (!(exp)) {fprintf(stderr, __VA_ARGS__); exit(1);}
#define ASSERT_RETURN(exp, R, ...) if (!(exp)) {fprintf(stderr, __VA_ARGS__); return R;}

#endif // !UTILS_H
