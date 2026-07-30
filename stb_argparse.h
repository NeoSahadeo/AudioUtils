/**
 * @file stb_argparse.h
 * @brief Single-header implementation of command line argument parser
 *
 * MIT License
 *
 * Copyright (c) 2026 pollyren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef STB_ARGPARSE_H
#define STB_ARGPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/********************* STRUCTS AND ENUMS *********************/

/**
 * @brief Enum representing errors in adding or parsing arguments
 */
typedef enum {
  /// no error occured
  ARGPARSE_NO_ERROR = 0,

  /** Add argument errors */
  /// the flag or name is invalid
  ARGPARSE_INVALID_FLAG_OR_NAME_ERROR = -1,
  /// type provided does not match available types
  ARGPARSE_INVALID_TYPE_ERROR = -2,
  /// action provided does not match available actions
  ARGPARSE_INVALID_ACTION_ERROR = -3,
  /// the presence of choices is not supported (e.g., for flags and counts)
  ARGPARSE_UNSUPPORTED_CHOICES_ERROR = -4,
  /// the action of the argument does not match its type
  ARGPARSE_UNSUPPORTED_ACTION_ERROR = -5,
  /// an argument with the same flag or name has already been added
  ARGPARSE_CONFLICTING_OPTIONS_ERROR = -6,
  /// argument cannot be required based on action
  ARGPARSE_UNSUPPORTED_REQUIRE_ERROR = -7,

  /** Parse argument errors */
  /// argument provided has not been added to parser
  ARGPARSE_ARGUMENT_UNKNOWN_ERROR = -8,
  /// no value is provided for a store-action argument
  ARGPARSE_ARGUMENT_MISSING_VALUE_ERROR = -9,
  /// integer argument value exceeds the range of an int
  ARGPARSE_INT_RANGE_EXCEEDED_ERROR = -10,
  /// value of argument does not match choices provided
  ARGPARSE_INVALID_CHOICE_ERROR = -11,
  /// argument is not provided on command line
  ARGPARSE_MISSING_ARGUMENT_ERROR = -12
} argparse_error_val;

/**
 * @brief Struct representing an argparse error
 *
 * Contains information on the error code, the error message and the argument
 * causing the error.
 */
typedef struct {
  /// error code
  argparse_error_val error_val;
  /// error message for printing (NULL if using generic error message)
  const char* error_msg;
  /// name of argument causing the error
  const char* arg_name;
  /// flag of argument causing the error
  char arg_flag;
} argparse_error_t;

/**
 * @brief Enum representing possible actions for an argument
 */
typedef enum {
  /// store the value of the argument, must be non-bool type
  ARGPARSE_STORE_ACTION,
  /// store true if the argument is present, must be bool type
  ARGPARSE_STORE_TRUE_ACTION,
  /// store false if the argument is present, must be bool type
  ARGPARSE_STORE_FALSE_ACTION,
  /// count the occurrences of argument, must be int type
  ARGPARSE_COUNT_ACTION,
  /// boolean action, similar to a flag, must be bool type
  ARGPARSE_BOOLEAN_OPTIONAL_ACTION
} argparse_action_t;

/**
 * @brief Enum representing possible types for an argument
 */
typedef enum {
  /// integer argument type, value should be `int`
  ARGPARSE_INT_TYPE,
  /// float argument type, value should be `float`
  ARGPARSE_FLOAT_TYPE,
  /// boolean argument type, value should be `bool`
  ARGPARSE_BOOL_TYPE,
  /// string argument type, value should be `const char *`
  ARGPARSE_STRING_TYPE
} argparse_type_t;

/**
 * @brief Struct representing an individual argument
 *
 * Contains metadata on argument type, next pointer and count, as well as
 * user-provided information on the flag and name and other argument data.
 */
typedef struct argparse_arg_t {
  /** Argument metadata */
  /// argument type
  argparse_type_t type_;
  /// pointer to the next argument, for linked list
  struct argparse_arg_t* next_;
  /// count occurrences when arguments are parsed
  int count_;

  /** User-provided information */
  /// single-character flag for the argument
  const char flag_;
  /// full name of the argument
  const char* name_;
  /// pointer to where the value should be stored, type should be consistent
  void* value_;
  /// action to perform when arguments are parsed
  argparse_action_t action_;
  /// help description for the argument
  const char* help_;
  /// whether the option is required
  bool required_;
  /// array of valid choices, type should be consistent
  void* choices_;
  /// number of valid choices choices provided in the array
  size_t num_choices_;
} argparse_arg_t;

/**
 * @brief Struct representing the parser
 *
 * Contains argc, argv, program name, program description and help message
 * epilog, as well as linked lists to all the options and positional arguments
 * that have been added to the parser.
 */
typedef struct {
  /// number of arguments
  int argc_;
  /// array of argument strings
  char** argv_;

  /// program name
  const char* prog_;
  /// text to display before argument help (NULL if no description)
  const char* description_;
  /// text to display after argument help (NULL if no epilog)
  const char* epilog_;

  /// linked list of non-positional options added to parser
  argparse_arg_t* options_;
  /// linked list of positional arguments added to parser
  argparse_arg_t* positional_args_;
} argument_parser_t;

