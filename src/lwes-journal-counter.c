#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <lwes.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RECEIPT_TIME_OFFSET (2)
#define SENDER_IP_OFFSET    (RECEIPT_TIME_OFFSET+8)
#define SENDER_PORT_OFFSET  (SENDER_IP_OFFSET+4)
#define SITE_ID_OFFSET      (SENDER_PORT_OFFSET+2)
#define EXTENSION_OFFSET    (SITE_ID_OFFSET+2)
#define EVENT_TYPE_OFFSET   (EXTENSION_OFFSET+4)
#define HEADER_LENGTH       (EVENT_TYPE_OFFSET)

struct ctxt {
  char key[100];
  char val[500];
};


static unsigned short header_uint16(const char* bytes) {
  return (unsigned short)ntohs(*((uint16_t*) bytes));
}

static unsigned int header_uint32(const char* bytes) {
  return (unsigned int)ntohl(*((unsigned int*) bytes));
}

static unsigned long long header_uint64(const char* bytes) {
  return (((unsigned long long)header_uint32(bytes))<<32) | header_uint32(bytes+4);
}

static unsigned short header_payload_length(const char* header)
{
  return (unsigned short)header_uint16(header);
}

static unsigned long long header_receipt_time(const char* header) {
  return (unsigned long long)header_uint64(header+RECEIPT_TIME_OFFSET);
}

int main(int argc, char **argv)
{
  gzFile file;
  const char *filename;
  const char *ctx_delim;
  char header[22];
  unsigned char buf[65535];
  if (argc == 2)
    {
      filename = argv[1];
      ctx_delim = "-";
    }
  else if (argc == 3)
    {
      filename = argv[1];
      ctx_delim = argv[2];
    }
  else
    {
      fprintf (stderr, "Usage: prog <journal> [<context delimiter> (defaults to '-') ]\n");
      exit (1);
    }
  file = gzopen (filename, "rb");
  while (gzread (file, header, 22) == 22) {
    unsigned short size = header_payload_length (header);
    long long timestamp = (long long)(header_receipt_time (header));
    long long seconds = (long long)(timestamp / 1000);
    if (gzread (file, buf, size) != size)
    {
      fprintf (stderr, "error reading\n");
      exit (1);
    }
    fprintf (stdout, "%lld\t%lld\t%u\n", seconds, timestamp, size);
  }
  gzclose (file);
  exit (0);
}
