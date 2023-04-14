//
// Created by HlebK on 4/14/2023.
//

#ifndef ARCHIVEHANDLER_H
#define ARCHIVEHANDLER_H

#include <string>
#include <vector>

class ArchiveHandler {
public:
    ArchiveHandler();
    bool createArchive(std::string archiveName, std::vector<std::string> files);
    bool extractArchive(std::string archiveName, std::string destinationDir);
    bool isArchive(std::string fileName);
    std::vector<std::string> getFilesInArchive(std::string archiveName);
};

#endif // ARCHIVEHANDLER_H
