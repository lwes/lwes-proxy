/*
 * Compile with:
 * cc -I/usr/local/include -o signal-test \
 *   signal-test.c -L/usr/local/lib -levent
 */

#include <assert.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <event.h>
#include <lwes.h>

#include "lwes_proxy_config.h"

int called = 0;
struct lwes_net_connection **outputs;
//LWES_BYTE buffer[MAX_MSG_SIZE][10000];
//size_t buffer_index = 0;

unsigned short output_current = 0;
unsigned short output_count = 0;
unsigned short output_index = 0;

static void
signal_cb(int fd, short event, void *arg)
{
  struct event *signal = arg;
  struct event_base *base = event_get_base (signal);
  (void)fd; /* appease -Wall -Werror */
  (void)event; /* appease -Wall -Werror */ 

  printf("%s: got signal %d\n", __func__, event_get_signal(signal));

  event_del(signal);
  event_base_loopexit (base, NULL);
//  called++;
/*  event_loopbreak();*/
}

static void
sigusr1_cb(int fd, short event, void *arg)
{
  struct event *signal = arg;
  (void)fd; /* appease -Wall -Werror */
  (void)event; /* appease -Wall -Werror */ 

  printf("%s: got signal %d\n", __func__, event_get_signal(signal));
}

//static void
//send_event (int fd, short event, void *arg)
//{
//  struct lwes_net_connection *connection = (struct lwes_net_connection *)arg;
//  (void)fd; /* appease -Wall -Werror */
//  (void)event; /* appease -Wall -Werror */ 
//}

static void
receive_event (int fd, short event, void *arg)
{
  struct lwes_net_connection *connection = (struct lwes_net_connection *)arg;
  LWES_BYTE buffer[MAX_MSG_SIZE];
  int n = lwes_net_recv_bytes (connection, buffer, MAX_MSG_SIZE);
  (void)fd; /* appease -Wall -Werror */
  (void)event; /* appease -Wall -Werror */ 

  if (n < 0)
    {
      printf ("ERROR - receiving bytes\n");
    }
  else
    {
      LWES_IP_ADDR  listener_ip  = connection->mcast_addr.sin_addr;
      LWES_U_INT_16 listener_port= ntohs (connection->mcast_addr.sin_port);
      LWES_IP_ADDR  sender_ip    = connection->sender_ip_addr.sin_addr;
      LWES_U_INT_16 sender_port  = ntohs (connection->sender_ip_addr.sin_port);

      struct lwes_net_connection *output = outputs[output_current];
      LWES_IP_ADDR  sendto_ip   = output->mcast_addr.sin_addr;
      LWES_U_INT_16 sendto_port = ntohs (output->mcast_addr.sin_port);
      LWES_INT_64   receipt_time = currentTimeMillisLongLong();
      size_t new_size = n;
      printf ("%s:%d received %d bytes from %s:%d send to %d : %s:%d\n",
              inet_ntoa (listener_ip), listener_port, (int)new_size,
              inet_ntoa (sender_ip), sender_port,
              output_current, inet_ntoa (sendto_ip), sendto_port);
      assert (0 == lwes_event_add_headers (buffer, MAX_MSG_SIZE, &new_size, receipt_time, sender_ip, sender_port));
      assert ((int)new_size == lwes_net_send_bytes (output, buffer, new_size));
      output_current = ( output_current + 1 ) % output_count;
    }
}

static const char help[] =
  "lwes-proxy [options]"                                               "\n"
  ""                                                                   "\n"
  "  where options are:"                                               "\n"
  ""                                                                   "\n"
  "    -f <config file> "                                              "\n"
  "       The config file to use"                                      "\n"
  ""                                                                   "\n"
  "    -c"                                                             "\n"
  "       Parse config only"                                           "\n"
  ""                                                                   "\n"
  ""                                                                   "\n"
  "    -h"                                                             "\n"
  "         show this message"                                         "\n"
  ""                                                                   "\n"
  "  arguments are specified as -option value or -optionvalue"         "\n"
  ""                                                                   "\n";

