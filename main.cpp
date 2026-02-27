#include"server/server.h"

int main(){

    sv::server s(8900,3,120,true,3306,"root","371832","account");
    s.start();

    return 0;
}
	
