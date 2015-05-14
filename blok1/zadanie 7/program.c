#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

int main () {
	DIR *directory;
	char path[PATH_MAX];
	struct dirent *entry;

	if (getcwd(path, PATH_MAX) == NULL) {
		perror("getcwd");
		exit(EXIT_FAILURE);
	}
	
	if ((directory = opendir(path)) == NULL) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}

	while ((entry = readdir(directory)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			fprintf(stdout, "%s ", entry->d_name);
	}
	fprintf(stdout, "\n");

	if (closedir(directory) != 0) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}
	return 1;
}
