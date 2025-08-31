/* Deadsec - Neo Sahadeo 2025
 * MIT LICENSE
 * */

#include <argp.h>
#include <argz.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

const char *argp_program_version = "v1.0.0";
const char *argp_program_bug_address = "neosahadeo+deadsec@protonmail.com";

int parse_opt(int key, char *arg, struct argp_state *state);

struct argp_option options[] = {
    {"link", 'l', "SOURCE SINK [-d]", 0,
     "Link sources to a sink. Specify -d to disonnect instead of connect"},
    {0, 'd', 0, OPTION_HIDDEN},
    {0, 0, 0, 0, " "}, // add a space
    {"search", 'S', "STRING [-i][-o]", 0,
     "Search sources/sinks. Specify -i for inputs or -o for outputs"},
    {0, 'i', 0, OPTION_HIDDEN},
    {0, 'o', 0, OPTION_HIDDEN},
    {0, 0, 0, 0, " "}, // add a space
    {"sample", 's', "NUM", 0, "Set the sample buffer size of pipewire"},
    {0}};

struct argp argp = {
    options, parse_opt, NULL,
    "Connect and search pipewire connections and the ability to "
    "adjust the sample rate"};

typedef struct arguments {
  char *argz;
  size_t argz_len;
  bool link;
  bool search;
  bool sample;
  bool disonnect;
  bool input;
  bool output;
} arguments;

int parse_opt(int key, char *arg, struct argp_state *state) {
  arguments *arguments = state->input;

  switch (key) {
  case 'l': {
    arguments->link = true;
    argz_add(&arguments->argz, &arguments->argz_len, arg);
  } break;
  case 'd': {
    arguments->disonnect = true;
  } break;
  case 's': {
    arguments->sample = true;
    argz_add(&arguments->argz, &arguments->argz_len, arg);
  } break;
  case 'o': {
    arguments->output = true;
  } break;
  case 'i': {
    arguments->input = true;
  } break;
  case 'S': {
    arguments->search = true;
    argz_add(&arguments->argz, &arguments->argz_len, arg);
  } break;
  case ARGP_KEY_ARG: {
    argz_add(&arguments->argz, &arguments->argz_len, arg);
  } break;
  case ARGP_KEY_INIT: {
    arguments->argz = 0;
    arguments->argz_len = 0;
  } break;
  case ARGP_KEY_END: {
    if ((arguments->link + arguments->search + arguments->sample) != 1) {
      argp_error(state, "Cannot use more than one flag at a time");
    }
    size_t c = argz_count(arguments->argz, arguments->argz_len);
  }
  }
  return 0;
}

void pwsample(const char *sample_rate) {
  FILE *pipe;
  char cmd[1024];
  char line[1024];
  sprintf(cmd, "pw-metadata -n settings 0 clock.force-quantum %s", sample_rate);
  pipe = popen(cmd, "w");
  if (pipe == NULL) {
    perror("popen failed");
    return;
  }
  pclose(pipe);
}

void pwsearch(const char *query, arguments args) {
  FILE *pipe;
  char cmd[1024];
  char line[1024];
  if (args.input) {
    sprintf(cmd, "pw-link -I -i | grep -Poe .*%s.*", query);
  } else {
    sprintf(cmd, "pw-link -I -o | grep -Poe .*%s.*", query);
  }

  pipe = popen(cmd, "r");
  if (pipe == NULL) {
    perror("popen failed");
    return;
  }
  while (fgets(line, sizeof(line), pipe) != NULL) {
    printf("%s\n", line);
  }
  pclose(pipe);
}

void pwconnect(const char *sink, const char *source, bool should_connect) {
  FILE *pipe;
  char cmd[1024];
  char line[1024];
  char loop_buffer[1024];

  // Get sink first
  sprintf(cmd, "pw-link -I -i | grep -Poe .*%s.*", sink);
  pipe = popen(cmd, "r");
  if (pipe == NULL) {
    perror("popen failed");
    return;
  }

  int max_output_channels = 2;
  int right_output = 0;
  int left_output = 0;
  while (fgets(line, sizeof(line), pipe) != NULL) {
    sprintf(cmd, "echo '%s' | grep -Poe '\\d+'", line);
    FILE *fp = popen(cmd, "r");
    if (pipe == NULL) {
      perror("popen failed");
      break;
    }
    fgets(loop_buffer, sizeof(loop_buffer), fp);

    // assumes left channel is listed first
    if (max_output_channels % 2) {
      right_output = atoi(loop_buffer);
    } else {
      left_output = atoi(loop_buffer);
    }

    max_output_channels--;
    pclose(fp);

    if (max_output_channels == 0)
      break;
  }
  pclose(pipe);

  // Get sources after sink
  sprintf(cmd, "pw-link -I -o  | grep -Poe '\\d+(?=.*%s)'", source);

  pipe = popen(cmd, "r");
  if (pipe == NULL) {
    perror("popen failed");
    return;
  }

  int c = 0;
  char disconnect_string[2] = "";
  if (should_connect) {
    disconnect_string[0] = '-';
    disconnect_string[1] = 'd';
  }

  while (fgets(line, sizeof(line), pipe) != NULL) {
    if (c % 2) {
      // source -> sink
      sprintf(loop_buffer, "pw-link %d %d %s > /dev/null 2>&1 &", atoi(line),
              right_output, disconnect_string);
    } else {
      sprintf(loop_buffer, "pw-link %d %d %s > /dev/null 2>&1 &", atoi(line),
              left_output, disconnect_string);
    }
    FILE *p = popen(loop_buffer, "r");
    pclose(p);
    c++;
  }
  pclose(pipe);
}

int main(int argc, char **argv) {
  arguments arguments;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  const char *buffer = NULL;
  const char *word = NULL;
  if (arguments.search) {
    word = argz_next(arguments.argz, arguments.argz_len, buffer);
    pwsearch(word, arguments);
  }

  if (arguments.sample) {
    word = argz_next(arguments.argz, arguments.argz_len, buffer);
    pwsample(word);
  }

  if (arguments.link) {
    const char *sink, *source = NULL;
    int c = 1;
    while ((word = argz_next(arguments.argz, arguments.argz_len, buffer))) {
      if (c % 2) {
        source = word;
      } else {
        sink = word;
      }
      buffer = word;
      c++;
    }
    if (c > 3) {
      printf("Too many arguments supplied\n");
      exit(EXIT_FAILURE);
    } else if (c < 3) {
      printf("Not enough arguments supplied\n");
      exit(EXIT_FAILURE);
    }
    pwconnect(sink, source, arguments.disonnect);
  }

  free(arguments.argz);

  exit(0);
}
