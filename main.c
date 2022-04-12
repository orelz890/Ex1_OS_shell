#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// getcwd
#include <unistd.h>
#include <sys/wait.h>
// socket
#include <sys/socket.h>
#include <arpa/inet.h>
// dir funcs
#include <dirent.h>


#define IP "127.0.0.1"
#define PORT 6060
#define ERROR -1
#define SUCCESS 0



bool local = true; // changes to false when TCP connection has been established
int sock;
char input[1000];
size_t outputsize = 1024;
char **output;
char cwd[1024];

// Was aided by this site:
// https://www.geeksforgeeks.org/socket-programming-cc/
void create_connection(){
    if(!local){
        printf("Already connected..\n");
        return;
    }
    struct sockaddr_in server_address;
    // Create an endpoint for communication
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == ERROR){
        perror("Error in create_connection - socket error\n");
        return;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    int convertion = inet_pton(AF_INET, IP, &server_address.sin_addr);
    if (convertion < 1){
        if (convertion == 0){
            fprintf(stderr, "Error in create_connection - inet_pton - Not in presentation format\n");
        }
        else{
            perror("Error - inet_pton\n");
        }
        return;
    }
    int connection = connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    if (connection == ERROR){
        perror("Error in create_connection - connection Failed\n");
        return;
    }
    // Set current state as client-server
    local = false;
}

void echo(){
    output = (char **)malloc((sizeof(char) * outputsize));
    memset(output,0,sizeof(output));
    getline(output, &outputsize, stdin);
    if (local){
        printf("%s", *output);
    }
    else{
        send(sock, *output, strlen(*output), 0);
        sleep(1);
    }
    free(output);
}

// chdir - is a system call (kernal).
void change_directory(){

    char dirname[256];
    scanf("%s", dirname);
    if (chdir(dirname) == -1){
        printf("Error - change_directory\n");
        return;
    }
    // Insert the path string
    getcwd(cwd, sizeof(cwd));
}

// Was aided by this site: 
// https://stackhowto.com/c-program-to-list-all-files-in-a-directory/
void read_current_directory(){

    DIR *d = opendir(cwd);
    if (d == NULL){
        printf("Cannot open directory '%s'\n", cwd);
        return;
    }
    struct dirent *dir;
    dir = readdir(d);
    char ans[1024];
    memset(ans,0,sizeof(ans));

    while (dir != NULL){
        strcat(ans,dir->d_name);
        strcat(ans," ");

        dir = readdir(d);
    }
    if (!local){
        send(sock, ans, strlen(ans), 0);
        sleep(1);
    }
    else{
        printf("%s\n",ans);
    }
    closedir(d);
}


// We were aided by this site in this func:
// https://www.geeksforgeeks.org/c-program-copy-contents-one-file-another-file/

// chdir is a system call, But fopen\fgetc\.. are library functions.
void copy_file_to_dest(){
    FILE *file_pointer1, *file_pointer2;
    char file_name[100], file_name2[100],folder_name[100], prev_folder_name[256], c;
  
    printf("Enter an existing filename & Chose the new file name\n");
    scanf("%s", file_name);
    scanf("%s", file_name2);
  
    // Open one file for reading
    file_pointer1 = fopen(file_name, "r");
    if (file_pointer1 == NULL){
        printf("Cannot open file %s \n", file_name);
        return;
    }

    printf("Chose the dst folder name\n");
    scanf("%s", folder_name);

    //save the current folder path
    getcwd(prev_folder_name,sizeof(prev_folder_name));
    // Enter the dst folder given by the user
    int inTheFolder = chdir(folder_name);
    if (inTheFolder != 0){
        printf("Destenation folder don't exist, the file will be copied to the current folder\n");
    }
    
    // Open another file for writing
    file_pointer2 = fopen(file_name2, "w");
    if (file_pointer2 == NULL){
        printf("Cannot open file %s \n", file_name2);
        fclose(file_pointer1);
        return;
    }
  
    // Read contents from file
    c = fgetc(file_pointer1);
    while (c != EOF){
        fputc(c, file_pointer2);
        c = fgetc(file_pointer1);
    }
  
    printf("\nFile %s seccessfully copied to %s\n", file_name, file_name2);

    // Return to the prev folder where we came from
    chdir(prev_folder_name);
    fclose(file_pointer1);
    fclose(file_pointer2);
}


// unlink is a system-call.
void delete_file(){

    char file_name[100];
    scanf("%s", file_name);
    if(unlink(file_name) != 0){
        printf("ERROR in delete_file\n");
        return;
    }
    printf("File %s deleted!\n", file_name);

}


int do_action(){

    if (!strcmp(input, "ECHO")){
        echo();
    }
    else if (!strcmp(input, "TCP")){
        create_connection();
    }
    else if (!strcmp(input, "LOCAL")){
        char* messege = "A client disconnected\n";
        send(sock, messege, strlen(messege), 0);
        local = true;
        close(sock);
    }
    else if (!strcmp(input, "DIR")){
        read_current_directory();
    }
    else if (!strcmp(input, "CD")){
        change_directory();
    }
    else if (!strcmp(input, "COPY")){
        copy_file_to_dest();
    }
    else if (!strcmp(input, "DELETE")){
        delete_file();
    }
    else{
        return -1;
    }
    return 0;
}

int main(){
    char temp[10];
    while (1){
        memset(input,0,sizeof(input));
        memset(cwd,0,sizeof(cwd));
        memset(temp,0,sizeof(temp));

        getcwd(cwd, sizeof(cwd));
        printf("%s YES MASTER SPLINTER# ", cwd);
        // scanf("%s", input);
        scanf("%s", input);
        if (!strcmp(input, "TCP")){
            scanf("%[^\n]s", temp);
        }
        if (!strcmp(input, "EXIT")){
            break;
        }
        int act = do_action();
        
        if(act == -1){
            char *commends_storage[128];
            char **modifiable_cs = commends_storage;
            // Allocate dinamic memory for the getline func
            char **out_ = (char **)malloc((sizeof(char) * outputsize));
            getline(out_, &outputsize, stdin);
            char *line = strcat(input, *out_);
            // Free the allocated space
            free(out_);

            // system(line); --->  System is a library function

            printf("unkown command, uses system..\n");
            char *temp = strtok(line, " \n");
            while (temp != NULL){
                *modifiable_cs = temp;
                temp = strtok(NULL, " \n");
                modifiable_cs++;
            }
            *modifiable_cs = NULL;

            // Create a child process
            int pid = fork();
            // Something went wrong
            if (pid == -1){
                exit(1);
            }
            // Creation was a success
            else if (pid == 0){
                if (execvp(commends_storage[0], commends_storage) == -1){
                    printf("exevp failed\n");
                }
                break;
            }
            // Wait for a response
            wait(NULL);
        }
    }

    return 1;
}