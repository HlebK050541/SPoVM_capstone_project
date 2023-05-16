//
// Created by Hleb Kokhanovsky
// Capstone Project
// FTP-client working on the TCP/IP protocol
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// Constants
#define DEFAULT_SERVER "ftp.example.com"
#define DEFAULT_PORT 21
#define DEFAULT_DIRECTORY "/home/ftp"

// Function to establish a TCP/IP connection with the server
int establishConnection(const char* server, int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket");
        return -1;  // Error
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server);
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("connect");
        return -1;  // Error
    }
    return sockfd;
}

// Function to authenticate with the server
int authenticate(int sockfd, const char* username, const char* password){
    char buffer[256];
    sprintf(buffer, "USER %s\r\n", username);
    send(sockfd, buffer, strlen(buffer), 0);

    recv(sockfd, buffer, sizeof(buffer), 0);
    if(strncmp(buffer, "331", 3) == 0){
        sprintf(buffer, "PASS %s\r\n", password);
        send(sockfd, buffer, strlen(buffer), 0);
        recv(sockfd, buffer, sizeof(buffer), 0);
        if(strncmp(buffer, "230", 3) == 0){
            return 0;  // Successful authentication
        }

    }
    return -1;  // Error

}

// Function to send commands to the server
int sendCommand(int sockfd, const char *command, const char *string) {
    send(sockfd, command, strlen(command), 0);
    return 0;  // Successful command
}

// Function to receive responses from the server
int receiveResponse(int sockfd){
    char buffer[256];
    recv(sockfd, buffer, sizeof(buffer), 0);
    return 0;  // Successful command
}

int sendData(int sockfd, char buffer[1024], size_t read) {
    send(sockfd, buffer, read, 0);
    return 0;
}

// Function to handle file upload
int uploadFile(int sockfd, const char* localFile, const char* remoteFile) {
    // Open the local file for reading
    FILE* file = fopen(localFile, "rb");
    if (file == NULL) {
        printf("Failed to open the local file for reading.\n");
        return -1;
    }

    // Send the STOR command to the server
    if (sendCommand(sockfd, "STOR %s\r\n", remoteFile) == 0) {
        int responseCode = receiveResponse(sockfd);
        if (responseCode == 150 || responseCode == 125) {
            // File upload is allowed, start sending the file content
            char buffer[1024];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                if (sendData(sockfd, buffer, bytesRead) < 0) {
                    printf("Failed to send file data.\n");
                    fclose(file);
                    return -1;
                }
            }

            fclose(file);
            responseCode = receiveResponse(sockfd);
            return responseCode;
        } else {
            printf("Server rejected the file upload.\n");
        }
    } else {
        printf("Failed to send the STOR command.\n");
    }

    fclose(file);
    return -1;
}


// Function to handle file download
int downloadFile(int sockfd, const char* remoteFile, const char* localFile){
    // Open the local file for writing
    FILE* file = fopen(localFile, "wb");
    if (file == NULL) {
        printf("Failed to open the local file for writing.\n");
        return -1;
    }

    // Send the RETR command to the server
    if (sendCommand(sockfd, "RETR %s\r\n", remoteFile) == 0) {
        int responseCode = receiveResponse(sockfd);
        if (responseCode == 150 || responseCode == 125) {
            // File download is allowed, start receiving the file content
            char buffer[1024];
            size_t bytesRead;

            while ((bytesRead = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
                if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
                    printf("Failed to write file data.\n");
                    fclose(file);
                    return -1;
                }


            }
            fclose(file);
            responseCode = receiveResponse(sockfd);
            return responseCode;
        } else {
            printf("Server rejected the file download.\n");
        }
    } else {
        printf("Failed to send the RETR command.\n");
    }
    fclose(file);
    return -1;
}


// Function to create a directory on the server
int createDirectory(int sockfd, const char* directory) {
    // Send the MKD command to the server
    if (sendCommand(sockfd, "MKD %s\r\n", directory) == 0) {
        int responseCode = receiveResponse(sockfd);
        if (responseCode == 257) {
            printf("Directory '%s' created successfully.\n", directory);
            return 0;
        } else {
            printf("Failed to create directory '%s'.\n", directory);
        }
    } else {
        printf("Failed to send the MKD command.\n");
    }

    return -1;
}


// Function to pack files using the default Linux archiver
int packFiles(const char* archiveFile, const char* sourceDirectory) {
    char command[256];
    snprintf(command, sizeof(command), "tar -czvf %s %s", archiveFile, sourceDirectory);

    // Execute the archiving command
    int result = system(command);
    if (result == 0) {
        printf("Files packed successfully into '%s'.\n", archiveFile);
        return 0;
    } else {
        printf("Failed to pack files into '%s'.\n", archiveFile);
    }

    return -1;
}


// Function to unpack files using the default Linux archiver
int unpackFiles(const char* archiveFile, const char* destinationDirectory);

// Function to handle the ABOR command
void handleABOR()
{
    // Implementation for ABOR command
    printf("ABOR command executed\n");
}

// Function to send the CWD command
int changeWorkingDirectory(int sockfd, const char* directory) {
    char command[256];
    snprintf(command, sizeof(command), "CWD %s\r\n", directory);
    if (sendCommand(sockfd, command, NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}

// Function to send the LIST command
int listFiles(int sockfd) {
    if (sendCommand(sockfd, "LIST\r\n", NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}

// Function to send the RETR command
int retrieveFile(int sockfd, const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "RETR %s\r\n", filename);
    if (sendCommand(sockfd, command, NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}

// Function to send the STOR command
int storeFile(int sockfd, const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "STOR %s\r\n", filename);
    if (sendCommand(sockfd, command, NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}

// Function to send the DELE command
int deleteFile(int sockfd, const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "DELE %s\r\n", filename);
    if (sendCommand(sockfd, command, NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}

// Function to send the HELP command
int sendHelpCommand(int sockfd) {
    if (sendCommand(sockfd, "HELP\r\n", NULL) == 0) {
        return receiveResponse(sockfd);
    }
    return -1;
}


int main() {
    // Entry point of the FTP client program
    // ...

    return 0;
}

