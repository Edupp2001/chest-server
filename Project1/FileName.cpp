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
	string name = "";
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
	if (SOCKET_ERROR == ::bind(listener, (struct sockaddr*)&players[ip].addr, sizeof(players[ip].addr))) //Связываем сокет с адресом
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
		//в случае потери соединения мы готовы ждать игрока 15 секунд, иначе он мудак
		if (n == -1) {
			auto now = chrono::system_clock::now();
			time_t timeStart = chrono::system_clock::to_time_t(now);
			time_t timeNow = timeStart;
			const int deltatime = 15;
			while (timeStart + deltatime >= timeNow) {
				timeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
			}
			user = false;
		}
		data = "";
		//очищаем буфер
		memset(buf, 0, 2000);
		//получаем сообщение
		n = recv(curr_sock, buf, 2000, 0);//пустое сообщение возвращает код -1
		//проверка что игрок здесь
		if (n != -1)
			user = true;
		data = buf;

		string msg = "";
		if (!gamestatus) {
			if (players[ip].name == "") 
				cout << data << " is here!" << endl;
			else 
				cout << players[ip].name << " changed his name to :" << data << endl;
			players[ip].name = data;
			msg = "the game is not started yet";
		}
		else {
			//0 получить колоду
			if (data == "0")
				for (int i = 0; i < players[ip].hand.size(); ++i)
					msg += ITS(players[ip].hand[i].id) + " ";//отправляем сообщение в виде id карт
			
		}
		send(curr_sock, msg.c_str(), msg.size(), 0);
	}
	closesocket(listener);
	closesocket(curr_sock);
	cout << players[ip].name << " left us :C" << endl;
}
void start_game_chest(int size) {
	const int cards_at_start = 4;
	const int each_suit = 15;//2-A is 13; 0,1,2 are reserved
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
				cout << "oops! chest from start, you are too lucky playerid:" << (*it).first << "!" << "I'm restarting the game!";//вероятно это лишнее, колода хорошо замешана
				gamestatus = false;
				start_game_chest(deck.size());//игроки глобальные, откажусь от этой идеи
			}
		}
	}
	*/
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
		SOCKET listener = socket(AF_INET, SOCK_STREAM, 0); //Создаем слушающий сокет
		if (listener == INVALID_SOCKET)
			cout << "Error with creating socket" << endl; //Ошибка создания сокета
		fd_set list;
		list.fd_array[50] = listener;
		sockaddr_in addr; //Создаем и заполняем переменную для хранения адреса
		addr.sin_family = AF_INET; //Семейство адресов, которые будет обрабатывать наш сервер, у нас это TCP/IP адреса
		addr.sin_port = htons(3128); //Используем функцию htons для перевода номера порта в TCP/IP представление
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
	cout << endl;
	if (players.size() == 1)
		cout << "look's like (s)he has no friends!" << endl;
	start_game_chest(num_cards);
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

