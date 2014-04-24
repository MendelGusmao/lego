#include "json-c/json.h"
#include "context.h"

void init() {
	//
}

void GET(context *ctx) {
  json_object *jobj = json_object_new_object();
  json_object_object_add(jobj, "message", json_object_new_string("Hello, world!"));

  ctx->status = 200;
  ctx->type = "application/json";
  ctx->content = json_object_to_json_string(jobj);
}

void GET_foobar(context *ctx) {
  json_object *jobj = json_object_new_object();
  json_object *jstring = json_object_new_string("Hello, foobar!");

  json_object_object_add(jobj, "message", jstring);

  ctx->status = 200;
  ctx->type = "application/json";
  ctx->content = json_object_to_json_string(jobj);
}
