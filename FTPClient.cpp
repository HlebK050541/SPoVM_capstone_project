//
// Created by HlebK on 3/28/2023.
//

#include "FTPClient.h"

class FTPClient {
private:
    std::string host;
    std::string username;
    std::string password;
    std::string current_directory;

public:
    FTPClient(std::string host, std::string username, std::string password);
    ~FTPClient();

    void connect();
    void disconnect();
    bool is_connected();

    std::vector<std::string> list_directory();
    void change_directory(std::string path);
    std::string get_current_directory();

    bool upload_file(std::string local_path, std::string remote_path);
    bool download_file(std::string remote_path, std::string local_path);

    bool archive_directory(std::string directory_path, std::string archive_path);
    bool compress_file(std::string file_path, std::string compressed_file_path);

    bool authenticate();
    bool is_authenticated();

    std::string get_error_message();

    void log(std::string message);
};