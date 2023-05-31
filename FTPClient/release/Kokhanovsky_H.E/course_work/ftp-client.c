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
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
#include <netinet/in.h>


// Constants
#define BUFFER_SIZE 512
#define MAX_INPUT_LENGTH 256
#define COMMAND_GETBACK "getback"




// Function to establish a TCP/IP connection with the server

int establishConnection(const char* server, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        return -1;  // Error
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server, &server_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        close(sockfd);  // Close the socket
        return -1;  // Error
    }

    if (connect(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect error");
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

int receiveResponse(int sockfd, char* response, size_t responseSize) {
    char buffer[256];
    ssize_t bytesRead = read(sockfd, buffer, sizeof(buffer));
    if (bytesRead < 0) {
        perror("Error receiving response");
        return -1;
    } else if (bytesRead == 0) {
        printf("Connection closed by the server.\n");
        return -1;
    } else {
        buffer[bytesRead] = '\0';
        strncpy(response, buffer, responseSize);
        printf("%s\n", buffer);
    }

    // Clear the buffer
    memset(buffer, 0, sizeof(buffer));

    return 0;
}

// Function to authenticate the user
int authenticate(int sockfd, const char* username, const char* password) {
    // Prepare the commands
    char userCommand[MAX_INPUT_LENGTH];
    snprintf(userCommand, sizeof(userCommand), "USER %s", username);

    char passCommand[MAX_INPUT_LENGTH];
    snprintf(passCommand, sizeof(passCommand), "PASS %s", password);

    // Send the username command to the server
    sendCommand(sockfd, userCommand);
    usleep(500000); // 100ms = 100,000 microseconds

    // Receive the response from the server
    char response[BUFFER_SIZE];
    if (receiveResponse(sockfd, response, sizeof(response)) != 0) {
        printf("Error receiving response from the server.\n");
        return -1;
    }

    // Send the password command to the server
    sendCommand(sockfd, passCommand);
    usleep(50000); // 100ms = 100,000 microseconds

    // Receive the response from the server
    if (receiveResponse(sockfd, response, sizeof(response)) != 0) {
        printf("Error receiving response from the server.\n");
        return -1;
    }

    return 0;
}


// Function to create a data connection
int createDataConnection(const char* serverIP, int port) {
    // Create a socket for the data connection
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (dataSocket < 0) {
        perror("Error creating data socket");
        return -1;  // Return an error code
    }

    // Set up the server address structure
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr)) <= 0) {
        perror("Invalid server IP address");
        close(dataSocket);
        return -1;  // Return an error code
    }

    // Connect to the server's data port
    if (connect(dataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error connecting to data port");
        close(dataSocket);
        return -1;  // Return an error code
    }

    return dataSocket;  // Return the data socket file descriptor
}

