#pragma once
#include <iostream>
#include <vector>
using namespace std;
void to_lower(char& s) {
	if (s >= 'A' && s <= 'Z')
		s += 'a' - 'A';
}
string to_lower(string S) {
	for (int i = 0; i < S.size(); ++i) {
		to_lower(S[i]);
	}
	return S;
}
string ITS(int a) {
	string tmp, ans;
	if (a < 0)
		ans += '-';
	if (a == 0)
		return "0";
	while (a) {
		tmp += abs(a % 10) + '0';
		a /= 10;
	}
	for (int i = 0; i < tmp.size(); ++i) {
		ans += tmp[tmp.size() - i - 1];
	}
	return ans;
}
int STI(string a) {
	long long b = 0;
	for (int i = 0; i < a.size(); ++i) {
		if (a[i] >= '0' && a[i] <= '9') {
			b = (a[i] - '0') + b * 10;
		}
	}
	if (a[0] == '-')
		return -b;
	else
		return b;
}
class Card {
public:
	int id;//id -> card
	int code;//code -> [2, A]
	pair <string, string> value;//card -> [2, A] + suit
	Card(int n) {
		this->id = n;
		int num = n % 13 + 2;
		this->code = num;
		int suit = n / 13;
		switch (suit) {
		case 0:
			this->value.second = "Diamonds";//бубны
			break;
		case 1:
			this->value.second = "Hearts";//червы
			break;
		case 2:
			this->value.second = "Spades";//пики
			break;
		case 3:
			this->value.second = "Clubs";//трефы
			break;
		}
		if (num <= 10) {
			this->value.first = ITS(num);
		}
		else switch (num) {
		case 11:
			this->value.first = "J";
			break;
		case 12:
			this->value.first = "Q";
			break;
		case 13:
			this->value.first = "K";
			break;
		case 14:
			this->value.first = "A";
			break;
		}
	}
	friend ostream& operator << (ostream& out, const Card& c) {
		out << c.value.first << " of " << c.value.second << endl;
		return out;
	}
};
vector <Card> create_deck(int size) {
	vector <Card> cards;
	vector <Card> new_deck;
	if (size == 36) {
		for (int i = 0; i < 52; ++i)
			if (i % 13 > 3)
				cards.push_back(Card(i));
	}
	else {
		for (int i = 0; i < 52; ++i)
			cards.push_back(Card(i));
	}
	size = cards.size();
	for (int i = 0; i < size; ++i) {
		int rnd = rand() % cards.size();
		new_deck.push_back(cards[rnd]);
		swap(cards[rnd], cards[cards.size() - 1]);
		cards.pop_back();
	}
	return new_deck;
}