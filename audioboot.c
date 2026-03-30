/* Audioboot - Neo Sahadeo 2025
 * MIT LICENSE
 * */

#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

#define PROGRAM_NAME "_Audioboot"
#define THREAD_NAME "audiobootthread"  // max 16 chars longs
#define CARLA_PROCESS "carla-jack-multi"
#define SINK_NAME "Default-Sink"
#define SOURCE_NAME "Virtual-Source"
#define DEFAULT_TIMEOUT 5

#define LOAD_SINK(name) "pactl load-module module-null-sink sink_name=" name
#define LOAD_SOURCE(name) \
  "pactl load-module module-pipe-source source_name=" name

#define UNLOAD_SINK "pactl unload-module module-null-sink"
#define UNLOAD_SOURCE "pactl unload-module module-pipe-source"

#define START_CARLA                                                      \
  "nohup carla-jack-multi /home/neosahadeo/.audio/multiConf.carxp -n > " \
  "/dev/null 2>&1 &"
#define START_CARLA_SHOW                                              \
  "nohup carla-jack-multi /home/neosahadeo/.audio/multiConf.carxp > " \
  "/dev/null 2>&1 &"

bool parse_toggle_flag(int argc, char** argv, const char* flag_name) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], flag_name) == 0)
      return true;
  }
  return false;
}

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
#ifdef NDEBUG
    printf("Checking if Carla is running: %d\n", id);
#endif
    if (id <= 0) {
#ifdef NDEBUG
      printf("Starting Carla\n");
#endif
      system(START_CARLA);
    }

    sleep(DEFAULT_TIMEOUT);
  }
  return NULL;
}

int main(int argc, char** argv) {
  const bool f_show = parse_toggle_flag(argc, argv, "show");
  const bool f_auto = parse_toggle_flag(argc, argv, "auto");
  const bool f_kill = parse_toggle_flag(argc, argv, "kill");

  int selfid = get_id(PROGRAM_NAME);
  if (selfid > 0) {
#ifdef NDEBUG
    printf("Killing %s: %d\n", PROGRAM_NAME, selfid);
#endif
    kill(selfid, SIGKILL);
  }

  int carlaid = get_id(CARLA_PROCESS);
  if (carlaid > 0) {
#ifdef NDEBUG
    printf("Killing Carla: %d\n", carlaid);
#endif
    kill(carlaid, SIGKILL);
  }

  if (f_kill) {
    unload_sink_source();
    return 0;
  }

  /* Reset sinks on load */
  reset_sink_source();

  /* Generate a thread if auto is selected */
  if (f_auto) {
#ifdef NDEBUG
#endif
    printf("Starting Auto Carla\n");
    strncpy(argv[0], PROGRAM_NAME, strlen(argv[0]));
    prctl(PR_SET_NAME, (unsigned long)PROGRAM_NAME, 0, 0, 0);

    pthread_t thread = create_thread(audio_start_auto);
    pthread_setname_np(thread, THREAD_NAME);
    pthread_detach(thread);
    pthread_exit(0);
  }

#ifdef NDEBUG
  printf("Starting Carla\n");
#endif
  if (f_show) {
    system(START_CARLA_SHOW);
  } else {
    system(START_CARLA);
  }

  return 0;
}
