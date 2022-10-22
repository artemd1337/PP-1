#include "digestpp/algorithm/sha3.hpp"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <istream>
#include <random>
#include <chrono>
using namespace std;

pair<string, double> bruteforce(const size_t passlength, const string& hash, const string& alphabet) {
	static bool exit = false;
	digestpp::sha3 hasher(256);
	string text(passlength, alphabet[0]);
	auto start = chrono::steady_clock::now();
	while (true) {
		hasher.absorb(text);
		auto hex = hasher.hexdigest(); // Получить хэш в виде hex-строки
		if (hex == hash) {
			auto end = chrono::steady_clock::now();
			auto diff = end - start;
			double duration = chrono::duration <double, milli>(diff).count();
			return { text , duration};
		}
		hasher.reset();
		for (size_t i = 0; i < passlength; ++i) {
			size_t numInAplhabet = alphabet.find(text[text.length() - 1 - i]);
			if (numInAplhabet == alphabet.length() - 1) {
				text[text.length() - 1 - i] = alphabet[0];
				if (i == passlength - 1) return { string(), -1 };
			}
			else {
				text[text.length() - 1 - i] = alphabet[numInAplhabet + 1];
				break;
			}
		}
	}
}

vector<string> readHash(const string& filepath) {
	ifstream input(filepath);
	string hash;
	vector<string> ans;
	while (input >> hash) {
		if (!hash.empty()) ans.push_back(hash);
	}
	return ans;
}

void writeHashWithResult(const string& filepath, const string& hash, const string& result) {
	ofstream output(filepath, std::ios_base::app);
	output << hash << ':' << result << std::endl;
}

string getRandomString(size_t length) {
	string result;
	for (size_t i = 0; i < length; ++i) {
		int randnum = rand() % 36;
		if (randnum < 10)
			result.push_back(char('0' + randnum));
		else
			result.push_back(char('a' + randnum - 10));
	}
	return result;
}

string getHash(string str) {
	digestpp::sha3 hasher(256);
	hasher.absorb(str);
	auto hex = hasher.hexdigest();
	return hex;
}

void logCalculation(string& str, string& hash, pair<string, double> bruteforceResult) {
	ofstream output("BruteforceLog.txt", std::ios_base::app);
	output << str << " : " << hash << " : " << bruteforceResult.second << " ms : " << bruteforceResult.first << std::endl;
}

int main() {
	srand(time(0));
	/*auto hashVector = readHash("hash.txt");
	for (auto hash : hashVector) {
		auto result = bruteforce(3, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		writeHashWithResult("result.txt", hash, result.first);
	}*/
	
	long double averageTime = 0;
	for (size_t i = 0; i < 5; ++i) {
		string str = getRandomString(5);
		auto hash = getHash(str);
		auto result = bruteforce(5, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		logCalculation(str, hash, result);
		averageTime += result.second;
	}
	std::cout << "AVERAGE DURATION WITH LENGTH 1: " << averageTime / 20 << std::endl;
}