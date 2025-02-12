#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "bs.h"
#include "kv.h"
#include "server.h"
#include "template.h"

Server *server = NULL;

static void sig(int signum)
{
  if (server)
    serverDel(server);

  fprintf(stdout, "\n[%d] Bye!\n", signum);
  exit(0);
}

static Response *notFound(Request *);
static Response *homePage(Request *);
static Response *apiHandle(Request *);

int main(int argc, char **argv)
{
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
  Server *server = serverNew(server_port);
  serverAddHandler(server, notFound);
  serverAddStaticHandler(server);
  serverAddHandler(server, homePage);
  serverAddHandler(server, apiHandle);

  serverServe(server);
  return 0;
}

static Response *notFound(Request *req)
{
  (void)req;
  Response *response = responseNew();
  Template *template = templateNew("templates/404.html");
  templateSet(template, "subtitle", "404 Not Found");
  responseSetStatus(response, NOT_FOUND);
  responseSetBody(response, templateRender(template));
  templateDel(template);
  return response;
}

static Response *homePage(Request *req) {
  EXACT_ROUTE(req, "/");
  Response *response = responseNew();
  Template *template = templateNew("templates/home.html");
  responseSetStatus(response, OK);
  responseSetBody(response, templateRender(template));
  templateDel(template);
  return response;
}

static Response *apiHandle(Request *req) {
  EXACT_ROUTE(req, "/api");
  Response *response = responseNew();
  responseSetStatus(response, OK);
  responseAddHeader(response, "Content-Type", "application/json");
  char *js = bsNew("{\"data\":\"Hi there\"}");
  responseSetBody(response, js);
  return response;
}
