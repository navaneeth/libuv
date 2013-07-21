/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void worker(void* arg) {
  int i;
  char* string;
  uv_chan_t* chan = arg;

  for (i = 0; i < 10; i++) {
    string = malloc(sizeof(char) * 2);
    sprintf(string, "%d", i);
    uv_chan_send (chan, string);
    uv_sleep (10);
  }

  uv_chan_send (chan, NULL);
}

TEST_IMPL(message_passing_with_channels) {
  int threads_exited = 0, messages_received = 0;
  char* message;
  uv_chan_t chan;
  uv_thread_t thread1, thread2;

  ASSERT(0 == uv_chan_init (&chan));
  ASSERT(0 == uv_thread_create(&thread1, worker, &chan));
  ASSERT(0 == uv_thread_create(&thread2, worker, &chan));

  while (threads_exited < 2) {
    message = uv_chan_receive(&chan);
    if (message == NULL)
      ++threads_exited;
    else
      ++messages_received;
  }

  ASSERT(20 == messages_received);

  uv_chan_destroy (&chan);

  return 0;
}

static const char* message = "Done";

static void worker1(void* arg) {
  uv_chan_t* chan = arg;
  uv_sleep (1000);
  uv_chan_send (chan, message);
}

TEST_IMPL(uv_chan_receive_should_block_when_no_data_available) {
  int data_received = 0;
  uv_chan_t chan;
  uv_thread_t thread;
  void* received;

  ASSERT(0 == uv_chan_init (&chan));
  ASSERT(0 == uv_thread_create (&thread, worker1, &chan));

  received = uv_chan_receive (&chan);
  ASSERT(0 == strcmp(message, received));

  uv_chan_destroy (&chan);

  return 0;
}

