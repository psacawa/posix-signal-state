# Signal State

Simple C script to find to signal dispositions of POSIX signals by parsing `/proc/$pid/status`.

To use;

```sh
make signal-state
./signal-state $pid
```

E.g.

```
> ./signal-state 1
Pid: 1
Pending:
Blocked: SIGHUP, SIGINT, SIGUSR1, SIGUSR2, SIGTERM, SIGCHLD, SIGWINCH, SIGPWR
Ignored: SIGPIPE
Caught: SIGQUIT, SIGILL, SIGABRT, SIGBUS, SIGFPE, SIGSEGV
```
