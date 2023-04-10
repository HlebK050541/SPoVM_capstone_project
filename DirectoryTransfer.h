//
// Created by HlebK on 3/28/2023.
//

#ifndef PROJECT_DIRECTORYTRANSFER_H
#define PROJECT_DIRECTORYTRANSFER_H

class DirectoryTransfer {
public:
    DirectoryTransfer();
    ~DirectoryTransfer();
    bool sendDirectory(const std::string& localDirectory, const std::string& remoteDirectory);
    bool receiveDirectory(const std::string& remoteDirectory, const std::string& localDirectory);

private:
    // Any private variables or helper functions can be defined here
};

#endif //PROJECT_DIRECTORYTRANSFER_H