/********************* FUNCTION DECLARATIONS *********************/

/**
 * @brief Prints the usage message
 *
 * @param parser Pointer to the `argument_parser_t`
 *
 * @note Only the options and positional arguments added to the parser will be
 * printed. This function can be called independently or by providing `-h` or
 * `--help` command-line options.
 */
void argparse_print_help(argument_parser_t* parser);

/**
 * @brief Initialises the parser
 *
 * @param parser Pointer to the `argument_parser_t` to initialise
 * @param argc Argument count (from `main`)
 * @param argv Argument vector (from `main`)
 * @param description Text to display before help (NULL if no description)
 * @param epilog Text to display after help (NULL if no epiilog)
 * @return An `argparse_error_t` indicating success or failure
 */
argparse_error_t argparse_init(argument_parser_t* parser,
                               int argc,
                               char** argv,
                               const char* description,
                               const char* epilog);

/**
 * @brief Adds a single argument to the parser
 *
 * @param parser Pointer to the parser
 * @param arg Pointer to the argument to add
 * @return An `argparse_error_t` indicating success or failure
 *
 * @note The argument should be initialised. The lifetime of `*arg` must also
 * extend at least until `argparse_parse_args` is called. The parser retains a
 * pointer to `*arg`, so having `*arg` go out-of-scope when the arguments are
 * parsed can lead to undefined behaviour. This means that if the arguments are
 * created in a different function from the one where `argparse_parse_args` is
 * called, the user is responsible for ensuring that `arg` is correctly
 * dynamically allocated.
 */
argparse_error_t argparse_add_argument(argument_parser_t* parser,
                                       argparse_arg_t* arg);

/**
 * @brief Adds multiple arguments to the parser
 *
 * @param parser Pointer to the parser
 * @param args Array of arguments to add
 * @param num_args The number of arguments in args
 * @return An `argparse_error_t` indicating success or failure
 *
 * @note All arguments should be initialised. The lifetime of each argument in
 * `args` must also extend at least until `argparse_parse_args` is called, as
 * the parser retains a pointer to each argument. See function comment for
 * `argparse_add_argument` for additional note.
 */
argparse_error_t argparse_add_arguments(argument_parser_t* parser,
                                        argparse_arg_t* args,
                                        size_t num_args);

/**
 * @brief Parses the command-line arguments
 *
 * @param parser Pointer to the parser
 * @return An `argparse_error_t` indicating success or failure
 *
 * @note The arguments should already be added to the parser. The parser and
 * all the arguments that have been added to the parser must also be in scope
 * when this function is called to avoid undefined behaviour.
 */
argparse_error_t argparse_parse_args(argument_parser_t* parser);

/**
 * @brief Prints error message if there is an error
 *
 * @param error Error struct containing error information
 * @return An `argparse_error_val` containing the exit code of the error
 */
argparse_error_val argparse_check_error(argparse_error_t error);

/**
 * @brief Prints error message and exits if there is an error
 *
 * @param error Error struct containing error information
 *
 * @note This function exits the program and returns the number corresponding
 * to the error code of the error, only if there is a non-zero exit code.
 */
void argparse_check_error_and_exit(argparse_error_t error);

/********************* CONVENIENCE MACROS *********************/

/**
 * Convenience placeholders for creating arguments. Mostly used for readability
 * in the convenience macros for creating arguments.
 */
#define NO_DESCRIPTION NULL
#define NO_EPILOG NULL
#define NO_FLAG '\0'
#define NO_NAME NULL
#define NO_HELP NULL
#define REQUIRED true
#define OPTIONAL false
#define NO_CHOICES NULL

/********************* BASIC ARGUMENT MACROS *********************/

/**
 * @defgroup ARGPARSE_MACROS Argument macros
 * @brief Macros for creating arguments
 *
 * These macros make it more convenient to create a `argparse_arg_t` argument.
 * They provide a shorthand for defining positional arguments and options
 * with various properties, such as required options or choices lists.
 *
 * For the following macros:
 * - `type`: must be `INT`, `FLOAT`, `BOOL` or `STRING`
 * - `flag`: a character flag, used as short name of argument
 * - `name`: long-form name of argument, should generally have `--` prefix
 * - `res`: pointer to the variable where parsed result should be stored
 * - `help`: a `const char *` describing the argument, for the help message
 * - `choices`: an array of valid choices
 * - `num_choices`: the number of entries within the `choices` array
 *
 * @note The type of `res` and the type of `choices` must be consistent with the
 * type of the argument.
 *
 * @{
 */

/**
 * @brief Defines a basic argparse argument
 * @param type Argument type (must be INT, FLOAT, BOOL or STRING)
 * @param ... Additional `argument_arg_t` fields
 *
 * @note All the fields of the argument, aside from type_, next_ and count_,
 * must be initialised in the varadic args.
 */
#define ARGPARSE_ARG(type, ...)                      \
  ((argparse_arg_t){.type_ = ARGPARSE_##type##_TYPE, \
                    .next_ = NULL,                   \
                    .count_ = 0,                     \
                    __VA_ARGS__})