// Function to pack files using the default Linux archiver
int packFiles(const char* archiveFile) {
    char command[256];
    int result;

    // Check input validity
    if (archiveFile == NULL) {
        printf("Invalid input parameter.\n");
        return -1;
    }

    // Get the current working directory
    char currentDirectory[256];
    if (getcwd(currentDirectory, sizeof(currentDirectory)) == NULL) {
        perror("Error getting current directory");
        return -1;
    }

    // Construct the archiving command
    snprintf(command, sizeof(command), "tar -czvf %s %s/*", archiveFile, currentDirectory);

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
        int result = packFiles(archiveName);
        if (result != 0) {
            printf("Failed to pack the file '%s'.\n", fileName);
            return -1;
        }

        // Transfer the packed archive to the server using FTP protocol
        int filefd = open(archiveName, O_RDONLY);
        if (filefd < 0) {
            perror("Error opening file");
            return -1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead, bytesSent;
        while ((bytesRead = read(filefd, buffer, sizeof(buffer))) > 0) {
            bytesSent = write(sockfd, buffer, bytesRead);
            if (bytesSent < 0) {
                perror("Error sending file data");
                close(filefd);
                return -1;
            }
        }

        close(filefd);
        printf("Packed file '%s' sent to the server.\n", archiveName);
    } else {
        // Transfer the file to the server using FTP protocol
        int filefd = open(fileName, O_RDONLY);
        if (filefd < 0) {
            perror("Error opening file");
            return -1;
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytesRead, bytesSent;
        while ((bytesRead = read(filefd, buffer, sizeof(buffer))) > 0) {
            bytesSent = write(sockfd, buffer, bytesRead);
            if (bytesSent < 0) {
                perror("Error sending file data");
                close(filefd);
                return -1;
            }
        }

        close(filefd);
        printf("File '%s' sent to the server.\n", fileName);
    }

    return 0;
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
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead, bytesWritten;
    while ((bytesRead = read(sockfd, buffer, sizeof(buffer))) > 0) {
        bytesWritten = write(filefd, buffer, bytesRead);
        if (bytesWritten < 0) {
            perror("Error writing file data");
            close(filefd);
            return -1;
        }
    }

    close(filefd);
    printf("File '%s' received from the server.\n", fileName);

    return 0;
}

// Function to check if a file exists
bool fileExists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

// Function to create a unique log file
FILE* createLogFile() {
    // Get the current date
    time_t currentTime = time(NULL);
    struct tm* localTime = localtime(&currentTime);

    // Create the log file name based on the current date
    char logFileName[64];
    strftime(logFileName, sizeof(logFileName), "%Y%m%d.log", localTime);

    // Check if a file with the same name already exists
    if (fileExists(logFileName)) {
        // Generate a unique file name by appending a copy number
        int copyNumber = 1;
        char uniqueLogFileName[64];
        while (true) {
            snprintf(uniqueLogFileName, sizeof(uniqueLogFileName), "%s(%d).log", logFileName, copyNumber);
            if (!fileExists(uniqueLogFileName)) {
                break;
            }
            copyNumber++;
        }

        // Open the unique log file in append mode
        FILE* logFile = fopen(uniqueLogFileName, "a");
        if (logFile == NULL) {
            perror("Error opening log file");
            return NULL;
        }

        return logFile;
    }

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
int commandMode(int sockfd, FILE* logFile) {
    char input[MAX_INPUT_LENGTH];

    printf("Enter commands or '%s' to go back to the main menu.\n", COMMAND_GETBACK);

    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("Error reading input");
            continue;  // Continue the loop on error
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, COMMAND_GETBACK) == 0) {
            printf("Returning to the main menu.\n");
            return 0;
        }

        // Log the command to the log file
        if (logFile != NULL) {
            logEvent(logFile, input);
        }

        // Send the command to the server
        sendCommand(sockfd, input);
        usleep(500000); // 100ms = 100,000 microseconds

        // Clear the input buffer
        memset(input, 0, sizeof(input));

        // Receive and print the response from the server
        char serverResponse[BUFFER_SIZE];
        if (receiveResponse(sockfd, serverResponse, sizeof(serverResponse)) < 0) {
            printf("Error receiving server response.\n");
            continue;
        }
        printf("%s\n", serverResponse);

        // Check if the command is a file transfer command
        if (strncmp(input, "RETR", 4) == 0 || strncmp(input, "STOR", 4) == 0 ||
            strncmp(input, "APPE", 4) == 0 || strncmp(input, "STOU", 4) == 0) {

            // Extract the file name from the command
            char fileName[MAX_INPUT_LENGTH];
            if (sscanf(input, "%*s %s", fileName) != 1) {
                printf("Invalid file transfer command.\n");
                continue;
            }

            // Determine the file transfer command type
            int packFlag = 0;
            if (strncmp(input, "STOR", 4) == 0 || strncmp(input, "APPE", 4) == 0 || strncmp(input, "STOU", 4) == 0) {
                packFlag = 1;
            }

            // Extract the IP and port from the server's response
            char serverIP[256];
            int serverPort;
            if (sscanf(serverResponse, "%s %d", serverIP, &serverPort) != 2) {
                printf("Invalid server response.\n");
                continue;
            }

            // Create a new data connection socket
            int dataSocket = createDataConnection(serverIP, serverPort);
            if (dataSocket < 0) {
                printf("Error creating data connection.\n");
                continue;
            }

            // Send or receive the file based on the command type
            if (packFlag) {
                sendFile(fileName, packFlag, dataSocket);
            } else {
                receiveFile(fileName, dataSocket);
            }

            // Close the data connection socket
            close(dataSocket);
        }
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
    fgets(serverIP, sizeof(serverIP), stdin);
    sscanf(serverIP, "%s", serverIP); // Remove trailing newline character

    char portStr[10];
    printf("Enter the port: ");
    fgets(portStr, sizeof(portStr), stdin);
    sscanf(portStr, "%d", &port); // Convert port string to integer

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
    fgets(username, sizeof(username), stdin);
    sscanf(username, "%s", username); // Remove trailing newline character

    printf("Enter your password: ");
    fgets(password, sizeof(password), stdin);
    sscanf(password, "%s", password); // Remove trailing newline character

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
        char choiceStr[10];
        fgets(choiceStr, sizeof(choiceStr), stdin);
        sscanf(choiceStr, "%d", &choice); // Convert choice string to integer

        switch (choice) {
            case 1:
                // Re-authenticate the user
                printf("Enter your username: ");
                fgets(username, sizeof(username), stdin);
                sscanf(username, "%s", username); // Remove trailing newline character

                printf("Enter your password: ");
                fgets(password, sizeof(password), stdin);
                sscanf(password, "%s", password); // Remove trailing newline character

                authenticate(sockfd, username, password);
                logEvent(logFile, "Authentication successful.");
                break;
            case 2:
            {
                // Enter command mode
                int result = commandMode(sockfd, logFile);
                if (result != 0) {
                    // Display error message
                    fprintf(stderr, "Error occurred in command mode\n");
                    printf("Returning to the main menu.\n");
                }
                break;
            }
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
}