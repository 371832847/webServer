#include<iostream>
#include<assert.h>
#include"sqlConn.h"



int main(){

    sqlConn* q = sqlConn::instance();
    for(int i = 0;i < 100;i++){
         MYSQL* conn = q->get_conn();
        MYSQL_RES* res;
        MYSQL_ROW row;

        mysql_query(conn,"SELECT * FROM account"); 

        res = mysql_store_result(conn);
        assert(res);

        while((row = mysql_fetch_row(res)) != NULL){
            std::cout << row[0] << " " << row[1] << " " << row[2] << " " << row[3] << std::endl;
        }
        q->return_conn(conn);
        mysql_free_result(res);
        std::cout << std::endl;
    }
   

    return 0;
}