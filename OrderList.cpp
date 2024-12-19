#include "OrderList.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

Order::Order(const string& id, char type, int qty, double price, int time) : id_(id), type_(type), quantity_(qty), limitPrice_(price), arrivalTime_(time) {}

bool Order::operator<(const Order& other) const {
	if (type_ == 'B') {
		if (isMarketOrder_() != other.isMarketOrder_()) {
			return other.isMarketOrder_();
		}
		if (limitPrice_ != other.limitPrice_) {
			return limitPrice_ < other.limitPrice_;
		}
	} else if (type_ == 'S') {
		if (isMarketOrder_() != other.isMarketOrder_()) {
			return other.isMarketOrder_();
		}
		if (limitPrice_ != other.limitPrice_) {
			return limitPrice_ > other.limitPrice_;
		}
	}
	return arrivalTime_ > other.arrivalTime_;
}

bool Order::isMarketOrder_() const {
	return 	limitPrice_ < 0;
}

OrderList::OrderList(double lastPrice) : lastTradedPrice_(lastPrice) {}

void OrderList::processOrders(ifstream& inputFile, const string& outputFileName) {
	outputFile_.open(outputFileName);
	if (!outputFile_) {
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
			matchOrder_(currentOrder, sellOrders_, buyOrders_);
		} else {
			matchOrder_(currentOrder, buyOrders_, sellOrders_);
		}

		displayPendingOrders_();
	}
	outputUnmatchedOrders_();
}

void OrderList::matchOrder_(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue) {
	vector<Order> incompatibleOrders = {};

	while (incoming.quantity_ > 0) {
		bool matchFound = false;

		while (!potentialMatches.empty() && !matchFound) {
			Order topOrder = potentialMatches.top();

			if (!isMatch_(incoming, topOrder)) {
				incompatibleOrders.push_back(topOrder);
				potentialMatches.pop();
				continue;
			}

			matchFound = true;
			int tradedQuantity = min(incoming.quantity_, topOrder.quantity_);
			lastTradedPrice_ = getExecutionPrice_(incoming, topOrder);

			outputTrade_(incoming, topOrder, tradedQuantity, lastTradedPrice_);
			incoming.quantity_ -= tradedQuantity;
			topOrder.quantity_ -= tradedQuantity;

			potentialMatches.pop();
			if (topOrder.quantity_ > 0) {
				potentialMatches.push(topOrder);
			}
		}

		reinsertIncompatibleOrders_(incompatibleOrders, potentialMatches);
		if (!matchFound) {
			break;
		}
	}

	if (incoming.quantity_ > 0 ) {
		sameQueue.push(incoming);
	}
}

float OrderList::getExecutionPrice_(const Order& incoming, const Order& top) const {
	Order buy = incoming.type_ == 'B' ? incoming : top;
	Order sell = incoming.type_ == 'S' ? incoming : top;

	if (!buy.isMarketOrder_() && !sell.isMarketOrder_()) {
		return sell.arrivalTime_ < buy.arrivalTime_ ? sell.limitPrice_ : buy.limitPrice_;
	} else if (buy.isMarketOrder_() && sell.isMarketOrder_()) {
		return lastTradedPrice_;
	}
	return buy.isMarketOrder_() ? sell.limitPrice_ : buy.limitPrice_;
}

void OrderList::reinsertIncompatibleOrders_(vector<Order> incompatibleOrders, priority_queue<Order>& queue) {
	for (const auto& order : incompatibleOrders) {
		queue.push(order);
	}
}

bool OrderList::isMatch_(Order incoming, Order top) const {
	Order buy = incoming.type_ == 'B' ? incoming : top;
	Order sell = incoming.type_ == 'S' ? incoming : top;
	return buy.isMarketOrder_() || sell.isMarketOrder_() || buy.limitPrice_ >= sell.limitPrice_;
}

void OrderList::displayPendingOrders_() const {
	cout << "Last trading price: " << lastTradedPrice_ << endl;
	if (buyOrders_.empty() && sellOrders_.empty()) {
		cout << "No Pending Orders" << endl;
	} else {
		cout << fixed << setprecision(2);

		cout << "Buy                       Sell" << endl;
		cout << "------------------------------------------" << endl;

		auto buyQueueCopy = buyOrders_;
		auto sellQueueCopy = sellOrders_;

		while (!buyQueueCopy.empty() || !sellQueueCopy.empty()) {
			string buyLine = "";
			string sellLine = "";

			if (!buyQueueCopy.empty()) {
				const Order& topBuy = buyQueueCopy.top();
				buyLine = topBuy.id_ + " " + (topBuy.isMarketOrder_() ? "M" : to_string(topBuy.limitPrice_)) + " " + to_string(topBuy.quantity_);
				buyQueueCopy.pop();
			}

			if (!sellQueueCopy.empty()) {
				const Order& topSell = sellQueueCopy.top();
				sellLine = topSell.id_ + " " + (topSell.isMarketOrder_() ? "M" : to_string(topSell.limitPrice_)) + " " + to_string(topSell.quantity_);
				sellQueueCopy.pop();
			}
			cout << left << setw(25) << buyLine << sellLine << endl;
		}
	}
	cout << "\n\n";
}

void OrderList::outputTrade_(const Order& incoming, const Order& top, int tradedQuantity, double executionPrice) {
	Order buy = incoming.type_ == 'B' ? incoming : top;
	Order sell = incoming.type_ == 'S' ? incoming : top;
	outputFile_ << "order " << buy.id_ << " " << tradedQuantity << " shares purchased at price " << fixed << setprecision(2) << executionPrice << endl;
	outputFile_ << "order " << sell.id_ << " " << tradedQuantity << " shares sold at price " << fixed << setprecision(2) << executionPrice << endl;
}

void OrderList::outputUnmatchedOrders_() {
	vector<Order> unmatchedOrders;

	auto buyQueueCopy = buyOrders_;
	while (!buyQueueCopy.empty()) {
		unmatchedOrders.push_back(buyQueueCopy.top());
		buyQueueCopy.pop();
	}

	auto sellQueueCopy = sellOrders_;
	while (!sellQueueCopy.empty()) {
		unmatchedOrders.push_back(sellQueueCopy.top());
		sellQueueCopy.pop();
	}

	sort(unmatchedOrders.begin(), unmatchedOrders.end(), [](const Order& a, const Order& b) {
		return a.arrivalTime_ < b.arrivalTime_;
	});

	for (const auto& order : unmatchedOrders) {
		outputFile_ << "order " << order.id_ << " " << order.quantity_ << " shares unexecuted" << endl;
	}
}