#define MAX_WORDS 10

static struct lwes_net_connection *
handle_connection_arg (const char *arg)
{
  const char *sep = ":";
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  char *buffer;
  char *tofree;
  int count = 0;
  struct lwes_net_connection *connection = NULL;
  int i;

  for (i = 0 ; i < MAX_WORDS; i++)
    {
      words[i] = empty;
    }

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return NULL;
    }
  /* get all the words between the ':'s */
  while ((word = strsep (&buffer, sep)) && count < MAX_WORDS)
    {
      words[count++]=word;
    }
  if (count < 3 || count > 4)
    {
      fprintf (stderr, "ERROR: lwes transports require 3 or 4 parts\n"
                       "       <iface>:<ip>:<port>\n"
                       "       <iface>:<ip>:<port>:<ttl>\n");
    }
  else
    {
      const char *iface = NULL;
      const char *ip = NULL;
      int port = -1;
      int ttl = 3;
      if (strcmp (words[0], "") != 0)
        {
          iface = words[0];
        }
      if (strcmp (words[1], "") != 0)
        {
          ip = words[1];
          if (strcmp (words[2], "") != 0)
            {
              port = atoi (words[2]);
              if (strcmp (words[3], "") != 0)
                {
                  ttl = atoi (words[3]);
                  if (ttl < 0 || ttl > 32)
                    {
                      fprintf (stderr, "WARNING: ttl must be between 0 "
                               "and 32, defaulting to 3\n");
                      ttl = 3;
                    }
                }
              // create here
              printf ("opening connection %s:%s:%d:%d\n",
                      (iface!=NULL?iface:""),
                      ip, port, ttl);
              connection =
                (struct lwes_net_connection *)malloc (
                  sizeof(struct lwes_net_connection));
              assert (0 == lwes_net_open (connection, ip, iface, port));
              assert (0 == lwes_net_set_ttl (connection, ttl));
            }
          else
            {
              fprintf (stderr,
                       "ERROR: lwes transport requires "
                       "non-empty port\n");
            }
        }
      else
        {
          fprintf (stderr,
                   "ERROR: lwes transport requires non-empty ip\n");
        }
    }

  free (tofree);
  return connection;
}

