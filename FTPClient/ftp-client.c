//
// Created by Hleb Kokhanovsky
// Capstone Project
// FTP-client working on the TCP/IP protocol
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// Constants
#define DEFAULT_SERVER "192.168.100.10"
#define DEFAULT_PORT 21
#define BUFFER_SIZE 512
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

// Function to close the TCP/IP connection with the server
void closeConnection(int sockfd) {
    close(sockfd);
}


// Function to send commands to the server
int sendCommand(int sockfd, const char* command, const char* string) {
    // Prepare the command to be sent to the server
    char commandString[256];
    snprintf(commandString, sizeof(commandString), "%s %s\r\n", command, string);

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
int receiveResponse(int sockfd, char string[256], unsigned long i) {
    char buffer[256];
    ssize_t bytesRead = read(sockfd, buffer, sizeof(buffer));
    if (bytesRead < 0) {
        perror("Error receiving response");
    } else if (bytesRead == 0) {
        printf("Connection closed by the server.\n");
    } else {
        buffer[bytesRead] = '\0';
        if (strncmp(buffer, string, strlen(string)) == 0) {
            printf("%lu: %s\n", i, buffer);
        } else {
            printf("%lu: %s\n", i, buffer);
            printf("%s\n", string);
            return -1;
        }

        }
    return 0;
}

// Function to authenticate with the server
int authenticate(int sockfd, const char* username, const char* password) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    // Send the username and password to the server
    sendCommand(sockfd, "USER", username);
    // Receive the response from the server
    receiveResponse(sockfd, buffer, 0);
    // Send the password to the server
    sendCommand(sockfd, "PASS", password);
    // Receive the response from the server
    receiveResponse(sockfd, buffer, 1);
    return 0;
}

// Function to send data to the server
int sendData(int sockfd, const void* data, size_t size) {
    ssize_t bytesSent = write(sockfd, data, size);
    if (bytesSent < 0) {
        perror("Error sending data");
    } else if (bytesSent != size) {
        printf("Error sending data\n");
    }
    return 0;
}

