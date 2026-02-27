#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H


#include<unordered_map>
#include<unordered_set>
#include<string>
#include<algorithm>
#include<regex>
#include"../buffer/Buffer.h"
#include"../sqlConn/sqlConn.h"
#include"../log/log.h"

//http response
class httpRequest{      
public:
    enum PARSE_STATE{
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH
    };

    enum HTTP_CODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NORESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    httpRequest(){init();};
    ~httpRequest() = default;

    void init();
    bool parse(Buffer& buff);

    std::string path() const;
    std::string& path();
    std::string method() const;
    std::string version() const;
    std::string get_post(const std::string& key) const;
    std::string get_post(const char* key) const;
    bool is_partial() const;
    bool is_keep_alive() const;
    std::string ranges(); 
private:
    
    bool _parse_request_line(const std::string& line);
    void _parse_header(const std::string& line);
    void _parse_body(const std::string& line);

    void _parse_path();
    void _parse_post();
    void _parse_from_url_encoded();

    static bool user_verify(const std::string& name,const std::string& pwd,bool isLogin);


    PARSE_STATE state_;
    std::string method_,path_,version_,body_;
    std::unordered_map<std::string,std::string> header_;
    std::unordered_map<std::string,std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string,int> DEFAULT_HTML_TAG;
    static int conver_hex(char ch);

};

#endif //HTTPRESQUEST_H