/**
 * @brief Initialises an already allocated argparse argument
 * @param ptr Pointer to memory allocated for an `argparse_arg_t`
 * @param type Argument type (must be INT, FLOAT, BOOL or STRING)
 * @param ... Additional `argparse_arg_t` fields
 *
 * @note The pointer `ptr` must not be NULL and should point to memory allocated
 * to hold an `argparse_arg_t`. All the fields of the argument, aside from
 * type_, next_ and count_, must be initialised in the varadic args.
 */
#define ARGPARSE_ARG_INIT(ptr, type, ...)                      \
  do {                                                         \
    *(ptr) = (argparse_arg_t){.type_ = ARGPARSE_##type##_TYPE, \
                              .next_ = NULL,                   \
                              .count_ = 0,                     \
                              __VA_ARGS__};                    \
  } while (0)

/********************* POSITIONAL ARGUMENT MACROS *********************/

/**
 * @brief Creates a positional argument without choices
 */
#define ARGPARSE_POSITIONAL(type, name, res, help)                       \
  ARGPARSE_ARG(type, '\0', (name), (res), ARGPARSE_STORE_ACTION, (help), \
               REQUIRED, NO_CHOICES, 0)

/**
 * @brief Creates a positional argument with choices
 */
#define ARGPARSE_POSITIONAL_WITH_CHOICES(type, name, res, help, choices, \
                                         num_choices)                    \
  ARGPARSE_ARG(type, '\0', (name), (res), ARGPARSE_STORE_ACTION, (help), \
               REQUIRED, (choices), (num_choices))

/********************* OPTION MACROS *********************/

/**
 * @brief Creates an option
 */
#define ARGPARSE_OPTION(type, flag, name, res, help)                       \
  ARGPARSE_ARG(type, (flag), (name), (res), ARGPARSE_STORE_ACTION, (help), \
               OPTIONAL, NO_CHOICES, 0)

/**
 * @brief Creates a required option
 */
#define ARGPARSE_OPTION_REQUIRED(type, flag, name, res, help)              \
  ARGPARSE_ARG(type, (flag), (name), (res), ARGPARSE_STORE_ACTION, (help), \
               REQUIRED, NO_CHOICES, 0)

/**
 * @brief Creates an option with choices
 */
#define ARGPARSE_OPTION_WITH_CHOICES(type, flag, name, res, help, choices, \
                                     num_choices)                          \
  ARGPARSE_ARG(type, (flag), (name), (res), ARGPARSE_STORE_ACTION, (help), \
               OPTIONAL, (choices), (num_choices))

/********************* SPECIAL OPTION MACROS *********************/

/**
 * @brief Creates an integer option that counts the occurrences of the argument
 * on the command line when parsing
 */
#define ARGPARSE_COUNT(flag, name, res, help)                             \
  ARGPARSE_ARG(INT, (flag), (name), (res), ARGPARSE_COUNT_ACTION, (help), \
               OPTIONAL, NO_CHOICES, 0)

/**
 * @brief Creates a boolean toggle option that stores true if --name is
 * provided on the command line and false if --no-name is provided
 */
#define ARGPARSE_TOGGLE(flag, name, res, help)                                \
  ARGPARSE_ARG(BOOL, (flag), (name), (res), ARGPARSE_BOOLEAN_OPTIONAL_ACTION, \
               (help), OPTIONAL, NO_CHOICES, 0)

/**
 * @brief Creates a boolean option that stores true if argument is provided
 * on the command line when parsing
 */
#define ARGPARSE_FLAG_TRUE(flag, name, res, help)                       \
  ARGPARSE_ARG(BOOL, (flag), (name), (res), ARGPARSE_STORE_TRUE_ACTION, \
               (help), OPTIONAL, NO_CHOICES, 0)

/**
 * @brief Creates a boolean option that stores false if argument is provided
 * on the command line when parsing
 */
#define ARGPARSE_FLAG_FALSE(flag, name, res, help)                       \
  ARGPARSE_ARG(BOOL, (flag), (name), (res), ARGPARSE_STORE_FALSE_ACTION, \
               (help), OPTIONAL, NO_CHOICES, 0)

/** @} */  // end of ARGPARSE_MACROS group

#ifdef STB_ARGPARSE_IMPLEMENTATION

/********************* ERROR MACROS *********************/

#define ARGPARSE_NO_ERROR() \
  ((argparse_error_t){ARGPARSE_NO_ERROR, NULL, NULL, 0})

#define ARGPARSE_INVALID_FLAG_OR_NAME_ERROR(msg, name) \
  ((argparse_error_t){ARGPARSE_INVALID_FLAG_OR_NAME_ERROR, (msg), (name), 0})

#define ARGPARSE_INVALID_TYPE_ERROR(type) \
  ((argparse_error_t){ARGPARSE_INVALID_TYPE_ERROR, NULL, NULL, (type)})

#define ARGPARSE_INVALID_ACTION_ERROR(action) \
  ((argparse_error_t){ARGPARSE_INVALID_ACTION_ERROR, NULL, NULL, (action)})