int receiveData(int sockfd, void* data, size_t size) {
    ssize_t bytesRead = read(sockfd, data, size);
    if (bytesRead < 0) {
        perror("Error receiving data");
    } else if (bytesRead != size) {
        printf("Error receiving data\n");
    }
    return 0;
}
// Function to handle file upload
int uploadFile(int sockfd, const char* localFile, const char* remoteFile) {
    // Open the local file for reading
    FILE* file = fopen(localFile, "rb");
    if (file == NULL) {
        printf("Failed to open the local file for reading.\n");
        return -1; // Return an error code
    }
    // Send the STOR command to the server
    char command[256];
    int commandSize = snprintf(command, sizeof(command), "STOR %s\r\n", remoteFile);
    if (commandSize < 0 || commandSize >= sizeof(command)) {
        printf("Error constructing STOR command.\n");
        fclose(file);
        return -1; // Return an error code
    }
    if (sendCommand(sockfd, command, NULL) == 0) {
        // File upload command sent successfully

        // Handle the file upload process
        char buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            // Write the buffer to the server
            if (sendData(sockfd, buffer, bytesRead) != 0) {
                printf("Failed to send data to the server.\n");
                fclose(file);
                return -1; // Return an error code
            }
        }
        // Close the local file
        fclose(file);
        return 0; // Return success code
    } else {
        printf("Failed to send the STOR command.\n");
        fclose(file);
        return -1; // Return an error code
    }
}
// Function to handle file download
int downloadFile(int sockfd, const char* remoteFile, const char* localFile) {
    // Send the RETR command to the server
    char command[256];
    int commandSize = snprintf(command, sizeof(command), "RETR %s\r\n", remoteFile);
    if (commandSize < 0 || commandSize >= sizeof(command)) {
        printf("Error constructing RETR command.\n");
        return -1; // Return an error code
    }

    if (sendCommand(sockfd, command, NULL) == 0) {
        // File download command sent successfully
        // Handle the file download process
        FILE* file = fopen(localFile, "wb");
        if (file == NULL) {
            printf("Failed to open the local file for writing.\n");
            return -1; // Return an error code
        }
        char buffer[1024];
        ssize_t bytesReceived;
        while ((bytesReceived = receiveData(sockfd, buffer, sizeof(buffer))) > 0) {
            // Write received data to the local file
            if (fwrite(buffer, 1, bytesReceived, file) != bytesReceived) {
                printf("Failed to write data to the local file.\n");
                fclose(file);
                return -1; // Return an error code
            }
        }
        // Close the local file
        fclose(file);
        return 0; // Return success code
    } else {
        printf("Failed to send the RETR command.\n");
        return -1; // Return an error code
    }
}
// Function to create a directory on the server
int createDirectory(int sockfd, const char* directory) {
    // Send the MKD command to the server
    if (sendCommand(sockfd, "MKD %s\r\n", directory) == 0) {
        int responseCode = receiveResponse(sockfd, NULL, 0);
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
int unpackFiles(const char* archiveFile, const char* destinationDirectory) {
    // Construct the command to unpack the archive file to the destination directory
    char command[256];
    snprintf(command, sizeof(command), "tar -xf %s -C %s", archiveFile, destinationDirectory);

    // Execute the command using the system function
    int result = system(command);

    if (result == -1) {
        perror("Error executing unpack command");
        return -1;  // Return an error code
    } else if (result != 0) {
        printf("Failed to unpack files\n");
        return -1;  // Return an error code
    }

    return 0;  // Return success code
}


bool startsWith(const char* string, const char* substring) {
    return strncmp(string, substring, strlen(substring)) == 0;
}

// Function to parse the PASV response and extract the IP address and port
int parsePASVResponse(const char* response, char* ipAddress, int* port) {
    // Find the opening parenthesis
    const char* openingParenthesis = strchr(response, '(');
    if (openingParenthesis == NULL) {
        return -1; // Return an error code
    }
    // Find the closing parenthesis
    const char* closingParenthesis = strchr(response, ')');
    if (closingParenthesis == NULL) {
        return -1; // Return an error code
    }
    // Extract the numbers between the parentheses
    char numbers[256];
    size_t numbersLength = closingParenthesis - openingParenthesis - 1;
    strncpy(numbers, openingParenthesis + 1, numbersLength);
    numbers[numbersLength] = '\0';
    // Split the numbers into IP address and port components
    char* token = strtok(numbers, ",");
    int ipComponents[4];
    for (int i = 0; i < 4; i++) {
        if (token == NULL) {
            return -1; // Return an error code
        }
        ipComponents[i] = atoi(token);
        token = strtok(NULL, ",");
    }
    int portComponents[2];
    for (int i = 0; i < 2; i++) {
        if (token == NULL) {
            return -1; // Return an error code
        }
        portComponents[i] = atoi(token);
        token = strtok(NULL, ",");
    }
    // Construct the IP address string
    snprintf(ipAddress, INET_ADDRSTRLEN, "%d.%d.%d.%d", ipComponents[0], ipComponents[1], ipComponents[2], ipComponents[3]);
    // Calculate the port number
    *port = (portComponents[0] << 8) + portComponents[1];
    return 0; // Return success code
}

void extractCurrentDirectory(char* response, char* currentDirectory, int size) {
    int i = 0;
    while (response[i] != '\0') {
        currentDirectory[i] = response[i];
        i++;
        if (response[i] == '\0') {
            break;
        } else {
            currentDirectory[i] = '\0';
            i++;
        }
    }
}

bool isPositiveCompletion(const char* response) {
    return strncmp(response, "230", 3) == 0;
}

// Function to handle the ABOR command
int handleABOR(int sockfd) {
    // Send the ABOR command to the server
    if (sendCommand(sockfd, "ABOR\r\n", NULL) == 0) {
        // ABOR command sent successfully
        printf("ABOR command sent successfully.\n");

        // Wait for the response from the server
        char response[256];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the ABOR response
            if (startsWith(response, "226")) {
                printf("ABOR command successful.\n");
                return 0; // Return success code
            } else {
                printf("ABOR command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the ABOR command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the CWD command
int handleCWD(int sockfd, const char* directory) {
    // Send the CWD command to the server
    char command[256];
    snprintf(command, sizeof(command), "CWD %s\r\n", directory);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // CWD command sent successfully
        printf("CWD command sent successfully.\n");

        // Wait for the response from the server
        char response[256];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the CWD response
            if (startsWith(response, "250")) {
                printf("CWD command successful.\n");
                return 0; // Return success code
            } else {
                printf("CWD command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the CWD command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the DELE command
int handleDELE(int sockfd, const char* filename) {
    // Send the DELE command to the server
    char command[256];
    snprintf(command, sizeof(command), "DELE %s\r\n", filename);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // DELE command sent successfully
        printf("DELE command sent successfully.\n");

        // Wait for the response from the server
        char response[256];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the DELE response
            if (startsWith(response, "250")) {
                printf("DELE command successful.\n");
                return 0; // Return success code
            } else {
                printf("DELE command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the DELE command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the HELP command
int handleHELP(int sockfd) {
    // Send the HELP command to the server
    if (sendCommand(sockfd, "HELP\r\n", NULL) == 0) {
        // HELP command sent successfully
        printf("HELP command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the HELP response
            printf("Response from server:\n%s\n", response);
            return 0; // Return success code
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the HELP command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the LIST command
int handleLIST(int sockfd) {
    // Send the LIST command to the server
    if (sendCommand(sockfd, "LIST\r\n", NULL) == 0) {
        // LIST command sent successfully
        printf("LIST command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the LIST response
            printf("Response from server:\n%s\n", response);
            return 0; // Return success code
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the LIST command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the MKD command
int handleMKD(int sockfd, const char* directory) {
    // Prepare the MKD command
    char command[256];
    snprintf(command, sizeof(command), "MKD %s\r\n", directory);
    // Send the MKD command to the server
    if (sendCommand(sockfd, command, NULL) == 0) {
        // MKD command sent successfully
        printf("MKD command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the MKD response
            if (strncmp(response, "257", 3) == 0) {
                printf("Directory '%s' created successfully.\n", directory);
                return 0;  // Return success code
            } else {
                printf("Failed to create directory '%s'.\n", directory);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the MKD command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the NLST command
int handleNLST(int sockfd, const char* directory) {
    // Prepare the NLST command
    char command[256];
    snprintf(command, sizeof(command), "NLST %s\r\n", directory);
    // Send the NLST command to the server
    if (sendCommand(sockfd, command, NULL) == 0) {
        // NLST command sent successfully
        printf("NLST command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Process the NLST response
            if (strncmp(response, "150", 3) == 0) {
                // Server started sending the directory listing
                printf("Directory listing for '%s' started.\n", directory);
                // Receive and print the directory listing
                char listing[1024];
                int bytesRead;
                while ((bytesRead = receiveData(sockfd, listing, sizeof(listing))) > 0) {
                    printf("%.*s", bytesRead, listing);
                }
                // Check the final response code
                if (strncmp(response, "226", 3) == 0) {
                    printf("Directory listing completed.\n");
                    return 0;  // Return success code
                } else {
                    printf("Failed to receive the complete directory listing.\n");
                    return -1; // Return an error code
                }
            } else {
                printf("Failed to start the directory listing.\n");
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the NLST command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the NOOP command
int handleNOOP(int sockfd) {
    // Send the NOOP command to the server
    if (sendCommand(sockfd, "NOOP\r\n", NULL) == 0) {
        // NOOP command sent successfully
        printf("NOOP command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "200", 3) == 0) {
                printf("NOOP command successful.\n");
                return 0;  // Return success code
            } else {
                printf("NOOP command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the NOOP command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the PASS command
int handlePASS(int sockfd, const char* password) {
    // Send the PASS command to the server
    char command[256];
    snprintf(command, sizeof(command), "PASS %s\r\n", password);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // PASS command sent successfully
        printf("PASS command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "230", 3) == 0) {
                printf("Authentication successful.\n");
                return 0;  // Return success code
            } else {
                printf("Authentication failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the PASS command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the PASV command
int handlePASV(int sockfd, char* ipAddress, int* port) {
    // Send the PASV command to the server
    if (sendCommand(sockfd, "PASV\r\n", NULL) == 0) {
        // PASV command sent successfully
        printf("PASV command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "227", 3) == 0) {
                // Parse the response to extract the IP address and port
                if (parsePASVResponse(response, ipAddress, port) == 0) {
                    printf("PASV response parsed successfully. IP: %s, Port: %d\n", ipAddress, *port);
                    return 0;  // Return success code
                } else {
                    printf("Failed to parse PASV response.\n");
                    return -1; // Return an error code
                }
            } else {
                printf("PASV command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the PASV command.\n");
        return -1; // Return an error code
    }
}

// Function to handle the PORT command
int handlePORT(int sockfd, const char* ipAddress, int port) {
    // Format the IP address and port into the PORT command
    char command[256];
    snprintf(command, sizeof(command), "PORT %s,%d,%d\r\n",
             ipAddress, port >> 8, port & 0xFF);

    // Send the PORT command to the server
    if (sendCommand(sockfd, command, NULL) == 0) {
        // PORT command sent successfully
        printf("PORT command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "200", 3) == 0) {
                printf("PORT command successful.\n");
                return 0;  // Return success code
            } else {
                printf("PORT command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the PORT command.\n");
        return -1; // Return an error code
    }
}


// Function to handle the PWD command
int handlePWD(int sockfd) {
    // Send the PWD command to the server
    if (sendCommand(sockfd, "PWD\r\n", NULL) == 0) {
        // PWD command sent successfully
        printf("PWD command sent successfully.\n");
    } else {
        printf("Failed to send the PWD command.\n");
        return -1; // Return an error code
    }
    return 0;
}

// Function to handle the QUIT command
int handleQUIT(int sockfd) {
    // Send the QUIT command to the server
    if (sendCommand(sockfd, "QUIT\r\n", NULL) == 0) {
        // QUIT command sent successfully
        printf("QUIT command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "221", 3) == 0) {
                printf("Server closed the connection. Goodbye!\n");
                return 0;  // Return success code
            } else {
                printf("QUIT command failed. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the QUIT command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the RETR command
int handleRETR(int sockfd, const char* filename) {
    // Send the RETR command to the server
    char command[256];
    snprintf(command, sizeof(command), "RETR %s\r\n", filename);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // RETR command sent successfully
        printf("RETR command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "150", 3) == 0 || strncmp(response, "125", 3) == 0) {
                // File download is allowed, start receiving the file content
                FILE* file = fopen(filename, "wb");
                if (file != NULL) {
                    char buffer[1024];
                    ssize_t bytesRead;

                    while ((bytesRead = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
                        if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
                            printf("Failed to write file data.\n");
                            fclose(file);
                            return -1; // Return an error code
                        }
                    }
                    fclose(file);
                    printf("File '%s' downloaded successfully.\n", filename);
                    return 0; // Return success code
                } else {
                    printf("Failed to open the local file for writing.\n");
                    return -1; // Return an error code
                }
            } else {
                printf("Server rejected the file download. Server response: %s\n", response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the RETR command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the RMD command
int handleRMD(int sockfd, const char* directory) {
    // Send the RMD command to the server
    char command[256];
    snprintf(command, sizeof(command), "RMD %s\r\n", directory);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // RMD command sent successfully
        printf("RMD command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "250", 3) == 0) {
                printf("Directory '%s' successfully removed.\n", directory);
                return 0; // Return success code
            } else {
                printf("Failed to remove directory '%s'. Server response: %s\n", directory, response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the RMD command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the RNFR command
int handleRNFR(int sockfd, const char* oldName) {
    // Send the RNFR command to the server
    char command[256];
    snprintf(command, sizeof(command), "RNFR %s\r\n", oldName);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // RNFR command sent successfully
        printf("RNFR command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "350", 3) == 0) {
                printf("Rename source '%s' accepted.\n", oldName);
                return 0; // Return success code
            } else {
                printf("Failed to rename source '%s'. Server response: %s\n", oldName, response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the RNFR command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the RNTO command
int handleRNTO(int sockfd, const char* newName) {
    // Send the RNTO command to the server
    char command[256];
    snprintf(command, sizeof(command), "RNTO %s\r\n", newName);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // RNTO command sent successfully
        printf("RNTO command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "250", 3) == 0) {
                printf("Rename target '%s' accepted.\n", newName);
                return 0; // Return success code
            } else {
                printf("Failed to rename target '%s'. Server response: %s\n", newName, response);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the RNTO command.\n");
        return -1; // Return an error code
    }
}
// Function to handle the STOR command
int handleSTOR(int sockfd, const char* localFile, const char* remoteFile) {
    // Open the local file for reading
    FILE* file = fopen(localFile, "rb");
    if (!file) {
        printf("Failed to open the local file '%s'.\n", localFile);
        return -1; // Return an error code
    }
    // Send the STOR command to the server
    char command[256];
    snprintf(command, sizeof(command), "STOR %s\r\n", remoteFile);
    if (sendCommand(sockfd, command, NULL) == 0) {
        // STOR command sent successfully
        printf("STOR command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (strncmp(response, "150", 3) == 0) {
                // Server is ready to receive the file
                printf("Server is ready to receive the file.\n");
                // Read and send the contents of the local file to the server
                char buffer[1024];
                size_t bytesRead;
                while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                    if (sendData(sockfd, buffer, bytesRead) == -1) {
                        printf("Failed to send file data to the server.\n");
                        fclose(file);
                        return -1; // Return an error code
                    }
                }
                // Close the local file
                fclose(file);
                // Wait for the response from the server
                if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
                    // Check the final response code
                    if (strncmp(response, "226", 3) == 0) {
                        printf("File '%s' uploaded successfully.\n", remoteFile);
                        return 0; // Return success code
                    } else {
                        printf("Failed to upload file '%s'. Server response: %s\n", remoteFile, response);
                        return -1; // Return an error code
                    }
                } else {
                    printf("Failed to receive response from the server.\n");
                    return -1; // Return an error code
                }
            } else {
                printf("Server is not ready to receive the file. Server response: %s\n", response);
                fclose(file);
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            fclose(file);
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the STOR command.\n");
        fclose(file);
        return -1; // Return an error code
    }
}
// Function to handle the SYST command
int handleSYST(int sockfd) {
    // Send the SYST command to the server
    if (sendCommand(sockfd, "SYST\r\n", NULL) == 0) {
        // SYST command sent successfully
        printf("SYST command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Print the server's system information
            printf("Server system information: %s\n", response);
            return 0; // Return success code
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the SYST command.\n");
        return -1; // Return an error code
    }
}

// Function to handle the USER command
int handleUSER(int sockfd, const char* username) {
    // Create the USER command string
    char command[256];
    snprintf(command, sizeof(command), "USER %s\r\n", username);

    // Send the USER command to the server
    if (sendCommand(sockfd, command, NULL) == 0) {
        // USER command sent successfully
        printf("USER command sent successfully.\n");

        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (isPositiveCompletion(response)) {
                printf("User logged in successfully.\n");
                return 0; // Return success code
            } else {
                printf("Failed to log in with the provided username.\n");
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the USER command.\n");
        return -1; // Return an error code
    }
}

// Function to handle the TYPE command
int handleTYPE(int sockfd, const char* type) {
    // Create the TYPE command string
    char command[256];
    snprintf(command, sizeof(command), "TYPE %s\r\n", type);
    // Send the TYPE command to the server
    if (sendCommand(sockfd, command, NULL) == 0) {
        // TYPE command sent successfully
        printf("TYPE command sent successfully.\n");
        // Wait for the response from the server
        char response[1024];
        if (receiveResponse(sockfd, response, sizeof(response)) == 0) {
            // Check the response code
            if (isPositiveCompletion(response)) {
                printf("Type set successfully.\n");
                return 0; // Return success code
            } else {
                printf("Failed to set the type.\n");
                return -1; // Return an error code
            }
        } else {
            printf("Failed to receive response from the server.\n");
            return -1; // Return an error code
        }
    } else {
        printf("Failed to send the TYPE command.\n");
        return -1; // Return an error code
    }
}

void displayMenu() {
    printf("\n===== Menu =====\n");
    printf("1. Authenticate\n");
    printf("2. Send Command\n");
    printf("3. Receive Response\n");
    printf("4. Upload File\n");
    printf("5. Download File\n");
    printf("6. Create Directory\n");
    printf("7. Pack Files\n");
    printf("8. Unpack Files\n");
    printf("0. Exit\n");
    printf("Enter command number: ");
}

int main() {
    // Establish connection with default server and port
    int sockfd = establishConnection(DEFAULT_SERVER, DEFAULT_PORT);
    if (sockfd == -1) {
        printf("Failed to establish connection. Exiting...\n");
        return 1;
    }

    // Get user credentials for authentication
    char username[256];
    char password[256];
    printf("Enter username: ");
    scanf("%s", username);
    printf("Enter password: ");
    scanf("%s", password);

    // Authenticate with the server
    int authResult = authenticate(sockfd, username, password);
    if (authResult != 0) {
        printf("Authentication failed. Exiting...\n");
        return 1;
    }

    // Variables for user input
    char command[256];
    char string[256];
    double port;
    char ipAddress[256];
    char remoteFile[256];
    char localFile[256];

    // Menu loop
    while (true) {
        displayMenu();

        // Get user input
        scanf("%s", command);

        if (strcmp(command, "1") == 0) {
            // Authenticate
            printf("Enter username: ");
            scanf("%s", username);
            printf("Enter password: ");
            scanf("%s", password);
            authenticate(sockfd, username, password);
        } else if (strcmp(command, "2") == 0) {
            printf("Available commands:\n");
            printf("1. ABOR\n");
            printf("2. CWD\n");
            printf("3. DELE\n");
            printf("4. HELP\n");
            printf("5. LIST\n");
            printf("6. MKD\n");
            printf("7. NLST\n");
            printf("8. NOOP\n");
            printf("9. PASS\n");
            printf("10. PASV\n");
            printf("11. PORT\n");
            printf("12. PWD\n");
            printf("13. QUIT\n");
            printf("14. RETR\n");
            printf("15. RMD\n");
            printf("16. RNFR\n");
            printf("17. RNTO\n");
            printf("18. STOR\n");
            printf("19. SYST\n");
            printf("20. USER\n");
            printf("21. TYPE\n");
            printf("0. Back to main menu\n");

            printf("Enter command number: ");
            int commandNumber;
            scanf("%d", &commandNumber);

            switch (commandNumber) {
                case 1:
                    handleABOR(sockfd);
                    break;
                case 2:
                    printf("Enter directory: ");
                    scanf("%s", string);
                    handleCWD(sockfd, string);
                    break;
                case 3:
                    printf("Enter filename: ");
                    scanf("%s", string);
                    handleDELE(sockfd, string);
                    break;
                case 4:
                    handleHELP(sockfd);
                    break;
                case 5:
                    handleLIST(sockfd);
                    break;
                case 6:
                    printf("Enter directory: ");
                    scanf("%s", string);
                    handleMKD(sockfd, string);
                    break;
                case 7:
                    printf("Enter directory: ");
                    scanf("%s", string);
                    handleNLST(sockfd, string);
                    break;
                case 8:
                    handleNOOP(sockfd);
                    break;
                case 9:
                    printf("Enter password: ");
                    scanf("%s", string);
                    handlePASS(sockfd, string);
                    break;
                case 10:
                    handlePASV(sockfd, ipAddress, &port);
                    break;
                case 11:
                    printf("Enter IP address: ");
                    scanf("%s", ipAddress);
                    printf("Enter port: ");
                    scanf("%d", &port);
                    handlePORT(sockfd, ipAddress, port);
                    break;
                case 12:
                    handlePWD(sockfd);
                    break;
                case 13:
                    handleQUIT(sockfd);
                    // Close the connection
                    closeConnection(sockfd);
                    return 0;
                case 14:
                    printf("Enter filename: ");
                    scanf("%s", string);
                    handleRETR(sockfd, string);
                    break;
                case 15:
                    printf("Enter directory: ");
                    scanf("%s", string);
                    handleRMD(sockfd, string);
                    break;
                case 16:
                    printf("Enter old name: ");
                    scanf("%s", string);
                    handleRNFR(sockfd, string);
                    break;
                case 17:
                    printf("Enter new name: ");
                    scanf("%s", string);
                    handleRNTO(sockfd, string);
                    break;
                case 18:
                    printf("Enter local file: ");
                    scanf("%s", localFile);
                    printf("Enter remote file: ");
                    scanf("%s", remoteFile);
                    handleSTOR(sockfd, localFile, remoteFile);
                    break;
                case 19:
                    handleSYST(sockfd);
                    break;
                case 20:
                    handleUSER(sockfd, remoteFile);
                    break;
                case 21:
                    handleTYPE(sockfd, remoteFile);
                    break;
                case 0:
                    displayMenu();
                default:
                    printf("Invalid command.\n");
                    displayMenu();
            }
        } else if (strcmp(command, "3") == 0) {
            // Receive Response
            receiveResponse(sockfd, NULL, 0);
        } else if (strcmp(command, "4") == 0) {
            // Upload File
            printf("Enter local file path: ");
            scanf("%s", string);
            printf("Enter remote file path: ");
            scanf("%s", command);
            uploadFile(sockfd, string, command);
        } else if (strcmp(command, "5") == 0) {
            // Download File
            printf("Enter remote file path: ");
            scanf("%s", string);
            printf("Enter local file path: ");
            scanf("%s", command);
            downloadFile(sockfd, string, command);
        } else if (strcmp(command, "6") == 0) {
            // Create Directory
            printf("Enter directory name: ");
            scanf("%s", command);
            createDirectory(sockfd, command);
        } else if (strcmp(command, "7") == 0) {
            // Pack Files
            printf("Enter archive file path: ");
            scanf("%s", string);
            printf("Enter source directory: ");
            scanf("%s", command);
            packFiles(string, command);
        } else if (strcmp(command, "8") == 0) {
            // Unpack Files
            printf("Enter archive file path: ");
            scanf("%s", string);
            printf("Enter destination directory: ");
            scanf("%s", command);
            unpackFiles(string, command);
        } else if (strcmp(command, "0") == 0) {
            // Exit the program
            break;
        } else {
            printf("Invalid command. Please try again.\n");
        }
    }

    // Close the connection
    closeConnection(sockfd);

    return 0;
}

