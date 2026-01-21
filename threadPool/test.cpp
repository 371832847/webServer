
#include<iostream>
#include<unistd.h>
#include"threadPool.h"

void function(void*){
}


int main(){

    func f = &function;
    class threadPool p;
    for(int i = 0;i < 100;i++){
        p.addTask(f,NULL);
    }
    sleep(1);
    return 0;
}