#define ARGPARSE_UNSUPPORTED_CHOICES_ERROR(name, flag) \
  ((argparse_error_t){ARGPARSE_UNSUPPORTED_CHOICES_ERROR, NULL, (name), (flag)})

#define ARGPARSE_UNSUPPORTED_ACTION_ERROR(msg, name, flag) \
  ((argparse_error_t){ARGPARSE_UNSUPPORTED_ACTION_ERROR, (msg), (name), (flag)})

#define ARGPARSE_CONFLICTING_OPTIONS_ERROR(msg, name, flag)              \
  ((argparse_error_t){ARGPARSE_CONFLICTING_OPTIONS_ERROR, (msg), (name), \
                      (flag)})

#define ARGPARSE_UNSUPPORTED_REQUIRE_ERROR(name, flag) \
  ((argparse_error_t){ARGPARSE_UNSUPPORTED_REQUIRE_ERROR, NULL, (name), (flag)})

#define ARGPARSE_ARGUMENT_UNKNOWN_ERROR(name) \
  ((argparse_error_t){ARGPARSE_ARGUMENT_UNKNOWN_ERROR, NULL, (name), 0})

#define ARGPARSE_INT_RANGE_EXCEEDED_ERROR(msg, name, flag) \
  ((argparse_error_t){ARGPARSE_INT_RANGE_EXCEEDED_ERROR, (msg), (name), (flag)})

#define ARGPARSE_ARGUMENT_MISSING_VALUE_ERROR(name, flag)                  \
  ((argparse_error_t){ARGPARSE_ARGUMENT_MISSING_VALUE_ERROR, NULL, (name), \
                      (flag)})

#define ARGPARSE_INVALID_CHOICE_ERROR(name, flag) \
  ((argparse_error_t){ARGPARSE_INVALID_CHOICE_ERROR, NULL, (name), (flag)})

#define ARGPARSE_MISSING_ARGUMENT_ERROR(name, flag) \
  ((argparse_error_t){ARGPARSE_MISSING_ARGUMENT_ERROR, NULL, (name), (flag)})

/*
 * Returns the error code and exits from the function if non-zero error code
 */
#define CHECK_ERROR(error) \
  do {                     \
    if (error.error_val) { \
      return error;        \
    }                      \
  } while (0)

/*
 * Converts a lowercase letter to uppercase
 */
#define TO_UPPER(c) ((c) >= 'a' && (c) <= 'z' ? (c) - ('a' - 'A') : (c))

/*
 * Produces string from flag if name is not provided
 */
#define FORMAT_FN_STRING(error) \
  (error.arg_name ? error.arg_name : (char[]){'-', error.arg_flag, '\0'})

/*
 * Epsilon tolerance for floating-point imprecision. Used for float choices
 */
#define EPSILON_FLOAT 1e-5

/*
 * Checks that `val` is within `EPSILON_FLOAT` of `tar`. Used for float choices
 */
#define WITHIN(val, tar) \
  ((val) >= (tar) - EPSILON_FLOAT && (val) <= (tar) + EPSILON_FLOAT)

/*
 * Prints the possible choices for the argument in a comma-delimited list. Wraps
 * choices in curly braces if curly is true. Helper for `argparse_print_help`
 */
static void print_choices(argparse_arg_t* arg, bool curly) {
  const char* start = curly ? " {" : " (choices: ";
  if (arg->choices_) {
    for (size_t j = 0; j < arg->num_choices_; j++) {
      switch (arg->type_) {
        case ARGPARSE_INT_TYPE:
          printf("%s%d", j ? "," : start, ((int*)arg->choices_)[j]);
          break;
        case ARGPARSE_FLOAT_TYPE:
          printf("%s%.3f", j ? "," : start, ((float*)arg->choices_)[j]);
          break;
        case ARGPARSE_STRING_TYPE:
          printf("%s\"%s\"", j ? "," : start, ((const char**)arg->choices_)[j]);
          break;
        default:
          fprintf(stderr,
                  "should not get here, bools do not support choices\n");
          exit(EXIT_FAILURE);
      }
    }
    printf(curly ? "}" : ")");
  }
}

/*
 * Prints the formatted help message, including all the options and positional
 * arguments added to the parser. Invoked using `-h` or `--help` option
 */
