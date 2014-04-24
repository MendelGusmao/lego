#include <string.h>
#include <dlfcn.h>
#include "mongoose/mongoose.h"
#include "json-c/json.h"
#include "context.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#define CONTROLLER_PARAM "controller"
#define ACTION_PARAM "action"
#define CONTROLLERS_DIR "controllers"
#define HTTP_BAD_REQUEST           400
#define HTTP_NOT_FOUND             404
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_IMPLEMENTED       501

void invoke(struct mg_connection *conn) {
    void* handle;
    char* error;
    void (*init)();
    void (*method)(context *ctx);
    char controller[32];
    char controller_full[32];
    char action[32];
    char action_full[32];

    int has_ctrl = mg_get_var(conn, CONTROLLER_PARAM, controller, sizeof(controller));
    int has_action = mg_get_var(conn, ACTION_PARAM, action, sizeof(action));

    if (has_ctrl == -1) {
      mg_send_status(conn, HTTP_BAD_REQUEST);
      mg_printf_data(conn, "%s", "");
      return;
    }

    sprintf(controller_full, "%s/%s.ctrl", CONTROLLERS_DIR, controller);

    struct stat buffer;   
    if (stat(controller_full, &buffer) != 0) {
      mg_send_status(conn, HTTP_NOT_FOUND);
      mg_printf_data(conn, "%s", "");
      return;
    }

    handle = dlopen(controller_full, RTLD_LAZY);
    if (!handle) {
      mg_send_status(conn, HTTP_INTERNAL_SERVER_ERROR);
      mg_printf_data(conn, "%s", dlerror());
      return;
    }

    init = dlsym(handle, "init");
    if (dlerror() == NULL)  {
      (*init)();
    }

    if (has_action == -1) {
      sprintf(action_full, "%s", conn->request_method);
    } else {
      sprintf(action_full, "%s_%s", conn->request_method, action);
    }

    method = dlsym(handle, action_full);
    if (dlerror() != NULL)  {
      mg_send_status(conn, HTTP_NOT_IMPLEMENTED);
      mg_printf_data(conn, "%s", "");
      return;
    }

    context ctx;
    (*method)(&ctx);

    mg_send_status(conn, ctx.status);
    mg_send_header(conn, "Content-Type", ctx.type);
    mg_printf_data(conn, "%s", ctx.content);

    dlclose(handle);
}

static int event_handler(struct mg_connection *conn, enum mg_event ev) {
  if (ev == MG_AUTH) {
    return MG_FALSE;
  } else if (ev == MG_REQUEST && !strcmp(conn->uri, "/gateway")) {
    invoke(conn);
    return MG_TRUE;
  } else {
    return MG_FALSE;
  }
}

int main(void) {
  struct mg_server *server = mg_create_server(NULL, event_handler);
  mg_set_option(server, "document_root", ".");
  mg_set_option(server, "listening_port", "60000");

  for (;;) {
    mg_poll_server(server, 1000);
  }

  mg_destroy_server(&server);

  return 0;
}

