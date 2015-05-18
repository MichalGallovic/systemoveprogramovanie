#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//detsky proces
void child() {
	printf("child: \n"); //pid = %d, ppid = %d\n"
}

//rodicovsky proces
//vstup:
//  childPid - cislo detskeho procesu
void parent(pid_t childPid) {
	printf("parent: \n"); //pid = %d, childPid = %d\n"
}

int main()
{
	pid_t pid;


		//child();
		//exit(EXIT_SUCCESS);

		//parent(DOPLNIT);
		//exit(EXIT_SUCCESS);

}
