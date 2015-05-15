#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 50

int logFile_ = 0;
char logFile[SIZE];
int prog_ = 0;
char prog[SIZE];
int copy_ = 0;

int child (int pipefd[]) {
	pid_t pid;
	close(pipefd[0]);
	
	fprintf(stdout, "Vsetky spustitelne subory pod aktualnym adresarom:\n");
	//execlp
	pid = fork();
	switch (pid) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			execlp("find", "find", "-executable", NULL);
		default:
			waitpid(pid, NULL, 0);
			break;
	}
	

	fprintf(stdout, "Zadaj meno suboru ktory chces spustit: ");
	scanf("%49s", (char*)&prog);

	if (kill(getppid(), SIGUSR1) != 0) {
		perror("kill");
		exit(EXIT_FAILURE);
	}

	umask(0);

	write(pipefd[1], &prog, sizeof(prog));

	close(pipefd[1]);

	return 1;
}

void getProgFromChild() {
	int pipefd[2];
	pid_t pid;
	char buf;
	sigset_t set;
	int sig_num;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	if (sigprocmask(SIG_SETMASK, &set, NULL) != 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}
	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	pid = fork();
	switch(pid){
		case 0:
			child(pipefd);
			exit(EXIT_SUCCESS);
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		default:
			if (sigwait(&set, &sig_num) != 0) {
				perror("sigwait");
				exit(EXIT_FAILURE);
			}
			break;
	}

	umask(0);

	close(pipefd[1]);

	if (read(pipefd[0], &prog, sizeof(prog)) != sizeof(prog)) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	close(pipefd[0]);
}

void parseArgs(int argc, char *argv[]){
	int c;
	int option_index = 0;
	static struct option long_options[] = {
		{"prog", 1, 0, 'p'},
		{"logfile", 1, 0, 'l'},
		{"copy", 0, 0, 'c'},
		{0, 0, 0, 0}
	};

	do {
		c = getopt_long(argc, argv, "p:l:c", long_options, &option_index);
		
		switch (c) {
		case 'p':
			prog_ = 1;
			//prog = optarg;
			strncpy(prog, optarg, SIZE);
			printf("Volba p s hodnotou %s\n", prog);
                   	break;
		case 'l':
			logFile_ = 1;
			//logFile = optarg;
			strncpy(logFile, optarg, SIZE);
			printf("Volba l s hodnotou %s\n", logFile);
		        break;
		case 'c':
			printf("Volba c\n");
			break;
		case '?':
			fprintf(stderr, "Neznama volba -%c\n", optopt);
			exit(EXIT_FAILURE);
		default:
			break;
		}
	} while (c != -1);

	if (!prog_) {
		getProgFromChild();
	}
}

void runProg() {
	if (logFile_) {
		int file = open(logFile, O_WRONLY | O_CREAT | O_TRUNC, 660);
		if (file == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		close(STDOUT_FILENO);
		dup2(file, STDOUT_FILENO);
		close(file);
	}

	pid_t pid = fork();
	switch (pid) {
		case -1:
			perror("fork");
			exit(EXIT_FAILURE);
		case 0:
			execl(prog, prog, NULL);
			perror("exec");
		default:
			waitpid(pid, NULL, 0);
			break;
	}
	if (logFile_) {
		
		close(STDOUT_FILENO);
	}
}

int main(int args, char *argv[]){
	parseArgs(args, argv);
	runProg();

	return 	EXIT_SUCCESS;
}
