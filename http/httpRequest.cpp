#include"httpRequest.h"

using namespace std;

const unordered_set<string> httpRequest::DEFAULT_HTML{
    "/index","/register","/login",
    "/welcome","/video","/picture"
};

const unordered_map<string,int> httpRequest::DEFAULT_HTML_TAG{
    {"/register.html",0},{"/login.html",1}
};

void httpRequest::init(){
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool httpRequest::is_keep_alive() const{
    if(header_.count("Connection") == 1){
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool httpRequest::parse(Buffer& buff){
    const char CRLF[] = "\r\n";
    if(buff.readable_bytes() <= 0){
        return false;
    }

    while(buff.readable_bytes() && state_ != FINISH){
        const char* lineEnd = search(buff.peek(),buff.begin_write_const(),CRLF,CRLF+2);
        string line(buff.peek(),lineEnd);
        // LOG_INFO(line.c_str());
        switch (state_){
        case REQUEST_LINE:{
            if(!_parse_request_line(line)){
                return false;
            }
            _parse_path();
            break;
        }
        case HEADERS:{
            _parse_header(line);
            if(buff.readable_bytes() <= 2){
                state_ = FINISH;
            }
            break;
        }   
        case BODY:{
            _parse_body(line);
            break;
        }   
        default:
            break;
        }
        if(lineEnd == buff.begin_write()){break;}
        buff.retrieve_until(lineEnd + 2);
    }
    return true;
}

void httpRequest::_parse_path(){
    if(path_ == "/"){
        path_ = "/index.html";
    }else{
        for(auto &item : DEFAULT_HTML){
            if(item == path_){
                path_ += ".html";
                break;
            }
        }
    }
}

bool httpRequest::_parse_request_line(const string& line){
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line,subMatch,patten)){
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS;
        return true;
    }
    return false;
}

void httpRequest::_parse_header(const string& line){
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line,subMatch,patten)){
        header_[subMatch[1]] = subMatch[2];
    }else{
        state_ = BODY;
    }
}

void httpRequest::_parse_body(const string& line){
    body_ = line;
    _parse_post();
    state_ = FINISH;
}

int httpRequest::conver_hex(char ch){
    if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return ch;
}

void httpRequest::_parse_post(){
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded"){
        _parse_from_url_encoded();
        if(DEFAULT_HTML_TAG.count(path_)){
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            if(tag == 0 || tag == 1){
                bool isLogin = (tag == 1);
                if(user_verify(post_["username"],post_["password"],isLogin)){
                    path_ = "/welcome.html";
                }else{
                    path_ = "error.html";
                }
            }
        }
    }
}

void httpRequest::_parse_from_url_encoded(){
    if(body_.size() == 0){return;}
    string key,value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;
    for(;i < n;i++){
        char ch = body_[i];
        switch(ch){
            case '=':
                key = body_.substr(j,i-j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = conver_hex(body_[i + 1] * 16 + conver_hex(body_[i + 2]));
                body_[i + 2 ] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                break;
            default:
                break;
        }
    }

    assert(j <= i);
    if(post_.count(key) == 0 && j < i){
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool httpRequest::user_verify(const string& name,const string& pwd,bool isLogin){
    if(name == "" || pwd == ""){return false;}
    MYSQL* sql = sqlConn::instance()->get_conn();
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;

    if(!isLogin){ flag = true;}
    snprintf(order,256,"SELECT id,pwd FROM account WHERE id='%s' LIMIT 1",name.c_str());

    if(mysql_query(sql,order) != 0){
        mysql_free_result(res);
        sqlConn::instance()->return_conn(sql);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);          //return num of fields
    fields = mysql_fetch_field(res);    //
    
    while(MYSQL_ROW row = mysql_fetch_row(res)){
        string password(row[1]);
        if(isLogin){
            if(pwd == password){flag = true;}
            else{
                flag = false;
            }
        }else{
            flag = false;
        }
    }

    mysql_free_result(res);
    if(!isLogin && flag == true){
        bzero(order,256);
        snprintf(order,256,"INSERT INTO account(id,pwd,resetKey) values('%s','%s','not')");
        if(mysql_query(sql,order) != 0){
            flag = false;
        }
        flag = true;
    }

    sqlConn::instance()->return_conn(sql);
    return flag;
}

std::string httpRequest::path() const{
    return path_;
}

std::string& httpRequest::path(){
    return path_;
}

std::string httpRequest::method() const{
    return method_;
}

std::string httpRequest::version() const{
    return version_;
}

std::string httpRequest::get_post(const std::string& key) const{
    assert(key != "");
    if(post_.count(key) == 1){
        return post_.find(key)->second;
    }
    return "";
}

std::string httpRequest::get_post(const char* key) const{
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

bool httpRequest::is_partial() const{
    return header_.count("Range");
}

string httpRequest::ranges(){
    if(header_.count("Range")){
        return header_.find("Range")->second;
    }
    return "";
}