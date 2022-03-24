#include <assert.h>

#include "../include/udx.h"

uv_loop_t loop;

udx_t asock;
udx_stream_t astream;

udx_t bsock;
udx_stream_t bstream;

udx_stream_send_t req;

int send_called = FALSE;
int recv_called = FALSE;


void
on_send (udx_stream_send_t *r, int status) {
  assert(&req == r);
  assert(status == 0);

  send_called = TRUE;
}

void
on_recv (udx_stream_t *handle, ssize_t read_len, const uv_buf_t *buf) {
  assert(buf->len == 5);
  assert(buf->len == read_len);
  assert(memcmp(buf->base, "hello", 5) == 0);

  uv_stop(&loop);

  recv_called = TRUE;
}

int
main () {
  int e;

  uv_loop_init(&loop);

  e = udx_init(&loop, &asock);
  assert(e == 0);

  e = udx_init(&loop, &bsock);
  assert(e == 0);

  struct sockaddr_in baddr;
  uv_ip4_addr("127.0.0.1", 8082, &baddr);
  e = udx_bind(&bsock, (struct sockaddr *) &baddr);
  assert(e == 0);

  struct sockaddr_in aaddr;
  uv_ip4_addr("127.0.0.1", 8081, &aaddr);
  e = udx_bind(&asock, (struct sockaddr *) &aaddr);
  assert(e == 0);

  e = udx_stream_init(&loop, &astream, 1);
  assert(e == 0);

  e = udx_stream_init(&loop, &bstream, 2);
  assert(e == 0);

  e = udx_stream_connect(&astream, &asock, 2, (struct sockaddr *) &baddr, NULL);
  assert(e == 0);

  e = udx_stream_connect(&bstream, &bsock, 1, (struct sockaddr *) &aaddr, NULL);
  assert(e == 0);

  e = udx_stream_recv_start(&astream, on_recv);
  assert(e == 0);

  uv_buf_t buf = uv_buf_init("hello", 5);
  udx_stream_send(&req, &bstream, &buf, 1, on_send);

  uv_run(&loop, UV_RUN_DEFAULT);

  assert(send_called && recv_called);
}
