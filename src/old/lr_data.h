#ifndef __LWES_PROXY_DATA_H
#define __LWES_PROXY_DATA_H


#include <lwes.h>

#ifdef __cplusplus
extern "C" {
#endif

struct lwes_connections {
  struct lwes_net_connection frontend[128];
  struct lwes_net_connection backend[1024];
};

struct backend {
  int index;
  struct backend *next;
};

struct lwes_proxy_fe {
  char *name;
  int index;
  struct backend *default_backends;
};

struct pool {
  char *name;
  struct pool *next;
};

struct lwes_proxy_be {
  char *name;
  int index;
  struct pool **pools;
};

struct filters {
  int *in;
  int *out;
};

#ifdef __cplusplus
}
#endif

#endif /* __LWES_PROXY_DATA_H */
