//
// Created by HlebK on 4/14/2023.
//

#include "FTPClient.h"
#include <iostream>

int main() {
    std::string hostname = "ftp.example.com";
    int port = 21;
    std::string username = "user";
    std::string password = "password";

    FTPClient ftpClient(hostname, port, username, password);

    if (ftpClient.connect()) {
        std::cout << "Connected to FTP server successfully" << std::endl;

        // Perform FTP operations here

        ftpClient.disconnect();
    } else {
        std::cerr << "Failed to connect to FTP server" << std::endl;
    }

    return 0;
}


