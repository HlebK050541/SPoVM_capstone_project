//
// Created by HlebK on 3/28/2023.
//

#include "FileTransfer.h"

class FileTransfer {
private:
    std::string hostname;
    int port;
    std::string username;
    std::string password;
    std::string remoteDir;
    std::string localDir;

public:
    FileTransfer(std::string h, int p, std::string u, std::string pw, std::string rDir, std::string lDir);

    bool connect();
    bool disconnect();
    bool upload(std::string filename);
    bool download(std::string filename);
};