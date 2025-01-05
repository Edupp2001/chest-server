#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "CardClass.h"
#include <time.h>
#include <set>
#include <map>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <WinSock2.h>
#include <thread>
using namespace std;

class Player {
public:
	string name = "";
	bool nothread = true;
	vector <Card> hand;//map?//should be private
	vector <int> chest;//
	vector <int> hand_counter;//should be private
	string nextip;
	vector <string> StolenCardsHistory;//SCH
	string disconnectedguy = "";
	bool myturn = false;
	//take card from deck
	//take card from oppenent
	//if 4 -> shest++
};

map <string, Player> players;
vector <Card> deck;
bool gamestatus = false;
bool waitingforcon = false;
int handsize(string ip) {
	return players[ip].hand.size();
}
void checkforchest(string ip, int code) {
	if (players[ip].hand_counter[code] == 4) {//��������� �������� �� ����
		players[ip].chest.push_back(code);//��������� ��� � "������ ������"
		players[ip].hand_counter[code] = 0;//�������� � ��������, �� ����� ����� �� ����
		for (int i = 0; i < players[ip].hand.size(); ++i) {//���������� �����
			if (players[ip].hand[i].code == code) {//������� ���������� �� ����
				swap(players[ip].hand[i], players[ip].hand[players[ip].hand.size() - 1]);//������ ����� � �����
				players[ip].hand.pop_back();//������� ��
			}
		}
	}
	if (handsize(ip) == 0) {
		Card topdeck = deck[deck.size() - 1];//����� ������
		int topdeckcode = topdeck.code;//����� ��� �������
		deck.pop_back();//������ ����� � �������
		players[ip].hand.push_back(topdeck);//������ � ����
		players[ip].hand_counter[topdeckcode]++;//����������� �������
	}
}
void drawcard(string ip) {
	Card topdeck = deck[deck.size() - 1];//����� ������
	int topdeckcode = topdeck.code;//����� ��� �������
	deck.pop_back();//������ ����� � �������
	players[ip].hand.push_back(topdeck);//������ � ����
	players[ip].hand_counter[topdeckcode]++;//����������� �������
	checkforchest(ip, topdeckcode);//��������� ��������
}
void TalkToClient(string ip, SOCKET curr_sock) {
	char buf[2000];
	int n = 1;
	string data = "";
	bool user = true;
	while (data != "quit" && user) {
		//� ������ ������ ���������� �� ������ ����� ������ 15 ������, ����� �� �����
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
		//������� �����
		memset(buf, 0, 2000);
		//�������� ���������
		n = recv(curr_sock, buf, 2000, 0);//������ ��������� ���������� ��� -1
		//�������� ��� ����� �����
		if (n != -1)
			user = true;
		data = buf;

		string msg = "";
		if (!gamestatus) {
			if (players[ip].name == "") {
				cout << data << " is here!" << endl;
				players[ip].name = data;
			}
			else if (data != "0") {
				cout << players[ip].name << " changed his name to: " << data << endl;
				players[ip].name = data;
			}
			
			msg = "the game is not started yet";
		}
		else {
			if (data == "what") {
				for (auto it = players.begin(); it != players.end(); ++it) {
					cout << (*it).first << endl;
				}
			}
			//0 �������� ������
			//info players turn order, num of cards in hand, chests
			else if (data == "0")
				for (int i = 2; i < players[ip].hand_counter.size(); ++i)
					msg += ITS(players[ip].hand_counter[i]) + " ";//���������� ��������� � ���� id ����
			else if (data == "1")
				for (int i = 0; i < players[ip].hand.size(); ++i)
					msg += ITS(players[ip].hand[i].id) + " ";
			else if ((data >= "2" && data <= "9") || data == "10" || data == "j" || data == "q" || data == "k" || data == "a") {
				
				int code;//��� ����� �������
				if (data == "j")
					code = 11;
				else if (data == "q")
					code = 12;
				else if (data == "k")
					code = 13;
				else if (data == "a")
					code = 14;
				else
					code = STI(data);
				if (players[ip].myturn) {//���� ��� ������
					if (players[players[ip].nextip].hand_counter[n]) {//�� ��������� ������� ����� � ��� ��������� 
						players[ip].hand_counter[code] += players[players[ip].nextip].hand_counter[code];//��������� � �������
						players[players[ip].nextip].hand_counter[code] = 0;//�������� ������� ���������
						for (int i = 0; i < players[players[ip].nextip].hand.size(); ++i) {//���������� �� ������ ���������
							if (players[players[ip].nextip].hand[i].code == code) {//����� ����� ����� �������������� ����
								players[ip].hand.push_back(players[players[ip].nextip].hand[i]);//��������� ����� ����
								swap(players[players[ip].nextip].hand[i], (players[players[ip].nextip].hand[players[players[ip].nextip].hand.size() - 1]));//���������� "�����" ����� � �����
								players[players[ip].nextip].hand.pop_back();//����� ������� ��
							}
						}
						checkforchest(ip, code);//��������� ��������, � ��� ��� �������� �������� ������� ���� �� ����������� �����
						if (handsize(players[ip].nextip) == 0) {//��������� �� ������� �� �� ��������� ����� � ���������
							drawcard(players[ip].nextip);//������, ���� �����
						}
					}
					else {
						drawcard(ip);//����� ����� � ����� ����
						players[ip].myturn = false;//����������� ���
						players[players[ip].nextip].myturn = true;//�������� ��� ���������� ������
						msg = "no card";
					}
				}
				else
					msg = "it is not your turn";
			}
			else if (data == "info") {

			}
			else
				msg = "unknown cmd";
		}
		send(curr_sock, msg.c_str(), msg.size(), 0);
	}
	//closesocket(listener);
	closesocket(curr_sock);
	for (auto it = players.begin(); it != players.end(); ++it) {
		(*it).second.disconnectedguy = players[ip].name;
	}
	cout << players[ip].name << " left us :C" << endl;
}
bool choosenextplayer(bool pc = false) {
	if (players.size() == 1 || pc) {
		players["0.0.0.0"].nothread = false;
		players["0.0.0.0"].name = "server";
	}
	if (players.size() >= 2) {
		auto prev = players.begin();
		(*prev).second.myturn = true;
		if (players.size()) {
			auto now = ++(players.begin());
			for (;now != players.end(); ++now, ++prev) {
				(*prev).second.nextip = (*now).first;
			}
			(*prev).second.nextip = (*players.begin()).first;
		}
	}
	return players.size() >= 2;
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
			deck.resize(size);
		}
	}	
	gamestatus = choosenextplayer();
	/*
	//check for chests from start, restart if found
	for (auto it = players.begin(); it != players.end(); ++it) {
		for (int j = 0; j < 15; ++j) {
			if ((*it).second.hand_counter[j] == 4) {
				cout << "oops! chest from start, you are too lucky playerid:" << (*it).first << "!" << "I'm restarting the game!";//�������� ��� ������, ������ ������ ��������
				gamestatus = false;
				start_game_chest(deck.size());//������ ����������, �������� �� ���� ����
			}
		}
	}
	*/
	if (players["0.0.0.0"].name == "server") {

	}
	//server-player part


}
void waitforcon(SOCKET listener, sockaddr_in addr, int len) {
	SOCKET curr_sock = accept(listener, (struct sockaddr*)&addr, &len);
	string ip = inet_ntoa(addr.sin_addr);
	//0.0.0.0 is listener socket ip
	if (ip != "0.0.0.0" && players[ip].nothread) {
		players[ip].nothread = false;
		thread thr(TalkToClient, ip, curr_sock);
		thr.detach();
	}
	waitingforcon = false;
}
void ClientFirstConnects(int num_cards) {
	WSADATA wsd;
	WSAStartup(MAKEWORD(2, 2), &wsd);
	auto now = chrono::system_clock::now();
	time_t timeStart = chrono::system_clock::to_time_t(now);
	time_t timeNow = timeStart;
	int deltatime = 10;
	int counter = 0;
	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0); //������� ��������� �����
	if (listener == INVALID_SOCKET)
		cout << "Error with creating socket" << endl; //������ �������� ������
	fd_set list;
	list.fd_array[50] = listener;
	sockaddr_in addr; //������� � ��������� ���������� ��� �������� ������
	addr.sin_family = AF_INET; //��������� �������, ������� ����� ������������ ��� ������, � ��� ��� TCP/IP ������
	addr.sin_port = htons(3128); //���������� ������� htons ��� �������� ������ ����� � TCP/IP �������������
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY ��������, ��� ������ ����� ��������� ������� � ����� IP
	if (SOCKET_ERROR == ::bind(listener, (struct sockaddr*)&addr, sizeof(addr))) //��������� ����� � �������
		cout << "Error with binding socket";
	
	listen(listener, 3);//������� ���������� 1
	while (timeStart + deltatime > timeNow) {
		int len = sizeof(addr);
		
		if (!waitingforcon) {
			waitingforcon = true;
			thread wfc(waitforcon, listener, addr, len);
			wfc.detach();
		}
		timeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());
	}
	closesocket(listener);
	if (players.size() || 1) {
		cout << "starting with " << players.size() << " players" << endl;
		for (auto it = players.begin(); it != players.end(); ++it) {
			cout << it->second.name << " ";
		}
	}
	cout << endl;
	if (players.size() == 1) {
		cout << "look's like (s)he has no friends!" << endl;
		//players["0.0.0.0"].nothread = false;
	}
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

