#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

typedef enum 
{
    Empty = 0,
    NonEmpty,
    All
} ChoiceType;

void runInPipe(char *path, ChoiceType choice)
{
    int fds[2]; //implement PIPE
    int status;
    pipe(fds);
    
    //two children do the job
    //does not metter in which sequence
    if (fork() == 0) 
    {
        close(0);
        dup(fds[0]); // redirect standard input to fds[0]
        close(fds[1]); // close unused end of pipe
        if (execlp("wc","wc","-l",NULL) == -1)
        {
            printf("wc failed\n");
        }
        exit(0);
    } 
    if (fork() == 0) 
    {
        close(1);
        dup(fds[1]);    // redirect standard output to fds[1];
        close(fds[0]);  // close unused end of pipe
        switch(choice)
        {
            case Empty:
            {
                fprintf(stderr,"Count of empty directories: \n");
                if(execlp("find","find",path,"-type","d","-empty",NULL) == -1)
                {
                    printf("find empty failed\n");
                }
                break;
            }
            case NonEmpty:
            {
                fprintf(stderr,"Count of nonempty directories: \n");
                if(execlp("find","find",path,"-not","-type","d","-empty",NULL) == -1)
                {
                    printf("find non-empty failed\n");
                }
                break;
            }
            case All:
            {
                fprintf(stderr,"Count of all directories: \n");
                if(execlp("find","find",path,"-type","d",NULL) == -1)
                {
                    printf("find all failed\n");
                }
                break;
            }
            default:
            {
                fprintf(stderr,"error!!");
                break;
            }
        }
        exit(0); 
    }
    close(0);
    close(1);           // parent
    waitpid(-1, &status, 0);
    // parent is waiting for both childre)
}

void handlePath(char *path, int showEmpty)
{
	if (showEmpty)
	{
		fprintf(stderr,"path is: %s\n",path);
        fprintf(stderr,"Show all empty directories within a path:\n");
        execlp("find","find",path,"-type","d","-empty",NULL);
	}
}

void handleHelp()
{
    fprintf(stderr,"Usage:\n");
    fprintf(stderr,"./zadanie_13  [-m | --match] [-n | --no_match] [-a | --all] <directory>\n");
    fprintf(stderr,"-h or --help     - shows help\n");
    fprintf(stderr,"-m or --match    - counts all empty directories\n");
    fprintf(stderr,"-n or --no_match - counts all non-empty directories\n");
    fprintf(stderr,"-a or --all      - counts both empty and non-empty  directories\n");
    fprintf(stderr,"only <path>      - show all empty directories\n");
    exit(0);
}

void parseArguments(int argc, char *argv[])
{
	int opt;
	const char *short_params = ":hmna";
	const struct option long_params[] = {
		{"help", no_argument, NULL, 'h'},
		{"match", no_argument, NULL, 'm'},
		{"no_match", no_argument, NULL, 'n'},
		{"all", no_argument, NULL, 'a'},
		{NULL, 0, NULL, 0}
	};
	
	int option_index = -1;
	
	int showEmptyDirectory = 1; 	

	while ((opt = getopt_long(argc, argv, short_params, long_params, &option_index)) != -1)
	{
		switch(opt)
		{
            case('h'):
            {
                handleHelp(); 
                break;
            }
			case('m'):
			{
				showEmptyDirectory = 0; 
                
                //command to count all empty directories
                //find . -type d -empty | wc -l
                runInPipe(argv[optind],Empty); 
			}
			case('n'):
			{
				showEmptyDirectory = 0; 

                //command to count all non-empty directories
                //find . -type d -not -empty | wc -l
                runInPipe(argv[optind],NonEmpty);
				break;
			}
			case('a'):
			{
				showEmptyDirectory = 0; 
                
                //command to count all non-empty directories
                //find . -type d | wc -l
                runInPipe(argv[optind],All);
				break;			
			}
			case('?'):
			{
				fprintf(stderr,"unknown symbol\n");
				exit(EXIT_FAILURE);
				break;
			}				
		}	
	}
	
    if (argv[optind] != NULL)
    {
		 handlePath(argv[optind],showEmptyDirectory);
	}
    else
    {
        handleHelp();
        exit(0);
	}
}

int main(int argc, char *argv[])
{
    parseArguments(argc,argv);

	return 0;
}
