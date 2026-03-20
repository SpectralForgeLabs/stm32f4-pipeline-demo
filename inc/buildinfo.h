#ifndef __INCbuildinfo_h
#define __INCbuildinfo_h

#ifdef __cplusplus
extern "C" {
#endif /** __cplusplus */


#include <stdint.h>

typedef struct __attribute__((packed)) {
  char git_sha[41];      /* 40 hex chars + null */
  char branch[32];
  char build_number[8];
} build_info_t;

extern const build_info_t build_info;

#ifdef __cplusplus
}
#endif /** __cplusplus */
#endif /** __INCbuildinfo_h */