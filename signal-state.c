#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int pending = 0, blocked = 0, ignored = 0, caught = 0;

void system_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

#define STATUS_FILENAME_TEMPLATE "/proc/%d/status"

#define DIE_IF_NEG(cmd)                                                        \
  ({                                                                           \
    typeof(cmd) ret = cmd;                                                     \
    if (ret < 0)                                                               \
      system_die(#cmd);                                                        \
    ret;                                                                       \
  })

#define DIE_IF_NULL(cmd)                                                       \
  ({                                                                           \
    typeof(cmd) ret = cmd;                                                     \
    if (ret == NULL)                                                           \
      system_die(#cmd);                                                        \
    ret;                                                                       \
  })

#define PRINT_INT(var) printf("%s: %d\n", #var, var)

#define MAX_SIGNAL 31

char signals[][31] = {
    "SIGHUP",  "SIGINT",    "SIGQUIT", "SIGILL",    "SIGTRAP", "SIGABRT",
    "SIGBUS",  "SIGFPE",    "SIGKILL", "SIGUSR1",   "SIGSEGV", "SIGUSR2",
    "SIGPIPE", "SIGALRM",   "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT",
    "SIGSTOP", "SIGTSTP",   "SIGTTIN", "SIGTTOU",   "SIGURG",  "SIGXCPU",
    "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH",  "SIGIO",   "SIGPWR",
    "SIGSYS",
};

void print_signal_mask(char *mask_str) {
  /* printf("%s\n", mask_str); */
  unsigned int mask;
  /* int num_scanned = sscanf("13\n", "%x", &mask); */
  /* last 8 chars of mask string: the basic signals */
  int num_scanned = DIE_IF_NEG(sscanf(mask_str, "%x", &mask));

  /* printf("%x\n", mask); */
  int first = 1;
  for (int i = 0; i != MAX_SIGNAL; ++i) {
    if (mask & (1 << i)) {
      if (first) {
        first = 0;
        printf("%s", signals[i]);
      } else {
        printf(", %s", signals[i]);
      }
    }
  }
  printf("\n");
}

void print_signals(pid_t target_pid) {
  printf("Pid: %d\n", target_pid);
  size_t len = snprintf(NULL, 0, STATUS_FILENAME_TEMPLATE, target_pid);
  char filename[len];
  sprintf(filename, STATUS_FILENAME_TEMPLATE, target_pid);

  FILE *file = DIE_IF_NULL(fopen(filename, "r"));

  ssize_t num_bytes_read;
  char *line = NULL;
  char mask_str[40];
  while ((num_bytes_read = getline(&line, &len, file) != -1)) {
    if (pending && sscanf(line, "SigPnd: %s", mask_str) == 1) {
      printf("Pending: ");
      print_signal_mask(mask_str);
    } else if (blocked && sscanf(line, "SigBlk: %s", mask_str) == 1) {
      printf("Blocked: ");
      print_signal_mask(mask_str);
    } else if (ignored && sscanf(line, "SigIgn: %s", mask_str) == 1) {
      printf("Ignored: ");
      print_signal_mask(mask_str);
    } else if (caught && sscanf(line, "SigCgt: %s", mask_str) == 1) {
      printf("Caught: ");
      print_signal_mask(mask_str);
    }
    free(line);
    line = NULL;
  }

  DIE_IF_NEG(fclose(file));
}

int main(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "pbicah")) != -1) {
    switch (opt) {
    case 'p':
      pending = 1;
      break;
    case 'b':
      blocked = 1;
      break;
    case 'i':
      ignored = 1;
      break;
    case 'c':
      caught = 1;
      break;
    case 'a':
      pending = 1;
      blocked = 1;
      ignored = 1;
      caught = 1;
      break;
    case 'h':
      fprintf(stderr, "Użycie: %s -[pbica] [-h] (pid)+\n", argv[0]);
      exit(EXIT_FAILURE);
    default:
      break;
      fprintf(stderr, "nieznana opcja: %c\n", opt);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "brak pida\n");
    exit(EXIT_FAILURE);
  }

  for (int i = optind; i != argc; i++) {
    char *end;
    pid_t target_pid = strtoul(argv[i], &end, 10);
    if (end - argv[i] != strlen(argv[i])) {
      fprintf(stderr, "Nie mozna interpretować jako pid: %s", argv[i]);
      exit(EXIT_FAILURE);
    }
    print_signals(target_pid);
  }
}
