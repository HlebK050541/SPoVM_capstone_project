//
// Created by HlebK on 3/28/2023.
//

#ifndef PROJECT_FTPCLIENT_H
#define PROJECT_FTPCLIENT_H

#include <string>

class FTPClient {
public:
    FTPClient();
    ~FTPClient();
    bool connect(const std::string& server, const std::string& username, const std::string& password);
    bool uploadFile(const std::string& localFilePath, const std::string& remoteDirectoryPath);
    bool downloadFile(const std::string& remoteFilePath, const std::string& localDirectoryPath);
    bool createDirectory(const std::string& remoteDirectoryPath);
    bool deleteFile(const std::string& remoteFilePath);
    bool deleteDirectory(const std::string& remoteDirectoryPath);
    bool renameFile(const std::string& remoteFilePath, const std::string& newFilename);
    bool renameDirectory(const std::string& remoteDirectoryPath, const std::string& newDirectoryname);
    bool listFiles(const std::string& remoteDirectoryPath);
private:
    // Private member variables and helper functions
};

#endif //PROJECT_FTPCLIENT_H
