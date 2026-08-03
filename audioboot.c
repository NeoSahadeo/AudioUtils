/* Audioboot - Neo Sahadeo 2026
 * MIT LICENSE
 * */

#define STB_ARGPARSE_IMPLEMENTATION
#include "stb_argparse.h"

#define __USE_GNU
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#define PROGRAM_NAME "_Audioboot"
#define THREAD_NAME "audiobootthread"  // max 16 chars longs
#define CARLA_PROCESS "carla-jack-multi"
#define SINK_NAME "Default-Sink"
#define SOURCE_NAME "Virtual-Source"

#define LOAD_SINK(name) "pactl load-module module-null-sink sink_name=" name
#define LOAD_SOURCE(name)                                                \
  "pactl load-module module-null-sink media.class=Audio/Source/Virtual " \
  "sink_name=" name

#define UNLOAD_SINK "pactl unload-module module-null-sink"
#define UNLOAD_SOURCE "pactl unload-module module-pipe-source"

int timeout = 5;
char command_buffer[1024] = {0};

int pactl_search(const char* name) {
  char buffer[1024];
  sprintf(buffer, "pactl list | grep %s -c", name);
  FILE* pipe = popen(buffer, "r");
  if (!pipe) {
    perror("popen failed");
    return -1;
  }
  fgets(buffer, sizeof(buffer), pipe);
  pclose(pipe);
  return atoi(buffer);
}

void load_sink_source() {
  system(LOAD_SOURCE(SOURCE_NAME));  // must be loaded first
  system(LOAD_SINK(SINK_NAME));
}

void unload_sink_source() {
  if (pactl_search(SINK_NAME) > 0) {
    system(UNLOAD_SINK);
  }
  if (pactl_search(SOURCE_NAME) > 0) {
    system(UNLOAD_SOURCE);
  }
}

void reset_sink_source() {
  unload_sink_source();
  load_sink_source();
}

int get_id(const char* query) {
  char buffer[1024];
  sprintf(buffer, "pgrep -f %s |  grep -v \"^$$\"", query);
  FILE* pipe = popen(buffer, "r");
  if (!pipe) {
    perror("popen failed");
    return -1;
  }
  fgets(buffer, sizeof(buffer), pipe);
  pclose(pipe);
  return atoi(buffer);
}

pthread_t create_thread(void* (*func)(void*)) {
  pthread_t thread;
  int result = pthread_create(&thread, NULL, func, NULL);
  if (result != 0) {
    perror("pthread_create");
    return 1;
  }
  return thread;
}

void* audio_start_auto() {
  for (;;) {
    int id = get_id(CARLA_PROCESS);
    if (id <= 0) {
      system(command_buffer);
    }

    sleep(timeout);
  }
  return NULL;
}

void kill_zone() {
  int selfid = get_id(PROGRAM_NAME);
  if (selfid > 0) {
    kill(selfid, SIGKILL);
  }

  int carlaid = get_id(CARLA_PROCESS);
  if (carlaid > 0) {
    kill(carlaid, SIGKILL);
  }
}

int main(int argc, char** argv) {
  const char* route_file = 0;
  char carla_show_flag[2] = "-n";
  bool display = false;
  bool killaudioboot = false;
  bool autostart = true;

  argument_parser_t parser;
  argparse_init(&parser, argc, argv, "Program description", "Epilog text");

  argparse_arg_t args[] = {
      ARGPARSE_POSITIONAL(STRING, "--conf", &route_file,
                          "multijack config for Carla"),
      ARGPARSE_TOGGLE('d', "--display", &display,
                      "display carla, default is false"),
      ARGPARSE_TOGGLE('a', "--auto", &autostart,
                      "auto restart carla if it closes, default is true"),
      ARGPARSE_TOGGLE('k', "--kill", &killaudioboot,
                      "kill carla and audioboot"),
      ARGPARSE_OPTION(
          INT, 's', "--sleep", &timeout,
          "how often audioboot checks for a crash, default is 5 seconds"),
  };

  argparse_add_arguments(&parser, args, 5);
  argparse_parse_args(&parser);

  if (killaudioboot) {
    kill_zone();
    unload_sink_source();
    return 0;
  }

  if (route_file == 0) {
    argparse_print_help(&parser);
    return 1;
  }

  if (display) {
    carla_show_flag[0] = ' ';
    carla_show_flag[1] = ' ';
  }

  sprintf(command_buffer, "nohup carla-jack-multi %s %s > /dev/null 2>&1 &",
          route_file, carla_show_flag);

  kill_zone();
  reset_sink_source();

  if (autostart) {
    strncpy(argv[0], PROGRAM_NAME, strlen(argv[0]));
    prctl(PR_SET_NAME, (unsigned long)PROGRAM_NAME, 0, 0, 0);

    pthread_t thread = create_thread(audio_start_auto);
    pthread_setname_np(thread, THREAD_NAME);
    pthread_detach(thread);
    pthread_exit(0);
  }

  system(command_buffer);

  return 0;
}
