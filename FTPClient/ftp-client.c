//
// Created by Hleb Kokhanovsky
// Capstone Project
// FTP-client working on the TCP/IP protocol
// The client is a simple program that sends commands to the server and receives responses from the server.
// The client supports working with the archived files
// The client supports uploading and downloading files
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

// Constants
#define DEFAULT_SERVER "127.0.0.1"
#define DEFAULT_PORT 8080
#define BUFFER_SIZE 512
#define MAX_INPUT_LENGTH 256
#define COMMAND_GETBACK "getback"


// Function to establish a TCP/IP connection with the server
int establishConnection(const char* server, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;  // Error
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);  // Close the socket
        return -1;  // Error
    }

    return sockfd;
}

// Function to send commands to the server
int sendCommand(int sockfd, const char* command) {
    // Prepare the command to be sent to the server
    char commandString[256];
    snprintf(commandString, sizeof(commandString), "%s\r\n", command);

    // Send the command to the server
    ssize_t bytesSent = write(sockfd, commandString, strlen(commandString));

    // Check if the command was successfully sent
    if (bytesSent < 0) {
        perror("Error sending command");
        return -1;  // Return an error code
    } else {
        return 0;  // Return success code
    }
}

// Function to receive responses from the server
int receiveResponse(int sockfd) {
    char buffer[256];
    ssize_t bytesRead = read(sockfd, buffer, sizeof(buffer));
    if (bytesRead < 0) {
        perror("Error receiving response");
    } else if (bytesRead == 0) {
        printf("Connection closed by the server.\n");
    } else {
        buffer[bytesRead] = '\0';
        printf("%s\n", buffer);
    }
    return 0;
}

// Function to authenticate the user
int authenticate(int sockfd, const char* username, const char* password) {
    char buffer[BUFFER_SIZE];

    // Send the username command to the server
    sendCommand(sockfd, "USER");

    // Send the username to the server
    sendCommand(sockfd, username);

    // Receive the response from the server
    receiveResponse(sockfd);

    // Send the password command to the server
    sendCommand(sockfd, "PASS");

    // Send the password to the server
    sendCommand(sockfd, password);

    // Receive the response from the server
    receiveResponse(sockfd);

    return 0;
}

/**
// Function to send commands to the server
void commandMode(int sockfd) {
    char input[MAX_INPUT_LENGTH];

    printf("Enter commands or '%s' to go back to the main menu.\n", COMMAND_GETBACK);

    while (1) {
        printf("> ");
        fgets(input, sizeof(input), stdin);

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, COMMAND_GETBACK) == 0) {
            printf("Returning to the main menu.\n");
            break;
        }

        // Send the command to the server
        sendCommand(sockfd, input);

        // Receive and print the response from the server
        receiveResponse(sockfd);
    }
}**/


// Function to pack files using the default Linux archiver
int packFiles(const char* archiveFile, const char* sourceDirectory) {
    char command[256];
    int result;

    // Check input validity
    if (archiveFile == NULL || sourceDirectory == NULL) {
        printf("Invalid input parameters.\n");
        return -1;
    }

    // Construct the archiving command
    snprintf(command, sizeof(command), "tar -czvf %s %s", archiveFile, sourceDirectory);

    // Execute the archiving command
    result = system(command);
    if (result == 0) {
        printf("Files packed successfully into '%s'.\n", archiveFile);
        return 0;
    } else {
        printf("Failed to pack files into '%s'.\n", archiveFile);
        return -1;
    }
}

// Function to unpack files using the default Linux archiver
int unpackFiles(const char* archiveFile, const char* destinationDirectory) {
    char command[256];
    int result;

    // Check input validity
    if (archiveFile == NULL || destinationDirectory == NULL) {
        printf("Invalid input parameters.\n");
        return -1;
    }

    // Construct the command to unpack the archive file to the destination directory
    snprintf(command, sizeof(command), "tar -xf %s -C %s", archiveFile, destinationDirectory);

    // Execute the command using the system function
    result = system(command);

    if (result == -1) {
        perror("Error executing unpack command");
        return -1;
    } else if (result != 0) {
        printf("Failed to unpack files\n");
        return -1;
    }

    return 0;
}


