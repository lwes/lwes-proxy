#ifndef __LWES_PROXY_DB_H
#define __LWES_PROXY_DB_H

#ifdef __cplusplus
extern "C" {
#endif

//#include <libcalg.h>

typedef struct _LwesProxyDb LwesProxyDb;

LwesProxyDb *lwes_proxy_db_new (void);

void lwes_proxy_db_free (LwesProxyDb *db);

#ifdef __cplusplus
}
#endif

#endif /* __LWES_PROXY_DB_H */
