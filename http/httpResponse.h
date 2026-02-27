#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include<unistd.h>
#include<string>
#include<sys/stat.h>
#include<unordered_map>
#include<fcntl.h>
#include<sys/mman.h>
#include"../buffer/Buffer.h"
#include"../log/log.h"

#define CHUNK_SIZE 1024*1024
//#define CHUNK_SIZE 100*1024

//mnt/c/Users/37183/
class httpResponse{
public:
    httpResponse();
    ~httpResponse();

    void init(const std::string& srcDir,std::string& path,bool isKeepAlive = false,int code = -1);
    void make_response(Buffer& buff);
    void unmap_file();
    char* file();
    size_t file_len() const;
    void error_content(Buffer& buff,std::string message);
    int _code() const;

    size_t base;
    size_t offset;
    std::string range;
private:
    void _add_state_line(Buffer& buff);
    void _add_header(Buffer& buff);
    void _add_content(Buffer& buff);
    void _error_html();
    std::string _get_file_type();

    int code;
    bool isKeepAlive;
    char* mmfile;
    struct stat mmfileStat;
    std::string path;
    std::string srcDir;

    static const std::unordered_map<std::string,std::string> SUFFIX_TYPE;
    static const std::unordered_map<int,std::string> CODE_STATUS;
    static const std::unordered_map<int,std::string> CODE_PATH;

};

#endif //HTTPRESPONSE_H