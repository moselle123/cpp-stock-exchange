#include "OrderList.h"
#include <iostream>
#include <iomanip>

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

void OrderList::processOrders(ifstream& inputFile, const string& outputFile) {
	ofstream outfile(outputFile);
	if (!outfile) {
		cerr << "Error opening file: " << outputFile << endl;
		return;
	}

	string id;
	char type;
	int quantity;
	double price;
	int time = 0;

	while (inputFile >> id >> type >> quantity) {
		Order currentOrder = inputFile >> price ? Order(id, type, quantity, price, time++) : Order(id, 'B', quantity, -1, time++);
		inputFile.clear();

		if (type == 'B') {
			matchOrder(currentOrder, sellOrders, buyOrders);
		} else {
			matchOrder(currentOrder, buyOrders, sellOrders);
		}

		displayPendingOrders();
	}
}

void OrderList::matchOrder(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue) {
	vector<Order> incompatibleOrders = {};
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
            	float executionPrice = determineExecutionPrice(incoming, topOrder);

		// recordTrade(incoming, topOrder, tradedQuantity, executionPrice);

		incoming.quantity -= tradedQuantity;
		topOrder.quantity -= tradedQuantity;

		potentialMatches.pop();
		if (topOrder.quantity > 0) {
			potentialMatches.push(topOrder);
		}
	}

	reinsertIncompatibleOrders(incompatibleOrders, potentialMatches);
	if (!matchFound) {
		sameQueue.push(incoming);
	}  else if (incoming.quantity > 0) {
		matchOrder(incoming, potentialMatches, sameQueue);
	}
}

float OrderList::determineExecutionPrice(const Order& incoming, const Order& top) const {
	Order buy = incoming.type == 'B' ? incoming : top;
	Order sell = incoming.type == 'S' ? incoming : top;
        if (!buy.isMarketOrder() && !sell.isMarketOrder()) {
            return sell.arrivalTime < buy.arrivalTime ? sell.limitPrice : buy.limitPrice;
        }
        return buy.isMarketOrder() ? sell.limitPrice : buy.limitPrice;
}

void OrderList::reinsertIncompatibleOrders(vector<Order> incompatibleOrders, priority_queue<Order>& queue) {
	while (!incompatibleOrders.empty()) {
		queue.push(incompatibleOrders.front());
		incompatibleOrders.pop_back();
        }
}

bool OrderList::isMatch(Order incoming, Order top) const {
	Order buy = incoming.type == 'B' ? incoming : top;
	Order sell = incoming.type == 'S' ? incoming : top;
	return buy.isMarketOrder() || sell.isMarketOrder() || buy.limitPrice >= sell.limitPrice;
}

void OrderList::displayPendingOrders() const {
	// Implementation to display pending orders on the screen
}

// void OrderList::recordTrade(const Order& buy, const Order& sell, int quantity, float price) {
//         std::ostringstream trade;
//         trade << "order " << buy.id << " " << quantity << " shares purchased at price "
//               << std::fixed << std::setprecision(2) << price << "\n";
//         trade << "order " << sell.id << " " << quantity << " shares sold at price "
//               << std::fixed << std::setprecision(2) << price << "\n";
//         executedOrders.push_back(trade.str());
//         lastTradedPrice = price;
// }