int
main (int  argc,
      char *argv[])
{
  struct event signal_int;
  struct event signal_usr1;
//  struct event lwes_listener_event1;
//  struct event lwes_listener_event2;
//  struct lwes_net_connection connection1;
//  struct lwes_net_connection connection2;
//  int connection_fd1;
//  int connection_fd2;
  const char *config_file = NULL;
  int parse_config_only = 0;
  const char *args = "o:i:f:ch";
  unsigned short input_count = 0;
  unsigned short input_index = 0;
  struct lwes_net_connection **inputs;
  struct event **lwes_listeners;
  struct event_base *base;

  /* turn off error messages, I'll handle them */
  opterr = 0;
  while (1)
    {
      char c = getopt (argc, argv, args);
      if (c == -1)
        {
          break;
        }
      switch (c)
        {
          case 'f':
          case 'c':
            break;
          case 'i':
            input_count++;
            break;
          case 'o':
            output_count++;
            break;
          case 'h':
            fprintf (stderr, "%s", help);
            return 1;
          default:
            fprintf (stderr, "error: unrecognized command line option -%c\n",
                     optopt);
            return 1;
        }
    }
  printf ("found %d inputs and %d outputs\n",input_count, output_count);
  inputs = (struct lwes_net_connection **)
    malloc(input_count * sizeof(struct lwes_net_connection *));
  lwes_listeners = (struct event **)
    malloc(input_count * sizeof(struct event *));
  outputs = (struct lwes_net_connection **)
    malloc(output_count * sizeof(struct lwes_net_connection *));

  /* reset the args list and go through again to actually configure things
   */
  optind = 1;
  while (1)
    {
      char c = getopt (argc, argv, args);
      if (c == -1)
        {
          break;
        }
      switch (c)
        {
          case 'f':
            config_file = optarg;
            break;
          case 'c':
            parse_config_only = 1;
            break;
          case 'i':
            inputs[input_index] = handle_connection_arg (optarg);
            input_index++;
            break;
          case 'o':
            outputs[output_index] = handle_connection_arg (optarg);
            output_index++;
            break;
          case 'h':
            fprintf (stderr, "%s", help);
            return 1;
          default:
            fprintf (stderr, "error: unrecognized command line option -%c\n",
                     optopt);
            return 1;
        }
    }
  if (config_file == NULL)
    {
      fprintf (stderr, "error: must specify a config file with -f\n");
      return 1;
    }

  if (parse_config_only)
    {
      return lwes_proxy_config_parse_file (config_file);
    }

  /* Initialize the event library */
  base = event_base_new();

  /* Initalize signal handlers */
  event_assign(&signal_int, base, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb,
               &signal_int);
  event_add(&signal_int, NULL);

  event_assign(&signal_usr1, base, SIGUSR1, EV_SIGNAL|EV_PERSIST, sigusr1_cb,
               &signal_usr1);
  event_add(&signal_usr1, NULL);

  /* Initialize listener handlers */
  {
    int i = 0;
    for (i = 0 ; i < input_count; i++)
      {
        struct event *e;
        struct lwes_net_connection *c = inputs[i];
        int fd = lwes_net_get_sock_fd (c);
        int flags = fcntl (fd, F_GETFL, 0);
        if (flags < 0)
          {
            fprintf (stderr, "ERROR: cannot get socket options\n");
          }
        if (fcntl (fd, F_SETFL, flags | O_NONBLOCK) < 0)
          {
            fprintf (stderr, "ERROR: cannot set socket options\n");
          }
        assert (0 == lwes_net_recv_bind (c));
        e = event_new (base, fd, EV_READ | EV_PERSIST, receive_event, c);
        event_add (e, NULL);
      }
  }
  event_base_dispatch(base);
  event_base_free(base);
  return (0);

#if 0
  assert (0 == lwes_net_open (&connection1, "0.0.0.0", NULL, 20402));
  connection_fd1 = lwes_net_get_sock_fd (&connection1);

  /* Start : Set flags in non-blocking mode */
  int flags1 = fcntl (connection_fd1, F_GETFL, 0);
  if (flags1 < 0)
  {
    printf("ERROR - cannot set socket options\n");
  }
  if (fcntl (connection_fd1, F_SETFL, flags1 | O_NONBLOCK) < 0)
  {
    printf("ERROR - cannot set socket options\n");
  }
  assert (0 == lwes_net_recv_bind (&connection1));

  assert (0 == lwes_net_open (&connection2, "0.0.0.0", NULL, 20502));
  connection_fd2 = lwes_net_get_sock_fd (&connection2);

  /* Start : Set flags in non-blocking mode */
  int flags2 = fcntl (connection_fd2, F_GETFL, 0);
  if (flags2 < 0)
  {
    printf("ERROR - cannot set socket options\n");
  }
  if (fcntl (connection_fd2, F_SETFL, flags2 | O_NONBLOCK) < 0)
  {
    printf("ERROR - cannot set socket options\n");
  }
  assert (0 == lwes_net_recv_bind (&connection2));

  /* Initalize the event library */
  event_init();

  /* Initalize signal handlers */
  event_set (&signal_int, SIGINT, EV_SIGNAL|EV_PERSIST, signal_cb, &signal_int);
  event_add(&signal_int, NULL);

  event_set (&lwes_listener_event1, connection_fd1,
             EV_READ | EV_PERSIST, receive_event,
             &connection1);
  event_add (&lwes_listener_event1, NULL);

  event_set (&lwes_listener_event2, connection_fd2,
             EV_READ | EV_PERSIST, receive_event,
             &connection2);
  event_add (&lwes_listener_event2, NULL);

  event_dispatch();

  return (0);

#endif
}

