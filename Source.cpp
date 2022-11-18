#include "digestpp/algorithm/sha3.hpp"
#include <fstream>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <istream>
#include <random>
#include <chrono>
#include <omp.h>

#include <mutex>
#include <windows.h>
using namespace std;

bool increment_str(string &str, const string& alphabet) {
	for (size_t i = 0; i < str.length(); ++i) {
		size_t numInAplhabet = alphabet.find(str[str.length() - 1 - i]);
		if (numInAplhabet == alphabet.length() - 1) {
			str[str.length() - 1 - i] = alphabet[0];
			if (i == str.length() - 1) {
				false;
			}
		}
		else {
			str[str.length() - 1 - i] = alphabet[numInAplhabet + 1];
			return true;
		}
	}
}



pair<string, double> bruteforce(const size_t passlength, const string& hash, const string& alphabet) {
	static bool exit = false;
	digestpp::sha3 hasher(256);
	string text(passlength, alphabet[0]);
	int k = 0;
	size_t i = 0;
	int completed = 0;
	std::string hex;
	std::string answer;
	double t1 = omp_get_wtime();
	std::mutex mu;
	omp_set_num_threads(1);
	#pragma omp parallel for shared(text, hash, hasher, passlength, alphabet, exit, answer) private(k, i, hex)
	for (k = 0; k < 1000000; ++k) {
		hasher.absorb(text);
		mu.lock();
		hex = hasher.hexdigest();
		if (hex == hash) {
			answer = text;
			exit = true;
			mu.unlock();
			break;
		}
		mu.unlock();
		hasher.reset();
		if (!exit) {
			auto res = increment_str(text, alphabet);
			if (res == false) {
				exit = true;
				break;
			}
		}
		else break;
	}
	double t2 = omp_get_wtime();
	return { answer , t2-t1};
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
	auto hashVector = readHash("hash.txt");
	for (auto hash : hashVector) {
		auto result = bruteforce(3, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		std::cout << result.first << ' ' << result.second << '\n';
		writeHashWithResult("result.txt", hash, result.first);
	}
	/*
	long double averageTime = 0;
	for (size_t i = 0; i < 5; ++i) {
		string str = getRandomString(5);
		auto hash = getHash(str);
		auto result = bruteforce(5, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		logCalculation(str, hash, result);
		averageTime += result.second;
	}
	std::cout << "AVERAGE DURATION WITH LENGTH 1: " << averageTime / 20 << std::endl;
	*/
}

/*
pair<string, double> bruteforce(size_t passlength, string hash, string alphabet){
	static bool exit = false;
	digestpp::sha3 hasher(256);
	string text(passlength, alphabet[0]);
	int completed = 0;
	int k = 0;
	size_t i = 0;
	auto t1 = omp_get_wtime();
	omp_set_num_threads();
	#pragma omp parallel for shared(passlength, hash, completed, alphabet, text) private(k, i)
	for (k = 0; k < 100000000; ++k) {
		hasher.absorb(text);
		auto hex = hasher.hexdigest(); // Получить хэш в виде hex-строки
		if (hex == hash) {
			cout << text << endl;
			completed = 1;
			break;
		}
		hasher.reset();
		for (i = 0; i < passlength && completed != -1; ++i) {
			size_t numInAplhabet = alphabet.find(text[text.length() - 1 - i]);
			if (numInAplhabet == alphabet.length() - 1) {
				text[text.length() - 1 - i] = alphabet[0];
				if (i == passlength - 1) {
					completed = -1;
					break;
				}
			}
			else {
				text[text.length() - 1 - i] = alphabet[numInAplhabet + 1];
				break;
			}
		}
		if (completed == -1) {
			break;
		}
	}
	auto t2 = omp_get_wtime();
	if (completed == 1)
		return { text, t2-t1 };
	else
		return { string(), -1 };

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
	auto hashVector = readHash("hash.txt");
	for (auto hash : hashVector) {
		auto result = bruteforce(3, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		writeHashWithResult("result.txt", hash, result.first);
	}

	long double averageTime = 0;
	for (size_t i = 0; i < 5; ++i) {
		string str = getRandomString(4);
		auto hash = getHash(str);
		auto result = bruteforce(4, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		logCalculation(str, hash, result);
		//averageTime += result.second;
	}
	//std::cout << "AVERAGE DURATION WITH LENGTH 1: " << averageTime / 5 << std::endl;
}*/