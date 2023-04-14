//
// Created by HlebK on 3/28/2023.
//
#include <string>

class FileTransfer {
private:
    std::string fileName;
    int fileSize;
    std::string fileType;

public:
    FileTransfer(std::string fileName, int fileSize, std::string fileType);
    std::string getFileName();
    int getFileSize();
    std::string getFileType();
};