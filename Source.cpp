﻿#include "digestpp/algorithm/sha3.hpp"
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

bool EXIT = false;
std::string ANSWER;

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

bool bruteOneThread(const size_t passlength, const string& hash, int block_size, const string& alphabet) {
	std::mutex mu;
	digestpp::sha3 hasher(256);
	string text(passlength, alphabet[0]);
	text[0] = alphabet[block_size * omp_get_thread_num()];
	int k = 0;
	int max_k = 2 * block_size;
	std::string hex;
	for (int i = 0; i < passlength - 1; ++i) {
		max_k *= alphabet.length();
	}
	double t1 = omp_get_wtime();
	for (k = 0; k < max_k && !EXIT; ++k) {
		hasher.absorb(text);
		hex = hasher.hexdigest();
		if (hex == hash) {
			double t2 = omp_get_wtime();
			ANSWER = text;
			EXIT = true;
			return true;
		}
		hasher.reset();
		auto ret = increment_str(text, alphabet);
		if (ret == false) {
			return false;
		}
	}
	return false;
}

pair<string, double> bruteforce_edited(const size_t passlength, const string& hash, const string& alphabet) {
	digestpp::sha3 hasher(256);
	string text(passlength, alphabet[0]);
	int k = 0;
	size_t i = 0;
	int completed = 0;
	std::string hex;
	std::string answer;
	double t1 = omp_get_wtime();
	std::mutex mu;
	omp_set_num_threads(12);
	#pragma omp parallel shared(hash, passlength, alphabet)
	{
		int block = alphabet.length() / omp_get_num_threads();
		bruteOneThread(passlength, hash, block, alphabet);
	}
	double t2 = omp_get_wtime();
	return { answer , t2-t1};
}

pair<string, double> bruteforce(const size_t passlength, const string& hash, const string& alphabet) {
	bool exit = false;
	digestpp::sha3 hasher(256); // Инициализация класса для выполнения хэширования
	string text(passlength, alphabet[0]); //Создание начальной предполагаемой строки
	int k = 0, k_max = 1;
	size_t i = 0;
	int completed = 0;
	std::string hex;
	std::string answer;
	for (int j = 0; j < passlength; ++j) {
		k_max *= alphabet.length();
	}
	omp_set_num_threads(12);
	double t1 = omp_get_wtime();
	#pragma omp parallel for shared(text, hash, hasher, passlength, alphabet, exit, answer, k_max) private(k, i, hex)
	for (k = 0; k < k_max; ++k) {
		#pragma omp critical
		{
			hasher.absorb(text); // Вычисление хэша
			hex = hasher.hexdigest();	//Приведение хэша к типу std::string
			if (hex == hash) {
				ANSWER = text;
				answer = text;
				exit = true;
			}
			hasher.reset(); // Сброс хэша
		}
		if (exit) {
			break;
		}
		auto res = increment_str(text, alphabet); //Увеличение строки на 1 (примерно как операция инкремента для числа с системой счисления alphabet.length())
		if (res == false) {
			//exit = true;
			break;
		}
	}
	double t2 = omp_get_wtime();
	return { answer , t2 - t1 };
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
		auto result = bruteforce_edited(4, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		std::cout << ANSWER << ' ' << result.second << '\n';
		result = bruteforce(4, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		std::cout << result.first << ' ' << result.second << '\n';
		writeHashWithResult("result.txt", hash, result.first);
	}*/
	long double averageTime = 0;
	std::vector<string> testStrings;
	for (size_t i = 0; i < 5; ++i) {
		testStrings.push_back(getRandomString(4));
		auto hash = getHash(testStrings[i]);
		auto result = bruteforce(4, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		std::cout << ANSWER << ' ' << result.second << '\n';
		logCalculation(testStrings[i], hash, result);
		averageTime += result.second;
	}
	std::cout << "AVERAGE DURATION WITH LENGTH 4: " << averageTime / 5 << std::endl;
	averageTime = 0;
	for (size_t i = 0; i < 5; ++i) {
		auto hash = getHash(testStrings[i]);
		auto result = bruteforce_edited(4, hash, "abcdefghijklmnopqrstuvwxyz0987654321");
		EXIT = false;
		std::cout << ANSWER << ' ' << result.second << '\n';
		logCalculation(testStrings[i], hash, std::pair<string, double>(ANSWER, result.second));
		averageTime += result.second;
	}
	std::cout << "AVERAGE DURATION WITH LENGTH 4: " << averageTime / 5 << std::endl;
}