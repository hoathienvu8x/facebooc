#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "bs.h"
#include "kv.h"
#include "server.h"

Server *server = NULL;

static void sig(int signum);
static Response *home(Request *);
static Response *ws_stream(Request *);
static Response *notFound(Request *);

int main(int argc, char **argv) {
  if (signal(SIGINT, sig) == SIG_ERR || signal(SIGTERM, sig) == SIG_ERR) {
    fprintf(stderr, "error: failed to bind signal handler\n");
    return 1;
  }

  srand(time(NULL));

  uint16_t server_port = 8080;
  if (argc > 1) {
    if (sscanf(argv[1], "%hu", &server_port) == 0) {
      fprintf(stderr,
        "error: invalid command line argument, using default port "
        "8080.\n");
      server_port = 8080;
    }
  }
  server = serverNew(server_port);
  if (!server) return -1;
  serverAddHandler(server, notFound);
  serverAddHandler(server, home);
  serverAddHandler(server, ws_stream);

  serverServe(server);

  return 0;
}

/* handlers */
static void sig(int signum) {
  if (server)
    serverDel(server);

  fprintf(stdout, "\n[%d] Bye!\n", signum);
  exit(0);
}

static Response *home(Request *req) {
  EXACT_ROUTE(req, "/");

  Response *response = responseNew();
  responseSetStatus(response, OK);

  responseSetBody(response, bsNew("Hello world!"));

  return response;
}

static Response *notFound(Request *req) {
  (void)req;
  Response *response = responseNew();
  responseSetStatus(response, NOT_FOUND);
  responseSetBody(response, bsNew("404 Not Found"));

  return response;
}

static Response *ws_stream(Request *req) {
  Response *response;
  EXACT_ROUTE(req, "/stream");
  if (req->method != GET || !requestIsUpgrade(req)) {
    goto bad_req;
  }

bad_req:
  response = responseNew();
  responseSetStatus(response, FORBIDDEN);
  responseSetBody(response, bsNew("Forbidden!"));

  return response;
}
