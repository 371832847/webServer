#include"httpResponse.h"

using namespace std;

const unordered_map<string,string> httpResponse::SUFFIX_TYPE = {
    {".html",   "text/html"},
    {".xml",    "text/xml"},
    {".xhtml",  "application/xhtml+xml"},
    {".txt",    "text/plain"},
    {".rtf",    "application/rtf"},
    {".pdf",    "application/pdf"},
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    {".mp4",    "video/mp4"},
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};


const unordered_map<int, string> httpResponse::CODE_STATUS = {
    { 200, "OK" },              //200 OK
    {206, "Partial Content"},   //206 部分资源
    { 400, "Bad Request" },     //400 请求错误
    { 403, "Forbidden" },       //403 禁止请求
    { 404, "Not Found" },       //404 资源未找到

};

const unordered_map<int, string> httpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

httpResponse::httpResponse(){
    code = -1;
    path = srcDir = "";
    isKeepAlive = false;
    mmfile = nullptr;
    mmfileStat = {0};
}

httpResponse::~httpResponse(){
    unmap_file();
    base = 0;
    offset = 0;
}

void httpResponse::init(const string& srcDir_,string& path_,bool isKeepAlive_,int code_){
    assert(srcDir_ != "");
    if(mmfile){
        if(base == 0 && offset == 0)
            unmap_file();
    }
    code = code_;
    isKeepAlive = isKeepAlive_;
    path = path_;
    offset = 0;
    base = 0;
    srcDir = srcDir_;
    mmfile = nullptr;
    range.clear();
    mmfileStat = {0};
}

void httpResponse::make_response(Buffer& buff){

    if(stat((srcDir + path).data(),&mmfileStat) < 0 || S_ISDIR(mmfileStat.st_mode)){
        string info;
        info.append( (srcDir + path) + "is not found!");
        LOG_WARN(info.c_str());
        code = 404;
    }else if(!(mmfileStat.st_mode & S_IROTH)){
        code = 403;
    }else if(code == -1){
        code = 200;
    }

    _error_html();
    _add_state_line(buff);
    _add_header(buff);
    _add_content(buff);
}

char* httpResponse::file(){
    return mmfile;
}

size_t httpResponse::file_len() const{
    return mmfileStat.st_size;
}

void httpResponse::_error_html(){
    if(CODE_PATH.count(code)){
        path = CODE_PATH.find(code)->second;
        stat((srcDir+path).data(),&mmfileStat);
    }
}

void httpResponse::_add_state_line(Buffer& buff){
    string status;
    if(CODE_STATUS.count(code) == 0){
        code = 400;
    }
    status = CODE_STATUS.find(code)->second;
    buff.append("HTTP/1.1 " + to_string(code) + " " + status + "\r\n");
}

void httpResponse::_add_header(Buffer& buff){
    buff.append("Connection: ");
    if(isKeepAlive){
        buff.append("keep-alive\r\n");
        buff.append("keep-alive: max=6, timeout=120\r\n");
    }else{
        buff.append("close\r\n");
    }

    buff.append("Content-type: " + _get_file_type() + "\r\n");
}

void httpResponse::_add_content(Buffer& buff){
    int srcfd = open((srcDir + path).data(),O_RDONLY);
    if(srcfd < 0){
        error_content(buff,"File Not Found!");
        LOG_WARN("FILE Not Found!");
        return;
    }
    int* mmRet = (int*)mmap(0,mmfileStat.st_size,PROT_READ,MAP_PRIVATE,srcfd,0);
    if(mmRet == MAP_FAILED){
        close(srcfd);
        error_content(buff,"File Not Found!");
        LOG_ERROR("mmap failed!");
        return;
    }
    mmfile = (char*)mmRet;
    close(srcfd);
    if(code == 206){
        buff.append("Accept-Ranges: bytes\r\n");
        if(range.find('=',0) != string::npos){
            string sub = range.substr(range.find('=',0) + 1);
            string front,back;

            front = sub.substr(0,sub.find('-'));
            back = to_string(min(stol(front) + CHUNK_SIZE,static_cast<long>(mmfileStat.st_size) - 1));
            offset = stol(back) + 1;
            base = stol(front);
            buff.append("Content-Range: bytes " + front + '-' + back + '/' + to_string(mmfileStat.st_size) + "\r\n");
            buff.append("Content-length: " + to_string( offset - base) + "\r\n\r\n");
        }
    }else{
        buff.append("Content-length: " + to_string(mmfileStat.st_size)+ "\r\n\r\n");
    }
}

void httpResponse::unmap_file(){
    if(mmfile){
        munmap(mmfile,mmfileStat.st_size);
        mmfile = nullptr;
    }
}

string httpResponse::_get_file_type(){
    string::size_type idx = path.find_last_of('.');
    if(idx == string::npos){
        return "text/plain";
    }
    string suffix = path.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1){
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void httpResponse::error_content(Buffer& buff,string message){
    string body;
    string status;
    body += "<html><title>Error</title></html>";
    body += "<style>body{bgcolor=\"ffffff\"}</style>";
    if(CODE_STATUS.count(code) == 1){
        status = CODE_STATUS.find(code)->second;
    }else{
        status = "Bad Request";
    }
    
    body += to_string(code) + ": " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></hr>";
    
    buff.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}