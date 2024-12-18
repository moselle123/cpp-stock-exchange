#include "OrderList.h"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <input file>" << endl;
		return 1;
	}

	string inputFile = argv[1];
	string outputFile = "output" + inputFile.substr(5);

	ifstream file(inputFile);
	if (!file) {
		cerr << "Error opening file: " << inputFile << endl;
		return 1;
	}

	double lastTradedPrice;
	file >> lastTradedPrice;

	OrderList orders(lastTradedPrice);
	orders.processOrders(file, outputFile);

	return 0;
}
