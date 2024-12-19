#include "OrderList.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

Order::Order(const string& id, char type, int qty, double price, int time) : id(id), type(type), quantity(qty), limitPrice(price), arrivalTime(time) {}

bool Order::operator<(const Order& other) const {
	if (type == 'B') {
		if (isMarketOrder() != other.isMarketOrder()) {
			return other.isMarketOrder();
		}
		if (limitPrice != other.limitPrice) {
			return limitPrice < other.limitPrice;
		}
	} else if (type == 'S') {
		if (isMarketOrder() != other.isMarketOrder()) {
			return other.isMarketOrder();
		}
		if (limitPrice != other.limitPrice) {
			return limitPrice > other.limitPrice;
		}
	}
	return arrivalTime > other.arrivalTime;
}

bool Order::isMarketOrder() const {
	return 	limitPrice < 0;
}

OrderList::OrderList(double lastPrice) : lastTradedPrice(lastPrice) {}

void OrderList::processOrders(ifstream& inputFile, const string& outputFileName) {
	outputFile.open(outputFileName);
	if (!outputFile) {
		cerr << "Error opening file: " << outputFileName << endl;
		return;
	}

	string id;
	char type;
	int quantity;
	double price;
	int time = 0;

	while (inputFile >> id >> type >> quantity) {
		Order currentOrder = inputFile >> price ? Order(id, type, quantity, price, time++) : Order(id, type, quantity, -1, time++);
		inputFile.clear();

		if (type == 'B') {
			matchOrder(currentOrder, sellOrders, buyOrders);
		} else {
			matchOrder(currentOrder, buyOrders, sellOrders);
		}

		displayPendingOrders();
	}
	outputUnmatchedOrders();
}

void OrderList::matchOrder(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue) {
	vector<Order> incompatibleOrders = {};

	while (incoming.quantity > 0) {
		bool matchFound = false;

		while (!potentialMatches.empty() && !matchFound) {
			Order topOrder = potentialMatches.top();

			if (!isMatch(incoming, topOrder)) {
				incompatibleOrders.push_back(topOrder);
				potentialMatches.pop();
				continue;
			}

			matchFound = true;
			int tradedQuantity = min(incoming.quantity, topOrder.quantity);
			lastTradedPrice = determineExecutionPrice(incoming, topOrder);

			recordTrade(incoming, topOrder, tradedQuantity, lastTradedPrice);
			incoming.quantity -= tradedQuantity;
			topOrder.quantity -= tradedQuantity;

			potentialMatches.pop();
			if (topOrder.quantity > 0) {
				potentialMatches.push(topOrder);
			}
		}

		reinsertIncompatibleOrders(incompatibleOrders, potentialMatches);
		if (!matchFound) {
			break;
		}
	}

	if (incoming.quantity > 0 ) {
		sameQueue.push(incoming);
	}
}

float OrderList::determineExecutionPrice(const Order& incoming, const Order& top) const {
	Order buy = incoming.type == 'B' ? incoming : top;
	Order sell = incoming.type == 'S' ? incoming : top;

	if (!buy.isMarketOrder() && !sell.isMarketOrder()) {
	    return sell.arrivalTime < buy.arrivalTime ? sell.limitPrice : buy.limitPrice;
	} else if (buy.isMarketOrder() && sell.isMarketOrder()) {
		return lastTradedPrice;
	}
	return buy.isMarketOrder() ? sell.limitPrice : buy.limitPrice;
}

void OrderList::reinsertIncompatibleOrders(vector<Order> incompatibleOrders, priority_queue<Order>& queue) {
	for (const auto& order : incompatibleOrders) {
		queue.push(order);
	}
}

bool OrderList::isMatch(Order incoming, Order top) const {
	Order buy = incoming.type == 'B' ? incoming : top;
	Order sell = incoming.type == 'S' ? incoming : top;
	return buy.isMarketOrder() || sell.isMarketOrder() || buy.limitPrice >= sell.limitPrice;
}

void OrderList::displayPendingOrders() const {
	cout << "Last trading price: " << lastTradedPrice << endl;
	if (buyOrders.empty() && sellOrders.empty()) {
		cout << "No Pending Orders" << endl;
	} else {
		cout << fixed << setprecision(2);

		cout << "Buy                       Sell" << endl;
		cout << "------------------------------------------" << endl;

		auto buyQueueCopy = buyOrders;
		auto sellQueueCopy = sellOrders;

		while (!buyQueueCopy.empty() || !sellQueueCopy.empty()) {
			string buyLine = "";
			string sellLine = "";

			if (!buyQueueCopy.empty()) {
				const Order& topBuy = buyQueueCopy.top();
				buyLine = topBuy.id + " " + (topBuy.isMarketOrder() ? "M" : to_string(topBuy.limitPrice)) + " " + to_string(topBuy.quantity);
				buyQueueCopy.pop();
			}

			if (!sellQueueCopy.empty()) {
				const Order& topSell = sellQueueCopy.top();
				sellLine = topSell.id + " " + (topSell.isMarketOrder() ? "M" : to_string(topSell.limitPrice)) + " " + to_string(topSell.quantity);
				sellQueueCopy.pop();
			}
			cout << left << setw(25) << buyLine << sellLine << endl;
		}
	}
	cout << "\n\n";
}

void OrderList::recordTrade(const Order& incoming, const Order& top, int tradedQuantity, double executionPrice) {
	Order buy = incoming.type == 'B' ? incoming : top;
	Order sell = incoming.type == 'S' ? incoming : top;
	outputFile << "order " << buy.id << " " << tradedQuantity << " shares purchased at price " << fixed << setprecision(2) << executionPrice << endl;
	outputFile << "order " << sell.id << " " << tradedQuantity << " shares sold at price " << fixed << setprecision(2) << executionPrice << endl;
}

void OrderList::outputUnmatchedOrders() {
	vector<Order> unmatchedOrders;

	auto buyQueueCopy = buyOrders;
	while (!buyQueueCopy.empty()) {
		unmatchedOrders.push_back(buyQueueCopy.top());
		buyQueueCopy.pop();
	}

	auto sellQueueCopy = sellOrders;
	while (!sellQueueCopy.empty()) {
		unmatchedOrders.push_back(sellQueueCopy.top());
		sellQueueCopy.pop();
	}

	sort(unmatchedOrders.begin(), unmatchedOrders.end(), [](const Order& a, const Order& b) {
		return a.arrivalTime < b.arrivalTime;
	});

	for (const auto& order : unmatchedOrders) {
		outputFile << "order " << order.id << " " << order.quantity << " shares unexecuted" << endl;
	}
}