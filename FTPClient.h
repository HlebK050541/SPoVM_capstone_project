//
// Created by HlebK on 3/28/2023.
//

#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#include <string>

class FTPClient {
private:
    std::string hostname;
    int port;
    std::string username;
    std::string password;

public:
    FTPClient(std::string h, int p, std::string u, std::string pw);

    bool connect();
    bool disconnect();
    bool login();
    bool logout();
    bool changeWorkingDirectory(std::string dir);
    bool uploadFile(std::string localPath, std::string remotePath);
    bool downloadFile(std::string remotePath, std::string localPath);
    bool deleteFile(std::string remotePath);
};

#endif // FTPCLIENT_H
