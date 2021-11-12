// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    const char *nobody = "nobody";
    struct passwd *nobody_user;
    int val;

    pid_t present_pid, parent_pid;
    int nobody_uid;
    present_pid = getpid();

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //creating a child process and adding the read and send functions into it

    printf("\nThis is beginning of privilege separation\n");

    present_pid = fork();

    if (present_pid == 0)
    {
        printf("\nWe have created a child process\n");
        printf("\nCurrently reading from the client \n");

        nobody_user = getpwnam(nobody);
        if (nobody_user == NULL){
            perror("Unable to find the UID for nobody user");
            return 0;
        }
        nobody_uid = nobody_user->pw_uid;
        printf("\n The user id is nobody user :%d", nobody_uid);

        val = setuid(nobody_uid);

        printf("\n Value of nobody user's uid is: %d\tpresent_uid: %d\n", val, getuid());

        if (val == -1)
        {
            printf("\n Error while trying to drop privilege\n");
            return 0;
        }
        valread = read(new_socket, buffer, 1024);
        printf("\nRead %d bytes: %s\n", valread, buffer);
        send(new_socket, hello, strlen(hello), 0);
        printf("\nHello message sent\n");
        printf("\nChild -> \t present_user_id: %d\n",getuid());

    }
    else if(present_pid > 0)
    {
        wait(NULL);
        printf("\nPassing the baton to the parent\n");
    }
    else
    {
        perror("\n forking process failed \n");
        _exit(2);
    }
    printf("\n Returned pid: %d\n",parent_pid);
    printf("\n End of the process of privilege separation \n");
    return 0;
}
