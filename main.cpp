#include "OrderList.h"
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]) {
	cout << fixed << setprecision(2);

	if (argc != 2) {
		cerr << "Usage: " << argv[0] << " <input file>" << endl;
		return 1;
	}

	string inputFileName = argv[1];
	string outputFileName = string(inputFileName).replace(inputFileName.find("input"), 5, "output");

	ifstream inputFile(inputFileName);
	if (!inputFile) {
		cerr << "Error opening file: " << inputFileName << endl;
		return 1;
	}

	double lastTradedPrice;
	inputFile >> lastTradedPrice;

	OrderList orders(lastTradedPrice);
	orders.processOrders(inputFile, outputFileName);

	return 0;
}
