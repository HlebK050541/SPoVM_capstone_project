//
// Created by HlebK on 3/28/2023.
//

#ifndef PROJECT_FILETRANSFER_H
#define PROJECT_FILETRANSFER_H

#include <string>

class FileTransfer {
public:
    FileTransfer();
    FileTransfer(const std::string& serverAddress, int portNumber, const std::string& username, const std::string& password);
    ~FileTransfer();

    bool connect();
    bool disconnect();
    bool uploadFile(const std::string& localFilePath, const std::string& remoteFilePath);
    bool downloadFile(const std::string& remoteFilePath, const std::string& localFilePath);

private:
    std::string m_serverAddress;
    int m_portNumber;
    std::string m_username;
    std::string m_password;

    // Additional private members for the implementation
};

#endif //PROJECT_FILETRANSFER_H
