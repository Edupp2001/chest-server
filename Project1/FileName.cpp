#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <vector>
#include <time.h>
#include <set>
#include <map>
#include <chrono>
#include <algorithm>
#include <WinSock2.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#include "CardClass.h"
using namespace std;

class Player {
public:
	string name;
	bool nothread = true;
	sockaddr_in addr;
	vector <Card> hand;//map?//should be private
	vector <string> chest;//
	vector <int> hand_counter;//should be private
	//take card from deck
	//take card from oppenent
	//if 4 -> shest++
};

map<string, Player> players;
vector <Card> deck;
bool gamestatus = false;

void TalkToClient(string ip, SOCKET listener) {
	if (SOCKET_ERROR == ::bind(listener, (struct sockaddr*)&players[ip].addr, sizeof(players[ip].addr))) //—в€зываем сокет с адресом
		cout << "Error with binding socket";
	char buf[2000];
	listen(listener, 1);//очередь соединений 1
	int len = sizeof(players[ip].addr);
	SOCKET curr_sock;
	if (FAILED(curr_sock = accept(listener, (struct sockaddr*)&players[ip].addr, &len)))
		cout << "fail" << endl;
	int n = 1;
	string data = "";
	bool user = true;
	while (data != "quit" && user) {
		if (n == -1) {
			auto now = chrono::system_clock::now();
			time_t timeStart = chrono::system_clock::to_time_t(now);
			time_t timeNow = timeStart;
			const int deltatime = 10;
			while (timeStart + deltatime >= timeNow) {
				timeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
			}
			user = false;
		}
		data = "";
		memset(buf, 0, 2000);
		n = recv(curr_sock, buf, 2000, 0);
		if (n) user = true;
		data = buf;
		players[ip].name = data;
		//cout << data << endl;
		string hand = "your msg is: " + data;
		cout << inet_ntoa(players[ip].addr.sin_addr) << endl;
		send(curr_sock, hand.c_str(), hand.size(), 0);
	}
	closesocket(listener);
	closesocket(curr_sock);
}
void ClientFirstConnects(int num_cards) {
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
	//players["0.0.0.0"] = Player();
	auto now = chrono::system_clock::now();
	time_t timeStart = chrono::system_clock::to_time_t(now);
	time_t timeNow = timeStart;
	int deltatime = 30;
	while (timeStart + deltatime > timeNow) {
		SOCKET listener = socket(AF_INET, SOCK_STREAM, 0); //—оздаем слушающий сокет
		if (listener == INVALID_SOCKET)
			cout << "Error with creating socket" << endl; //ќшибка создани€ сокета
		fd_set list;
		list.fd_array[50] = listener;
		sockaddr_in addr; //—оздаем и заполн€ем переменную дл€ хранени€ адреса
		addr.sin_family = AF_INET; //—емейство адресов, которые будет обрабатывать наш сервер, у нас это TCP/IP адреса
		addr.sin_port = htons(3128); //»спользуем функцию htons дл€ перевода номера порта в TCP/IP представление
		addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY означает, что сервер будет принимать запросы с любых IP
		string ip = inet_ntoa(addr.sin_addr);
		if (players[ip].nothread) {
			players[ip].addr = addr;
			players[ip].nothread = false;
			thread thr(TalkToClient, inet_ntoa(addr.sin_addr), listener);
			thr.detach();
		}
		timeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
	}
	if (players.size()) {
		cout << "starting with " << players.size() << " players" << endl;
		for (auto it = players.begin(); it != players.end(); ++it) {
			cout << it->second.name << " ";
		}
	}
	if (players.size() == 1)
		cout << "look's like (s)he has no friends!" << endl;
	//start_game_chest(players, num_cards);
}
void start_game_chest(int size) {
	const int cards_at_start = 4;
	const int each_suit = 15;//2-A is 13; 0,1,2 are reserved
	bool gamestatus = true;
	vector <Card> deck = create_deck(size);
	for (auto it = players.begin(); it != players.end(); ++it) {
		(*it).second.hand_counter.resize(each_suit);
		for (int k = 0; k < each_suit; ++k) {
			(*it).second.hand_counter[k] = 0;
		}
	}
	//give 4 cards to players
	for (int i = 0; i < cards_at_start; ++i) {
		for (auto it = players.begin(); it != players.end(); ++it) {
			(*it).second.hand.push_back(deck[size - 1]);
			(*it).second.hand_counter[deck[size - 1].code]++;
			size--;
		}
	}
	gamestatus = true;
	/*
	//check for chests from start, restart if found
	for (auto it = players.begin(); it != players.end(); ++it) {
		for (int j = 0; j < 15; ++j) {
			if ((*it).second.hand_counter[j] == 4) {
				cout << "oops! chest from start, you are too lucky playerid:" << (*it).first << "!" << "I'm restarting the game!";//веро€тно это лишнее, колода хорошо замешана
				gamestatus = false;
				start_game_chest(deck.size());//игроки глобальные, откажусь от этой идеи
			}
		}
	}
	*/
}

int main() {
	srand(time(NULL));
	string input = "";
	while (input != "quit") {
		cin >> input;
		if (input == "start_chest36") {
			ClientFirstConnects(36);
		}
		else if (input == "start_chest52" || input == "1") {
			ClientFirstConnects(52);
		}
		else {
			cout << "no" << endl;
		}
	}
	
 }

