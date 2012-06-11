#include <stdlib.h>

#include "lwes_proxy_db.h"

struct _LwesProxyDb {
  HashTable *frontend_to_index;
  ArrayList *frontends;
  HashTable *backend_to_index;
  ArrayList *backends;
};

LwesProxyDb *lwes_proxy_db_new (void)
{
  LwesProxyDb *db = (LwesProxyDb *)malloc (sizeof (LwesProxyDb));
  db->frontend_to_index = hash_table_new (int_hash, int_equal);
  db->frontends = arraylist_new (8);
  db->backend_to_index = hash_table_new (int_hash, int_equal);
  db->backends = arraylist_new (8);
  return db;
}

void lwes_proxy_db_free (LwesProxyDb *db)
{
  hash_table_free (db->frontend_to_index);
  arraylist_free (db->frontends);
  hash_table_free (db->backend_to_index);
  arraylist_free (db->backends);
  free (db);
}
