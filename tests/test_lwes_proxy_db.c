#include "../src/lwes_proxy_db.c"

#include <assert.h>

int main (void) 
{
  LwesProxyDb *db = lwes_proxy_db_new ();
  lwes_proxy_db_free (db);
  return 0;
}
