typedef struct lwes_repeater_group {
  int id;
  enum filter;
   
} LwesRepeaterGroup;

/* 2 of 5 would be
 *
 * { 0, 2, 5, [c1, c2, c3, c4, c5] }
 * for i = s; i < (s + n) % m
typedef struct nofm_strategy {
  int s;
  int n;
  int m;
  void *channels;
} NofM_Strategy;

/* 2 use cases
 *
 * N of M strategy, like 2 of 5 can be represented with
 * j = 1
 * o = 1
 * channels = { { i = 0, n = 2, m = 5, channels = [c1, c2, c3, c4, c5 ] ] }
 *
 * while ( j > 0 )
 */
typedef struct oofnofm_strategy {
  int j;
  int o;
  NofM_Strategy *channels;
} OofNofM_Strategy;


int main (int   argc,
          char *argv[])
{
  Strategy s1; 

}
