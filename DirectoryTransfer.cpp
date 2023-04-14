//
// Created by HlebK on 3/28/2023.
//

#include <string>
#include "FTPClient.h"
#include "ArchiveHandler.h"

class DirectoryTransfer {
private:
    FTPClient ftpClient;
    ArchiveHandler archiveHandler;
    std::string remoteDir;
    std::string localDir;

public:
    DirectoryTransfer(std::string hostname, int port, std::string username, std::string password, std::string remoteDir, std::string localDir);
    bool syncRemoteToLocal();
    bool syncLocalToRemote();
};