void argparse_print_help(argument_parser_t* parser) {
  printf("usage: %s [-h]", parser->prog_);

  argparse_arg_t* arg = parser->options_;
  while (arg) {
    printf("%s", arg->required_ ? " " : " [");
    if (arg->action_ == ARGPARSE_BOOLEAN_OPTIONAL_ACTION) {
      if (arg->flag_ && arg->name_) {
        printf("-%c | %s | --no-%s", arg->flag_, arg->name_, arg->name_ + 2);
      } else if (arg->flag_) {
        printf("-%c", arg->flag_);
      } else if (arg->name_) {
        printf("%s | --no-%s", arg->name_, arg->name_ + 2);
      }
    } else if (arg->flag_) {
      printf("-%c", arg->flag_);
    } else if (arg->name_) {
      printf("%s", arg->name_);
    }

    if (arg->action_ == ARGPARSE_STORE_ACTION && arg->choices_ == NULL) {
      if (arg->name_) {
        size_t i = 2;
        printf(" ");
        while (arg->name_[i]) {
          printf("%c", arg->name_[i] == '-' ? '_' : TO_UPPER(arg->name_[i]));
          i++;
        }
      } else {
        printf(" %c", TO_UPPER(arg->flag_));
      }
    }

    print_choices(arg, true);

    printf("%s", arg->required_ ? "" : "]");

    arg = arg->next_;
  }
  arg = parser->positional_args_;
  while (arg) {
    printf(" %s", arg->name_);
    arg = arg->next_;
  }
  printf("\n");

  if (parser->description_) {
    printf("\n%s\n", parser->description_);
  }

  if (parser->positional_args_) {
    printf("\npositional arguments:\n");
    arg = parser->positional_args_;
    while (arg) {
      printf("  %-24s %s\n", arg->name_, arg->help_ ? arg->help_ : "");
      arg = arg->next_;
    }
  }

  printf("\noptions:\n");
  printf("  -h, %-20s %s\n", "--help", "show this help message and exit");
  arg = parser->options_;
  while (arg) {
    printf("  -%c, %-20s %s", arg->flag_ ? arg->flag_ : ' ',
           arg->name_ ? arg->name_ : "", arg->help_ ? arg->help_ : "");
    print_choices(arg, false);
    printf("\n");
    arg = arg->next_;
  }

  if (parser->epilog_) {
    printf("\n%s\n", parser->epilog_);
  }
}

argparse_error_t argparse_init(argument_parser_t* parser,
                               int argc,
                               char** argv,
                               const char* description,
                               const char* epilog) {
  parser->argc_ = --argc;
  parser->prog_ = argv++[0];
  parser->argv_ = argv;
  parser->description_ = description;
  parser->epilog_ = epilog;
  parser->options_ = NULL;
  parser->positional_args_ = NULL;

  return ARGPARSE_NO_ERROR();
}

/*
 * Checks if `string` starts with `prefix`
 */
static bool starts_with(const char* string, const char* prefix) {
  return strncmp(string, prefix, strlen(prefix)) == 0;
}

/*
 * Checks if the flags or names match between two arguments. Helper for
 * checking whether a conflicting argument is added in `argparse_add_argument`
 */
static argparse_error_t check_added(argparse_arg_t* added,
                                    argparse_arg_t* arg) {
  if (arg->flag_ && arg->flag_ == added->flag_) {
    return ARGPARSE_CONFLICTING_OPTIONS_ERROR(
        "argparse_add_argument: option string %s already in use\n", NULL,
        arg->flag_);
  }

  if (arg->name_ && strcmp(arg->name_, added->name_) == 0) {
    return ARGPARSE_CONFLICTING_OPTIONS_ERROR(
        "argparse_add_argument: option string %s already in use\n", arg->name_,
        0);
  }

  return ARGPARSE_NO_ERROR();
}

