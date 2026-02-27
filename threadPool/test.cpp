
#include<iostream>
#include<unistd.h>
#include"threadPool.h"

void print(int x){
    std::cout << "num : " << x << "\t" << __func__ << "\t" << __LINE__ << "\t"<< std::this_thread::get_id() << std::endl;
}


int main(){

    threadPool* p = new threadPool(8);

    for(int i = 0;i < 100;i++){
        p->add_task(std::bind(&print,i));
    }
    sleep(1);
    return 0;
}