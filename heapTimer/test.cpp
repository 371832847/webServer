#include"heaptimer.h"
#include<iostream>
#include<unistd.h>

void func_(){
    std::cout << __func__ << "for 1" << std::endl;  
}

void task(){
    std::cout << __func__ << "for 2" << std::endl;
}

int main(){

    HeapTimer timer;

    timer.add(1,200,func_);
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    timer.add(2,3000,task);
    usleep(100000);
    std::cout << "NextTick time:" << timer.GetNextTick() << std::endl;
   while(1){
        if(timer.empty()){
            break;
        }
        timer.tick();
    }

    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if(duration.count()){
            std::cout << "execution time:"<< duration.count() << std::endl;
        }

    return 0;
}