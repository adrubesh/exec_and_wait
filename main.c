#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

void
exec_and_wait(char * const * cmd, long timeout)
{
        pid_t cpid, w;
        int status, done = 0;
        struct timespec tsstart, tscur;

        /* man time.h for clock explanation */
        clock_gettime(CLOCK_MONOTONIC, &tsstart);

        /* man 2 fork */
        if((cpid = fork()) == 0) {
                /* child proc */
                /* man 3 exec */
                if(execvp(cmd[0], cmd) == -1) {
                        printf("execvp error\n");
                }
        } else if (cpid > 0) {
                /* parent proc */
                do {
                        clock_gettime(CLOCK_MONOTONIC, &tscur);
                        /* man 2 wait */
                        w = waitpid(cpid, &status, WUNTRACED | WCONTINUED | WNOHANG);

                        if ((tscur.tv_sec - tsstart.tv_sec) >= timeout) {
                                printf("timeout reached! killing child\n");
                                printf("%d\n", kill(cpid, SIGTERM));
                                done = 1;
                        } else if (w == -1) {
                                done = 1; 
                                if(errno == ECHILD) { // Child has already exited.

                                } else {
                                        printf("%s\n", strerror(errno));
                                }
                        } else if (w > 0) {
                                done = 1;
                                if (WIFEXITED(status)) {
                                        printf("child exited of natural causes\n");
                                }
                        }
                } while (done == 0);
        } else {
                /* error cond. in forking */
        }
}

int
main(int argc, char **argv)
{
        char * const tests[3][3] = {
                { "sleep", "5", NULL },
                { "sleep", "2", NULL},
                { "ls", "/tmp", NULL}
        };
        
        for (int i = 0; i < 3; i++) {
                printf("==== TEST %d ====\n", (i + 1));
                exec_and_wait(tests[i], 3);
        }
}
