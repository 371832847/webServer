#include"sqlConn.h"




sqlConn* sqlConn::instance(){
	static sqlConn pool;
	return &pool;
}



sqlConn::sqlConn(unsigned int size) {
	this->connSize = size;
	avaliable = size;
	for (int i = 0; i < size; i++) {
		MYSQL* conn;
		conn = mysql_init(NULL);
		assert(mysql_real_connect(conn, IP_ADDR, NAME, PWD, DATABASE, 3306, 0, NULL));
		 
		pool.emplace_back(conn);
		flag.push_back(true);
	}
}


sqlConn::~sqlConn() {
	while(!pool.empty()){
		mysql_close(pool.back());
		pool.pop_back();
	}
}

MYSQL* sqlConn::get_conn() {
	std::lock_guard<std::mutex> m(mtx);
	if (avaliable == 0) {
		return NULL;
	}
	for (int i = 0; i < connSize; i++) {
		if (flag[i]) {
			flag[i] = 0;
			avaliable--;
			return pool[i];
		}
	}
	return NULL;
}

bool sqlConn::return_conn(MYSQL *conn) {
	std::lock_guard<std::mutex> m(mtx);
	for (int i = 0; i < connSize; i++) {
		if (conn == pool[i]) {
			flag[i] = true;
			avaliable++;
			return true;
		}
	}
	return false;
}

unsigned int sqlConn::get_avaliable() { return avaliable; }