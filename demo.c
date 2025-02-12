#ifdef _WIN32
#include <windows.h>
#include "resource.h"
#endif
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "bs.h"
#include "kv.h"
#include "server.h"
#include "template.h"

Server *server = NULL;

#ifndef _WIN32
static void sig(int signum)
{
  if (server)
    serverDel(server);

  fprintf(stdout, "\n[%d] Bye!\n", signum);
  exit(0);
}
#else
static LPCTSTR g_szClassName = TEXT("iMusicID");
static HWND g_hInstance;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

static Response *notFound(Request *);
static Response *homePage(Request *);
static Response *apiHandle(Request *);

static Response *playlistHandle(Request *);
static Response *songHandle(Request *);

#ifndef _WIN32
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

  serverAddHandler(server, playlistHandle);
  serverAddHandler(server, songHandle);

  serverServe(server);
  return 0;
}
#else
DWORD WINAPI serverMain(LPVOID lParam) {
  (void)lParam;
  uint16_t server_port = 8080;
  Server *server = serverNew(server_port);
  serverAddHandler(server, notFound);
  serverAddStaticHandler(server);
  serverAddHandler(server, homePage);
  serverAddHandler(server, apiHandle);

  serverAddHandler(server, playlistHandle);
  serverAddHandler(server, songHandle);

  serverServe(server);
  return 0;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  (void)hPrevInstance;
  (void)lpCmdLine;
  SetErrorMode(SEM_FAILCRITICALERRORS);
  g_hInstance = FindWindow(g_szClassName, NULL);
  if (g_hInstance != NULL) {
    BringWindowToTop(g_hInstance);
    ShowWindow(g_hInstance, nCmdShow);
    UpdateWindow(g_hInstance);
    return 0;
  }
  WNDCLASSEX wc;
  MSG Msg;
  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.style         = 0;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_SHARED);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = g_szClassName;
  wc.hIconSm       = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

  if(!RegisterClassEx(&wc)) {
    MessageBox(NULL, TEXT("Không thể khởi tạo được ứng dụng!"), TEXT("Lỗi !"), MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  g_hInstance = CreateWindowEx(
    WS_EX_LAYERED,
    g_szClassName,
    (LPCTSTR)"Hello World",
    WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
    GetSystemMetrics(SM_CXSCREEN)/2-160,
    GetSystemMetrics(SM_CYSCREEN)/2-120,
    400, 250,
    NULL, NULL, hInstance, NULL
  );
  if(g_hInstance == NULL) {
    MessageBox(NULL, TEXT("Không thể khởi tạo được ứng dụng!"), TEXT("Lỗi!"), MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  ShowWindow(g_hInstance, nCmdShow);
  UpdateWindow(g_hInstance);
  while(GetMessage(&Msg, NULL, 0, 0) > 0) {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }
  return Msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch(msg) {
    case WM_CLOSE: {
      DestroyWindow(hwnd);
    } break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}
#endif
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

  templateSet(template, "ohcarol", "let us know");

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

static Response *playlistHandle(Request *req) {
  EXACT_ROUTE(req, "/playlist");
  Response *response = responseNew();
  responseSetStatus(response, OK);
  responseAddHeader(response, "Content-Type", "application/json");
  char *js = bsNew("{\"data\":\"Hi from playlist\"}");
  responseSetBody(response, js);
  return response;
}

static Response *songHandle(Request *req) {
  EXACT_ROUTE(req, "/song");
  Response *response = responseNew();
  responseSetStatus(response, OK);
  responseAddHeader(response, "Content-Type", "application/json");
  char *js = bsNew("{\"data\":\"Hi from song\"}");
  responseSetBody(response, js);
  return response;
}
