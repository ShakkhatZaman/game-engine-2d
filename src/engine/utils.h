#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

#define ERROR_RETURN(R, ...) fprintf(stderr, __VA_ARGS__); return R
#define ERROR_EXIT(...) fprintf(stderr, __VA_ARGS__)
#define ERROR_EXIT_PROGRAM(...) fprintf(stderr, __VA_ARGS__); exit(1)

#ifdef _DEBUG_
#define ASSERT(exp, msg) if (!(exp)) {fprintf(stderr, "File: %s, Line: %d\n\t" msg "\n", __FILE__, __LINE__);}
#define ASSERT_EXIT(exp, msg) if (!(exp)) {fprintf(stderr, "File: %s, Line: %d\n\t" msg "\n", __FILE__, __LINE__); exit(1);}
#define ASSERT_RETURN(exp, R, msg) if (!(exp)) {fprintf(stderr, "File: %s, Line: %d\n\t" msg "\n", __FILE__, __LINE__); return R;}
#else
#define ASSERT(exp, msg) (void) 0;
#define ASSERT_EXIT(exp, msg) (void) 0;
#define ASSERT_RETURN(exp, R, msg) (void) 0;
#endif

#endif // !UTILS_H
