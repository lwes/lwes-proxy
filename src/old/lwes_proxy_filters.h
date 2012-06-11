#ifndef __LWES_PROXY_FILTERS_H
#define __LWES_PROXY_FILTERS_H

#include "trie.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lwes_proxy_filters {
  Trie *filters;
} LwesProxyFilters;

typedef struct lwes_proxy_filter {
  int *in;
  int *out;
} LwesProxyFilter;

typedef enum lwes_proxy_filter_type { IN, OUT } LwesProxyFilterType;

LwesProxyFilters *lwes_proxy_filters_new (void)
{
  LwesProxyFilters *filters;
  filters = (LwesProxyFilters *) malloc (sizeof (LwesProxyFilters));

  if (filters == NULL)
    {
      return NULL;
    }
  filters->filters = trie_new ();
  if (filters->filters == NULL)
    {
      free (filters);
      return NULL;
    }
  return filters;
}

void lwes_proxy_filters_free (LwesProxyFilters *filters)
{
  trie_free (filters->filters);
  free (filters);
}

#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)))

int lwes_proxy_filter_add (LwesProxyFilters *filters,
                           unsigned char* key,
                           int key_length,
                           LwesProxyFilterType type,
                           int t)
{
  LwesProxyFilter *filter =
    (LwesProxyFilter *) trie_lookup_binary (filters->filters, key, key_length);

  if ((TrieValue)filter == TRIE_NULL)
    {
      LwesProxyFilter *filter =
        (LwesProxyFilter *)malloc (sizeof (LwesProxyFilter));
      filter->in = NULL;
      filter->out = NULL;

      assert (filter != NULL);

      if (type == IN)
        {
          filter->in = (int *)malloc (sizeof (int));
          filter->in[0] = t;
        }
      else
        {
          filter->out = (int *)malloc (sizeof (int));
          filter->out[0] = t;
        }
      trie_insert_binary (filters->filters, key, key_length, (TrieValue)filter);
    }
  else
    {
      if (type == IN)
        {
          int *tmp = filter->in;
          int size = NUM_ELEM (size);
          filter->in = (int *)malloc (sizeof (int) * (size+1));
          memcpy (filter->in, tmp, size);
          filter->in[size] = t;
          free (tmp);
        }
      else
        {
          int *tmp = filter->out;
          int size = NUM_ELEM (size);
          filter->out = (int *)malloc (sizeof (int) * (size+1));
          memcpy (filter->out, tmp, size);
          filter->in[size] = t;
          free (tmp);
        }
    }
}


#ifdef __cplusplus
}
#endif

#endif /* __LWES_PROXY_FILTERS_H */
