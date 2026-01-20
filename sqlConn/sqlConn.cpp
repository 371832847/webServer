#include"sqlConn.h"


sqlConn::sqlConn(unsigned int size) {
	this->connSize = size;
	avaliable = size;
	for (int i = 0; i < size; i++) {
		MYSQL* conn;
		conn = mysql_init(NULL);
		mysql_real_connect(conn, "14.103.168.41", "root", "371832", "webchat", 3306, 0, NULL);
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

MYSQL* sqlConn::getConn() {
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

}

bool sqlConn::returnConn(MYSQL *conn) {
	std::lock_guard<std::mutex> m(mtx);
	for (int i = 0; i < connSize; i++) {
		if (conn == pool[i]) {
			flag[i] = true;
			avaliable++;
			return true;
		}
	}
}

unsigned int sqlConn::getAvaliable() { return avaliable; }