argparse_error_t argparse_add_argument(argument_parser_t* parser,
                                       argparse_arg_t* arg) {
  // check that at least one of flag or name is provided
  if (arg->flag_ == NO_FLAG &&
      (arg->name_ == NO_NAME || strlen(arg->name_) == 0)) {
    return ARGPARSE_INVALID_FLAG_OR_NAME_ERROR(
        "argparse_add_argument: argument must "
        "contain at least one of flag or name\n",
        NULL);
  }

  // check that type is valid
  switch (arg->type_) {
    case ARGPARSE_INT_TYPE:
    case ARGPARSE_FLOAT_TYPE:
    case ARGPARSE_BOOL_TYPE:
    case ARGPARSE_STRING_TYPE:
      break;
    default:
      return ARGPARSE_INVALID_TYPE_ERROR(arg->type_);
  }

  bool positional = arg->name_ && !starts_with(arg->name_, "-");
  bool bool_action = false;

  // check that action is valid
  // check that store and boolean actions have a bool type
  // check that non-store actions do not provide choices
  switch (arg->action_) {
    case ARGPARSE_STORE_TRUE_ACTION:
    case ARGPARSE_STORE_FALSE_ACTION:
    case ARGPARSE_BOOLEAN_OPTIONAL_ACTION:
      bool_action = true;
      if (arg->type_ != ARGPARSE_BOOL_TYPE) {
        return ARGPARSE_UNSUPPORTED_ACTION_ERROR(
            "argparse_add_argument: store true/false and boolean optional "
            "actions must have bool type for %s\n",
            arg->name_, arg->flag_);
      }
      goto remaining;
    case ARGPARSE_COUNT_ACTION:
      if (arg->type_ != ARGPARSE_INT_TYPE) {
        return ARGPARSE_UNSUPPORTED_ACTION_ERROR(
            "argparse_add_argument: count action must have int type for %s",
            arg->name_, arg->flag_);
      }
    remaining:
      if (arg->choices_) {
        return ARGPARSE_UNSUPPORTED_CHOICES_ERROR(arg->name_, arg->flag_);
      }
      if (positional) {
        return ARGPARSE_UNSUPPORTED_ACTION_ERROR(
            "argparse_add_argument: positional argument %s must have store "
            "action\n",
            arg->name_, 0);
      }
      if (arg->required_) {
        return ARGPARSE_UNSUPPORTED_REQUIRE_ERROR(arg->name_, arg->flag_);
      }
      __attribute__((fallthrough));
    case ARGPARSE_STORE_ACTION:
      if (!bool_action && arg->type_ == ARGPARSE_BOOL_TYPE) {
        return ARGPARSE_UNSUPPORTED_ACTION_ERROR(
            "argparse_add_argument: store or count actions must have "
            "non-bool type for %s\n",
            arg->name_, arg->flag_);
      }
      break;
    default:
      return ARGPARSE_INVALID_ACTION_ERROR(arg->action_);
  }

  // check that positional argument does not have a flag as well
  if (positional && arg->flag_) {
    return ARGPARSE_INVALID_FLAG_OR_NAME_ERROR(
        "argparse_add_argument: option string '%s' must start with '-'\n",
        arg->name_);
  }

  // check that optional argument name is not just '--'
  if (arg->name_ && !positional && strlen(arg->name_) == 2) {
    return ARGPARSE_INVALID_FLAG_OR_NAME_ERROR(
        "argparse_add_argument: must provide name for options like '--'\n",
        NULL);
  }

  // checks that flag or name does not overlap with help options
  if (arg->flag_ == 'h') {
    return ARGPARSE_CONFLICTING_OPTIONS_ERROR(
        "argparse_add_argument: -h flag reserved for help\n", NULL, 0);
  }
  if (arg->name_ && strcmp(arg->name_, "--help") == 0) {
    return ARGPARSE_CONFLICTING_OPTIONS_ERROR(
        "argparse_add_argument: --help option string reserved for help\n", NULL,
        0);
  }

  // check that flag and name are unused
  argparse_arg_t* added =
      positional ? parser->positional_args_ : parser->options_;
  argparse_arg_t* added_prev = NULL;

  argparse_error_t error;
  while (added) {
    error = check_added(added, arg);
    CHECK_ERROR(error);
    added_prev = added;
    added = added->next_;
  }

  if (added_prev) {
    added_prev->next_ = arg;
  } else {
    if (positional) {
      parser->positional_args_ = arg;
    } else {
      parser->options_ = arg;
    }
  }

  return ARGPARSE_NO_ERROR();
}

argparse_error_t argparse_add_arguments(argument_parser_t* parser,
                                        argparse_arg_t* args,
                                        size_t num_args) {
  argparse_error_t error;
  for (size_t i = 0; i < num_args; i++) {
    error = argparse_add_argument(parser, &args[i]);
    CHECK_ERROR(error);
  }
  return ARGPARSE_NO_ERROR();
}

/*
 * Struct to store flag, name or index to search in list of arguments. The `tag`
 * field indicates which member of `u` to access. `prefix` should be unused
 * except when prepending a prefix when searching for a name
 */
typedef struct {
  union {
    char flag;
    const char* name;
    size_t index;
  } u;
  const char* prefix;
  enum { FLAG_TO_FIND, NAME_TO_FIND, INDEX_TO_FIND } tag;
} arg_to_find;

/*
 * Find an argument `arg` in the `arg_list`. Checks flag, name or index member
 * based on the tag
 */
static argparse_arg_t* find_arg(argparse_arg_t* arg_list, arg_to_find arg) {
  argparse_arg_t* curr_arg = arg_list;

  size_t i = 0;
  while (curr_arg) {
    switch (arg.tag) {
      case FLAG_TO_FIND:
        if (arg.u.flag == curr_arg->flag_) {
          return curr_arg;
        }
        break;
      case NAME_TO_FIND:
        if (arg.prefix) {
          if (starts_with(curr_arg->name_, arg.prefix) &&
              strcmp(arg.u.name, curr_arg->name_ + strlen(arg.prefix)) == 0) {
            return curr_arg;
          }
        } else {
          if (strcmp(arg.u.name, curr_arg->name_) == 0) {
            return curr_arg;
          }
        }
        break;
      case INDEX_TO_FIND:
        if (arg.u.index == i) {
          return curr_arg;
        }
        break;
      default:
        fprintf(stderr, "find_arg: should not ever get here\n");
        exit(EXIT_FAILURE);
    }

    curr_arg = curr_arg->next_;
    i++;
  }

  return NULL;
}

/*
 * Updates the memory location pointed to by `arg->value_` with `value`.
 * Performs type-cast based on `arg->type_`. Helper for `get_and_update_value`
 */
