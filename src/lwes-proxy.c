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

static void
signal_cb(int fd, short event, void *arg)
{
  struct event *signal = arg;
  (void)fd; /* appease -Wall -Werror */
  (void)event; /* appease -Wall -Werror */ 

  printf("%s: got signal %d\n", __func__, EVENT_SIGNAL(signal));

  event_del(signal);
  event_loopbreak();
}

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
      printf ("%s:%d received %d bytes from %s:%d\n",
              inet_ntoa (listener_ip), listener_port, n,
              inet_ntoa (sender_ip), sender_port);
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

int
main (int argc, char **argv)
{
  struct event signal_int;
  struct event lwes_listener_event1;
  struct event lwes_listener_event2;
  struct lwes_net_connection connection1;
  struct lwes_net_connection connection2;
  int connection_fd1;
  int connection_fd2;
  const char *config_file = NULL;
  int parse_config_only = 0;
  (void)argc;
  (void)argv;

  opterr = 0;
  while (1)
    {
      char c = getopt (argc, argv, "f:ch");
      if (c == -1)
        break;
      switch (c)
        {
          case 'f':
            config_file = optarg;
            break;
          case 'c':
            parse_config_only = 1;
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
}

