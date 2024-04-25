#include "MyAllocator.h"
#include <vector>
#include <iostream>

using namespace std;

int main() {
	vector<int, MyAllocator<int>> vec;
	for (int i = 0; i < 100; ++i) {
		vec.emplace_back(rand() % 1000);
	}
	for (int val : vec) {
		cout << val << " ";
	}
	cout << endl;
	return 0;
}