#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

#include "apa102.h"
#include "bubbles.h"
#include "color.h"
#include "particles.h"
#include "simple_step.h"
#include "single_color.h"
#include "waves.h"

#define stof(a) strtof(a, NULL)

int usage(const char* name) {
	fprintf(stderr, "Usage: %s hsv <hue> <saturation> <value> (0.0-1.0)\n", name);
	fprintf(stderr, "       %s rgb <red> <green> <blue> (0.0-1.0)\n", name);
	fprintf(stderr, "       %s particles <period> (1-1000)\n", name);
	fprintf(stderr, "       %s bubbles <hue> <saturation> <value> (0.0-1.0)\n", name);

	return 1;
}

// TODO: refactor to accessor with static running
int running = 1;

void sighandler(int signum) {
	running = 0;
}

int daemonize() {
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	int log_fd = open("/tmp/apa102.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
	int input_fd = open("/dev/null", O_RDONLY);

	if (log_fd < 0) {
		perror("open logfile");
	}

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log the failure */
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	dup2(input_fd, STDIN_FILENO);
	dup2(log_fd, STDOUT_FILENO);
	dup2(log_fd, STDERR_FILENO);

	printf("apa102 started (%i)\n", getpid());

	FILE* pidfile = fopen("/tmp/apa102.pid", "w");
	if (pidfile != NULL) {
		fprintf(pidfile, "%u", getpid());
		fclose(pidfile);
	} else {
		printf("Could not write pidfile\n");
	}
}

static void kill_existing() {
	FILE* pidfile = fopen("/tmp/apa102.pid", "r");
	if (pidfile != NULL) {
		int pid;
		if (fscanf(pidfile, "%u", &pid) == 1) {
			if (kill(pid, SIGINT) == 0) {
				printf("running daemon detected. signal sent.\n");
				while (access("/tmp/apa102.pid", F_OK) == 0) {
					usleep(10000);
				}
			} else {
				printf("stale pidfile detected.\n");
				unlink("/tmp/apa102.pid");
			}
		}

		fclose(pidfile);
	}
}

int main(int argc, char** argv) {
	if (argc < 5) {
		return usage(argv[0]);
	}

	setresuid(0, 0, 0);

	kill_existing();

	printf("starting...\n");

	daemonize();
	signal(SIGINT, sighandler);
	signal(SIGTERM, sighandler);

	if (strcmp(argv[1], "hsv") == 0) {
		if (argc < 5) return usage(argv[0]);
		hsv_main(stof(argv[2]), stof(argv[3]), stof(argv[4]));

	} else if (strcmp(argv[1], "rgb") == 0) {
		if (argc < 5) return usage(argv[0]);
		rgb_main(stof(argv[2]), stof(argv[3]), stof(argv[4]));

	} else if (strcmp(argv[1], "particles") == 0) {
		if (argc < 3) return usage(argv[0]);
		float tmp = stof(argv[4]);
		int interval = 10000;
		if (tmp > 0.0001) {
			interval = 32.0 / tmp;
		}

		particles_main(interval);

	} else if (strcmp(argv[1], "waves") == 0) {
		if (argc < 4) return usage(argv[0]);
		waves_main(stof(argv[2]), stof(argv[3]));

	} else if (strcmp(argv[1], "bubbles") == 0) {
		if (argc < 5) return usage(argv[0]);
		bubbles_main(stof(argv[2]), stof(argv[3]), stof(argv[4]));
	} else if (strcmp(argv[1], "step") == 0) {
		if (argc < 6) return usage(argv[0]);
		struct hsv_t color = {
			.h = stof(argv[2]),
			.s = stof(argv[3]),
			.v = stof(argv[4])
		};
		simplestep_main(color, atoi(argv[5]));
	}

	/* turn off */
	struct apa102_led* l = apa102_open();
	if (l == NULL) return 1;

	struct apa102_led black;
	black.global = 0xe0 | 1;
	black.r = 0;
	black.g = 0;
	black.b = 0;

	for (int i = 0; i < NUM_LEDS; i++)
		l[i] = black;

	apa102_sync();
	apa102_close();

	printf("apa102 exiting\n");

	unlink("/tmp/apa102.pid");
}
