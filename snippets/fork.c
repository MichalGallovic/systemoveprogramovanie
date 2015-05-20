//NAJBEJZIKOVEJSI FORK S ODCHYTENIM STATUSU

int main()
{
	pid_t pid;
    int status;
    
    pid = fork();

    switch(pid) {
        case -1:
            perror("Error fork");
            return -1;
            break;
        case 0:
            write_to_file("udaje.txt",get_current_time());
            if(close(fd) == -1) {
                perror("Error close");
                return -1;
            }
            exit(EXIT_SUCCESS);
            break;
        default:
            wait(&status);
            if(WIFEXITED(status)) {
                print_file("udaje.txt");
            } else {
                fprintf(stderr,"Child exited with code %d", WEXITSTATUS(status));
            }
            break;
    }

} 