// Function to send a file, with an optional flag to indicate if it should be packed
int sendFile(const char* fileName, int packFlag, int sockfd) {
    if (fileName == NULL) {
        printf("Invalid file name.\n");
        return -1;
    }

    // Check if packing is requested
    if (packFlag) {
        // Pack the file into a tar.gz archive
        char archiveName[256];
        snprintf(archiveName, sizeof(archiveName), "%s.tar.gz", fileName);
        int result = packFiles(archiveName, fileName);
        if (result != 0) {
            printf("Failed to pack the file '%s'.\n", fileName);
            return -1;
        }

        // Transfer the packed archive to the server using FTP protocol
        int filefd = open(archiveName, O_RDONLY);
        if (filefd < 0) {
            perror("Error opening file");
            return -1;  // Return an error code
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead, bytesSent;
        while ((bytesRead = read(filefd, buffer, sizeof(buffer))) > 0) {
            bytesSent = write(sockfd, buffer, bytesRead);
            if (bytesSent < 0) {
                perror("Error sending file data");
                close(filefd);
                return -1;  // Return an error code
            }
        }

        close(filefd);
        printf("Packed file '%s' sent to the server.\n", archiveName);
    } else {
        // Transfer the file to the server using FTP protocol
        int filefd = open(fileName, O_RDONLY);
        if (filefd < 0) {
            perror("Error opening file");
            return -1;  // Return an error code
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead, bytesSent;
        while ((bytesRead = read(filefd, buffer, sizeof(buffer))) > 0) {
            bytesSent = write(sockfd, buffer, bytesRead);
            if (bytesSent < 0) {
                perror("Error sending file data");
                close(filefd);
                return -1;  // Return an error code
            }
        }

        close(filefd);
        printf("File '%s' sent to the server.\n", fileName);
    }

    return 0;  // Return success code
}



// Function to receive a file from the server
int receiveFile(const char* fileName, int sockfd) {
    if (fileName == NULL) {
        printf("Invalid file name.\n");
        return -1;
    }

    // Create a new file to save the received data
    int filefd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (filefd < 0) {
        perror("Error creating file");
        return -1;  // Return an error code
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead, bytesWritten;
    while ((bytesRead = read(sockfd, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(filefd, buffer, bytesRead);
        if (bytesWritten < 0) {
            perror("Error writing file data");
            close(filefd);
            return -1;  // Return an error code
        }
    }

    close(filefd);
    printf("File '%s' received from the server.\n", fileName);

    return 0;  // Return success code
}



// Function to create the log file
FILE* createLogFile() {
    // Get the current date
    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);

    // Create the log file name based on the current date
    char logFileName[64];
    strftime(logFileName, sizeof(logFileName), "%Y%m%d.log", localTime);

    // Open the log file in append mode
    FILE* logFile = fopen(logFileName, "a");
    if (logFile == NULL) {
        perror("Error opening log file");
        return NULL;
    }

    return logFile;
}

// Function to log an event
void logEvent(FILE* logFile, const char* event) {
    if (logFile == NULL) {
        printf("Log file is not available.\n");
        return;
    }

    // Get the current time
    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);

    // Get the current time in the desired format
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    // Write the event to the log file with the timestamp
    fprintf(logFile, "[%s] %s\n", timestamp, event);
}


// Function to enter command mode
void commandMode(int sockfd, const char* logFileName) {
    char input[MAX_INPUT_LENGTH];
    FILE* logFile = NULL;

    // Open the log file in append mode
    if (logFileName != NULL) {
        logFile = fopen(logFileName, "a");
        if (logFile == NULL) {
            perror("Error opening log file");
            return;
        }
    }

    printf("Enter commands or '%s' to go back to the main menu.\n", COMMAND_GETBACK);

    while (1) {
        printf("> ");
        fgets(input, sizeof(input), stdin);

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, COMMAND_GETBACK) == 0) {
            printf("Returning to the main menu.\n");
            break;
        }

        // Log the command to the log file
        if (logFile != NULL) {
            logEvent(logFile, input);
        }

        // Send the command to the server
        sendCommand(sockfd, input);

        // Receive and print the response from the server
        receiveResponse(sockfd);

        // Check if the command is a file transfer command
        if (strncmp(input, "RETR", 4) == 0 || strncmp(input, "STOR", 4) == 0 ||
            strncmp(input, "APPE", 4) == 0 || strncmp(input, "STOU", 4) == 0) {

            // Extract the file name from the command
            char fileName[MAX_INPUT_LENGTH];
            sscanf(input, "%*s %s", fileName);

            // Determine the file transfer command type
            int packFlag = 0;
            if (strncmp(input, "STOR", 4) == 0 || strncmp(input, "APPE", 4) == 0 || strncmp(input, "STOU", 4) == 0) {
                packFlag = 1;
            }

            // Send or receive the file based on the command type
            if (packFlag) {
                sendFile(fileName, packFlag, sockfd);
            } else {
                receiveFile(fileName, sockfd);
            }
        }
    }

    // Close the log file
    if (logFile != NULL) {
        fclose(logFile);
    }
}


// Function to close the TCP/IP connection with the server
void closeConnection(int sockfd) {
    close(sockfd);
}


int main() {
    char serverIP[50];
    int port;

    // Prompt for server's IP address and port
    printf("Enter the server's IP address: ");
    scanf("%s", serverIP);
    printf("Enter the port: ");
    scanf("%d", &port);

    // Establish the TCP/IP connection with the server
    int sockfd = establishConnection(serverIP, port);
    if (sockfd < 0) {
        printf("Failed to establish a connection with the server.\n");
        return -1;
    }

    // Create the log file
    FILE* logFile = createLogFile();
    if (logFile == NULL) {
        printf("Failed to create the log file.\n");
        closeConnection(sockfd);
        return -1;
    }

    // Authenticate the user
    char username[MAX_INPUT_LENGTH];
    char password[MAX_INPUT_LENGTH];
    printf("Enter your username: ");
    scanf("%s", username);
    printf("Enter your password: ");
    scanf("%s", password);
    authenticate(sockfd, username, password);
    logEvent(logFile, "Authentication successful.");

    int choice;
    do {
        // Display menu and prompt for choice
        printf("\n--- Menu ---\n");
        printf("1. Authentication\n");
        printf("2. Command Mode\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Re-authenticate the user
                printf("Enter your username: ");
                scanf("%s", username);
                printf("Enter your password: ");
                scanf("%s", password);
                authenticate(sockfd, username, password);
                logEvent(logFile, "Authentication successful.");
                break;
            case 2:
                // Enter command mode
                commandMode(sockfd, logFile);
                break;
            case 3:
                // Exit the program
                printf("Closing connection and exiting...\n");
                logEvent(logFile, "Exiting the program.");

                // Close the connection with the server
                closeConnection(sockfd);

                // Close the log file
                fclose(logFile);

                // Exit the program
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (1);

    return 0;
}