static argparse_error_t update_value(argparse_arg_t* arg,
                                     void* value,
                                     bool string) {
  switch (arg->type_) {
    case ARGPARSE_INT_TYPE: {
      errno = 0;
      long res;
      res = string ? strtol(value, NULL, 0) : *(int*)value;
      if (errno == ERANGE || res > INT32_MAX || res < INT32_MIN) {
        return ARGPARSE_INT_RANGE_EXCEEDED_ERROR(
            "argparse_parse_args: value for %s exceeds range of integer\n",
            arg->name_, arg->flag_);
      }
      *(int*)arg->value_ = (int)res;
      break;
    }
    case ARGPARSE_FLOAT_TYPE:
      *(float*)arg->value_ = string ? strtof(value, NULL) : *(float*)value;
      break;
    case ARGPARSE_BOOL_TYPE:
      *(bool*)arg->value_ = *(bool*)value;
      break;
    case ARGPARSE_STRING_TYPE:
      *(const char**)arg->value_ = value;
      break;
    default:
      fprintf(stderr, "argparse_parse_args: should not ever get here\n");
      exit(EXIT_FAILURE);
  }
  return ARGPARSE_NO_ERROR();
}

/*
 * Gets the value for the corresponding argument from `argv` and updates the
 * the `value_` field of that argument. Advances the index `i` in the parsing
 * function if the argument is positional. Helper for `argparse_parse_args`
 */
static argparse_error_t get_and_update_value(argument_parser_t* parser,
                                             argparse_arg_t* arg,
                                             int* i,
                                             bool positional,
                                             bool negated) {
  arg->count_++;
  bool tmp_bool = arg->num_choices_ == 0;
  void* value = NULL;

  switch (arg->action_) {
    case ARGPARSE_STORE_ACTION:
      if (!positional && *i + 1 == parser->argc_) {
        return ARGPARSE_ARGUMENT_MISSING_VALUE_ERROR(parser->argv_[*i], 0);
      }

      value =
          positional ? (void*)parser->argv_[*i] : (void*)parser->argv_[++(*i)];

      // check if the value is in the choices
      for (size_t j = 0; j < arg->num_choices_; j++) {
        switch (arg->type_) {
          case ARGPARSE_INT_TYPE:
            errno = 0;
            long res = strtol(value, NULL, 0);
            if (errno == ERANGE || res > INT32_MAX || res < INT32_MIN) {
              return ARGPARSE_INT_RANGE_EXCEEDED_ERROR(
                  "argparse_parse_args: choice for %s exceeds range of "
                  "integer\n",
                  arg->name_, arg->flag_);
            }
            tmp_bool = ((int*)arg->choices_)[j] == (int)res;
            break;
          case ARGPARSE_FLOAT_TYPE:
            tmp_bool = WITHIN(strtof(value, NULL), ((float*)arg->choices_)[j]);
            break;
          case ARGPARSE_BOOL_TYPE:
            // non-store actions and bool types do not support choices
            break;
          case ARGPARSE_STRING_TYPE:
            tmp_bool = strcmp(((const char**)arg->choices_)[j],
                              (const char*)value) == 0;
            break;
          default:
            fprintf(stderr, "should not get here\n");
            exit(EXIT_FAILURE);
        }
        if (tmp_bool) {
          break;
        }
      }
      if (!tmp_bool) {
        return ARGPARSE_INVALID_CHOICE_ERROR(parser->argv_[*i - !positional],
                                             0);
      }

      break;

    case ARGPARSE_STORE_TRUE_ACTION:
      tmp_bool = true;
      value = &tmp_bool;
      break;

    case ARGPARSE_STORE_FALSE_ACTION:
      tmp_bool = false;
      value = &tmp_bool;
      break;

    case ARGPARSE_BOOLEAN_OPTIONAL_ACTION:
      tmp_bool = !negated;
      value = &tmp_bool;
      break;

    case ARGPARSE_COUNT_ACTION:
      // count will be set when argument is finalised
      return ARGPARSE_NO_ERROR();

    default:
      fprintf(stderr, "get_and_update_value: should not ever get here\n");
      exit(EXIT_FAILURE);
  }

  argparse_error_t error =
      update_value(arg, value, arg->action_ == ARGPARSE_STORE_ACTION);
  CHECK_ERROR(error);

  return ARGPARSE_NO_ERROR();
}

/*
 * Finalises the argument after all the arguments have been parsed. Values for
 * count actions are updated and missing arguments result in an error
 */
static argparse_error_t finalise_arg(argparse_arg_t* arg) {
  if (arg->action_ == ARGPARSE_COUNT_ACTION) {
    return update_value(arg, &arg->count_, false);
  }

  if (arg->count_ == 0 && arg->required_) {
    return ARGPARSE_MISSING_ARGUMENT_ERROR(arg->name_, arg->flag_);
  }

  return ARGPARSE_NO_ERROR();
}

