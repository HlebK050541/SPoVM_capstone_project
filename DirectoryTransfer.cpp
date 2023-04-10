//
// Created by HlebK on 3/28/2023.
//

#include "DirectoryTransfer.h"

class DirectoryTransfer {
private:
    std::string localDirectory;
    std::string remoteDirectory;
public:
    DirectoryTransfer(const std::string& local, const std::string& remote) : localDirectory(local), remoteDirectory(remote) {}

    bool sendDirectory(FTPClient& client);
    bool receiveDirectory(FTPClient& client);
};