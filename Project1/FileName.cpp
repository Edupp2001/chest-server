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
				--i;//������ ��� ����� � ����� ���� ����� ���������
			}
		}
	}
	if (handsize(ip) == 0 && deck.size()) {
		Card topdeck = Card(deck[deck.size() - 1].id);//����� ������
		int topdeckcode = topdeck.code;//����� ��� �������
		deck.pop_back();//������ ����� � �������
		players[ip].hand.push_back(topdeck);//������ � ����
		players[ip].hand_counter[topdeckcode]++;//����������� �������
	}
}
int drawcard(string ip) {
	if (deck.size() == 0)
		return 0;
	else {
		Card topdeck = Card(deck[deck.size() - 1].id);//����� ������
		int topdeckcode = topdeck.code;//����� ��� �������
		deck.pop_back();//������ ����� � �������
		players[ip].hand.push_back(topdeck);//������ � ����
		players[ip].hand_counter[topdeckcode]++;//����������� �������
		checkforchest(ip, topdeckcode);//��������� ��������
		return deck.size() + 1;
	}
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
			if (players[ip].disconnectedguy != "") {
				data = "quit";
				msg = "game is finished";
				send(curr_sock, msg.c_str(), msg.size(), 0);
				msg = players[ip].disconnectedguy + " has diconnected game is over";
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
				if (data == "j") {
					code = 11;
					data = "J";
				}
				else if (data == "q") {
					code = 12;
					data = "Q";
				}
				else if (data == "k") {
					code = 13;
					data = "K";
				}
				else if (data == "a") {
					code = 14;
					data = "A";
				}
				else
					code = STI(data);
				if (players[ip].myturn) {//���� ��� ������
					players[players[ip].nextip].StolenCardsHistory.push_back(data);
					if (players[players[ip].nextip].hand_counter[code]) {//�� ��������� ������� ����� � ��� ��������� 
						players[ip].hand_counter[code] += players[players[ip].nextip].hand_counter[code];//��������� � �������
						players[players[ip].nextip].hand_counter[code] = 0;//�������� ������� ���������
						for (int i = 0; i < players[players[ip].nextip].hand.size(); ++i) {//���������� �� ������ ���������
							if (players[players[ip].nextip].hand[i].code == code) {//����� ����� ����� �������������� ����
								players[ip].hand.push_back(players[players[ip].nextip].hand[i]);//��������� ����� ����
								swap(players[players[ip].nextip].hand[i], (players[players[ip].nextip].hand[players[players[ip].nextip].hand.size() - 1]));//���������� "�����" ����� � �����
								players[players[ip].nextip].hand.pop_back();//����� ������� ��
								--i;//������ ��� �� �� ��������� �����, ��� ���� � �����
							}
						}
						
						checkforchest(ip, code);//��������� ��������, � ��� ��� �������� �������� ������� ���� �� ����������� �����
						if (handsize(players[ip].nextip) == 0) {//��������� �� ������� �� �� ��������� ����� � ���������
							while (drawcard(players[ip].nextip) == 0 && players[ip].nextip != ip)//������, ���� �����
								players[ip].nextip = players[players[ip].nextip].nextip;//���� ������ ����� � � ���������� ������ ��� ����, ���� ��� ���� �����������
						}
						msg = "good choice, your turn again";
						
					}
					else {
						msg = "no card";
						if (drawcard(ip)) {//����� ����� � ����� ����
							Card topdeck = players[ip].hand[players[ip].hand.size() - 1];
							msg += ", you draw " + topdeck.value.first + " of " + topdeck.value.second;
						}
						players[ip].myturn = false;//����������� ���
						players[players[ip].nextip].myturn = true;//�������� ��� ���������� ������
					}
				}
				else
					msg = "it is not your turn";
				
			}
			else if (data == "info") {
				msg = "";
				for (auto it = players.begin(); it != players.end(); ++it) {
					msg += (*it).second.name + ": " + ITS((*it).second.hand.size()) + " ";
				}
				msg += "\n";
				for (int i = 0; i < players[ip].StolenCardsHistory.size(); ++i) {
					msg += players[ip].StolenCardsHistory[i] + " ";
				}
				msg += "\n";
				msg += "decksize: " + ITS(deck.size());
			}
			else
				msg = ":" + data + ":" "is unknown cmd";
		}
		if (players[ip].nextip == ip) {
			msg = "game is finished";
			send(curr_sock, msg.c_str(), msg.size(), 0);
			vector <string> winnerip;
			int maxchest = 0;
			for (auto it = players.begin(); it != players.end(); ++it) {
				msg += (*it).second.name + " ";
				if (maxchest < (*it).second.chest.size()) {
					winnerip.clear();
					maxchest = ((*it).second.chest.size());
				}
				else if (maxchest == ((*it).second.chest.size())) {
					winnerip.push_back((*it).first);
				}
				for (int i = 0; i < (*it).second.chest.size(); ++i) {
					msg += (*it).second.chest[i] + " ";
				}
			}
			msg += "";
			msg += "winner" + (winnerip.size() == 1) ? " is " : "s are ";
			for (int i = 0; i < winnerip.size(); ++i) {
				msg += players[winnerip[i]].name + " ";
			}
			msg += "with " + ITS(players[winnerip[0]].chest.size());
			data = "quit";
		}
		send(curr_sock, msg.c_str(), msg.size(), 0);
		if (data != "quit") {
			while (!players[ip].myturn) {
				//:C
			}
			msg = "your turn!" + players[ip].StolenCardsHistory.size() ? " enemy asked for " + players[ip].StolenCardsHistory[players[ip].StolenCardsHistory.size() - 1] : "";
			send(curr_sock, msg.c_str(), msg.size(), 0);
		}
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
	deck = create_deck(size);
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
	//if (players["0.0.0.0"].name == "server") {

	//}
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
	int deltatime = 15;
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
		if (input == "start_chest36" || input == "1") {
			ClientFirstConnects(36);
		}
		else if (input == "start_chest52") {
			ClientFirstConnects(52);
		}
		else {
			cout << "no" << endl;
		}
	}
	
 }