argparse_error_t argparse_parse_args(argument_parser_t* parser) {
  size_t positional_i = 0;
  for (int i = 0; i < parser->argc_; i++) {
    const char* arg_string = parser->argv_[i];
    argparse_arg_t* arg = NULL;
    bool positional = false, negated = false;
    if (strcmp(arg_string, "-h") == 0 || strcmp(arg_string, "--help") == 0) {
      // display usage string
      argparse_print_help(parser);
      exit(EXIT_SUCCESS);
    } else if (strlen(arg_string) == 2 && starts_with(arg_string, "-")) {
      // option flag
      arg = find_arg(parser->options_,
                     (arg_to_find){.tag = FLAG_TO_FIND,
                                   .prefix = NULL,
                                   .u.flag = *(arg_string + 1)});
    } else if (starts_with(arg_string, "-")) {
      if (starts_with(arg_string, "--no-")) {
        // potential boolean optional
        arg =
            find_arg(parser->options_, (arg_to_find){.tag = NAME_TO_FIND,
                                                     .prefix = "--",
                                                     .u.name = arg_string + 5});
        negated = true;
      }

      if (arg == NULL || arg->action_ != ARGPARSE_BOOLEAN_OPTIONAL_ACTION) {
        // option name
        arg = find_arg(parser->options_, (arg_to_find){.tag = NAME_TO_FIND,
                                                       .prefix = NULL,
                                                       .u.name = arg_string});
        negated = false;
      }

      if (arg == NULL) {
        // grouped option flags
        size_t j = 1;
        while (arg_string[j]) {
          arg = find_arg(parser->options_,
                         (arg_to_find){.tag = FLAG_TO_FIND,
                                       .prefix = NULL,
                                       .u.flag = *(arg_string + j)});
          if (arg == NULL) {
            goto unknown;
          }
          argparse_error_t error = get_and_update_value(
              parser, arg, &i, arg->action_ != ARGPARSE_STORE_ACTION, false);
          CHECK_ERROR(error);
          j++;
        }
        continue;
      }
    } else {
      // positional argument
      arg = find_arg(
          parser->positional_args_,
          (arg_to_find){
              .tag = INDEX_TO_FIND, .prefix = NULL, .u.index = positional_i++});
      positional = true;
    }

  unknown:
    if (arg == NULL) {
      return ARGPARSE_ARGUMENT_UNKNOWN_ERROR(arg_string);
    }

    argparse_error_t error =
        get_and_update_value(parser, arg, &i, positional, negated);
    CHECK_ERROR(error);
  }

  // set default values for missing options and raise an error if any missing
  // arguments are required
  argparse_arg_t* args[] = {parser->options_, parser->positional_args_};
  argparse_error_t error;
  for (size_t i = 0; i < 2; i++) {
    argparse_arg_t* arg = args[i];
    while (arg) {
      error = finalise_arg(arg);
      CHECK_ERROR(error);
      arg = arg->next_;
    }
  }

  return ARGPARSE_NO_ERROR();
}

argparse_error_val argparse_check_error(argparse_error_t error) {
  if (error.error_val != ARGPARSE_NO_ERROR) {
    fprintf(stderr, "\n");
  }
  switch (error.error_val) {
    case ARGPARSE_NO_ERROR:
      break;
    case ARGPARSE_INVALID_FLAG_OR_NAME_ERROR:
    case ARGPARSE_UNSUPPORTED_ACTION_ERROR:
    case ARGPARSE_CONFLICTING_OPTIONS_ERROR:
    case ARGPARSE_INT_RANGE_EXCEEDED_ERROR:
      if (error.arg_name || error.arg_flag) {
        fprintf(stderr, error.error_msg, FORMAT_FN_STRING(error));
      } else {
        fprintf(stderr, "%s", error.error_msg);
      }
      break;
    case ARGPARSE_INVALID_TYPE_ERROR:
      fprintf(stderr, "argparse_add_argument: %d is not a valid type\n",
              error.arg_flag);
      break;
    case ARGPARSE_INVALID_ACTION_ERROR:
      fprintf(stderr, "argparse_add_argument: %d is not a valid action\n",
              error.arg_flag);
      break;
    case ARGPARSE_UNSUPPORTED_CHOICES_ERROR:
      fprintf(
          stderr,
          "argparse_parse_args: only store action supports choices for %s\n",
          FORMAT_FN_STRING(error));
      break;
    case ARGPARSE_UNSUPPORTED_REQUIRE_ERROR:
      fprintf(stderr,
              "argparse_add_argument: only store actions can be required for "
              "%s\n",
              FORMAT_FN_STRING(error));
      break;
    case ARGPARSE_ARGUMENT_UNKNOWN_ERROR:
      fprintf(stderr, "argparse_parse_args: unknown %s argument\n",
              error.arg_name);
      break;
    case ARGPARSE_ARGUMENT_MISSING_VALUE_ERROR:
      fprintf(stderr, "argparse_parse_args: expected value for %s\n",
              error.arg_name);
      break;
    case ARGPARSE_INVALID_CHOICE_ERROR:
      fprintf(stderr,
              "argparse_parse_args: value provided for %s is not a valid "
              "choice\n",
              error.arg_name);
      break;
    case ARGPARSE_MISSING_ARGUMENT_ERROR:
      fprintf(stderr, "argparse_parse_args: missing required argument %s\n",
              FORMAT_FN_STRING(error));
      break;
    default:
      fprintf(stderr, "unknown error: %d\n", error.error_val);
      exit(EXIT_FAILURE);
  }
  return error.error_val;
}

void argparse_check_error_and_exit(argparse_error_t error) {
  if (argparse_check_error(error)) {
    exit(error.error_val);
  }
}

#endif /* STB_ARGPARSE_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* STB_ARGPARSE_H */
