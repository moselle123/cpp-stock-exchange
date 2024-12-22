#include "OrderList.h"

Order::Order(const string& id, char type, int qty, double price, int time) : id_(id), type_(type), quantity_(qty), limitPrice_(price), arrivalTime_(time) {}

Order::~Order() {}

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

OrderList::~OrderList() {
	if (outputFile_.is_open()) {
		outputFile_.close();
	}
}

void OrderList::processOrders(ifstream& inputFile, const string& outputFileName) {
	outputFile_.open(outputFileName);
	if (!outputFile_) {
		cerr << "Error opening file: " << outputFileName << endl;
		return;
	}

	outputFile_ << fixed << setprecision(2);

	string id;
	char type;
	int quantity;
	double price;
	int time = 0;

	while (inputFile >> id >> type >> quantity) {
		Order currentOrder = inputFile >> price ? Order(id, type, quantity, price, time++) : Order(id, type, quantity, -1, time++);
		inputFile.clear();

		displayPendingOrders_(currentOrder);

		if (type == 'B') {
			matchOrder_(currentOrder, sellOrders_, buyOrders_);
		} else {
			matchOrder_(currentOrder, buyOrders_, sellOrders_);
		}
	}
	outputUnmatchedOrders_();
}

void OrderList::matchOrder_(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue) {
	vector<Order> incompatibleOrders = {};

	while (incoming.quantity_ > 0) {
		bool matchFound = false;

		while (!potentialMatches.empty() && !matchFound) {
			Order topOrder = potentialMatches.top();

			Order buy = (incoming.type_ == 'B') ? incoming : topOrder;
			Order sell = (incoming.type_ == 'S') ? incoming : topOrder;

			if (!isMatch_(buy, sell)) {
				incompatibleOrders.push_back(topOrder);
				potentialMatches.pop();
				continue;
			}

			matchFound = true;
			int tradedQuantity = min(incoming.quantity_, topOrder.quantity_);
			lastTradedPrice_ = getExecutionPrice_(buy, sell);

			outputTrade_(buy, sell, tradedQuantity, lastTradedPrice_);
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

float OrderList::getExecutionPrice_(const Order& buy, const Order& sell) const {
	if (!buy.isMarketOrder_() && !sell.isMarketOrder_()) {
		return (sell.arrivalTime_ < buy.arrivalTime_) ? sell.limitPrice_ : buy.limitPrice_;
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

bool OrderList::isMatch_(const Order& buy, const Order& sell) const  {
	return buy.isMarketOrder_() || sell.isMarketOrder_() || buy.limitPrice_ >= sell.limitPrice_;
}

void OrderList::displayPendingOrders_(const Order& incoming) const {
	cout << "Last trading price: " << lastTradedPrice_ << endl;
	cout << "Buy                        Sell" << endl;
	cout << "------------------------------------------------" << endl;

	vector<Order> buyOrdersReversed;
	vector<Order> sellOrdersReversed;

	incoming.type_ == 'B' ? buyOrdersReversed.push_back(incoming) : sellOrdersReversed.push_back(incoming);

	auto buyQueueCopy = buyOrders_;
	while (!buyQueueCopy.empty()) {
		buyOrdersReversed.push_back(buyQueueCopy.top());
		buyQueueCopy.pop();
	}

	auto sellQueueCopy = sellOrders_;
	while (!sellQueueCopy.empty()) {
		sellOrdersReversed.push_back(sellQueueCopy.top());
		sellQueueCopy.pop();
	}

	while (!buyOrdersReversed.empty() || !sellOrdersReversed.empty()) {
		if (!buyOrdersReversed.empty()) {
			const Order& buy = buyOrdersReversed.back();
			cout << left << setw(9) << buy.id_;
			if (buy.isMarketOrder_()) {
				cout << setw(8) << "M";
			} else {
				cout << setw(8) << buy.limitPrice_;
			}
			cout << setw(5) << buy.quantity_ << setw(5) << "";;
			buyOrdersReversed.pop_back();
		} else {
			cout << left << setw(27) << "";
		}

		if (!sellOrdersReversed.empty()) {
			const Order& sell = sellOrdersReversed.back();
			cout << left << setw(9) << sell.id_;
			if (sell.isMarketOrder_()) {
				cout << setw(8) << "M";
			} else {
				cout << setw(8) << sell.limitPrice_;
			}
			cout << setw(5) << sell.quantity_;
			sellOrdersReversed.pop_back();
		}
		cout << endl;
	}
	cout << "\n";
}

void OrderList::outputTrade_(const Order& buy, const Order& sell, int tradedQuantity, double executionPrice) {
	if (firstLine) {
		firstLine = false;
	} else {
		outputFile_ << "\n";
	}
	outputFile_ << "order " << buy.id_ << " " << tradedQuantity << " shares purchased at price " << executionPrice << endl;
	cout << "order " << buy.id_ << " " << tradedQuantity << " shares purchased at price " << executionPrice << endl;
	outputFile_ << "order " << sell.id_ << " " << tradedQuantity << " shares sold at price " << executionPrice;
	cout << "order " << sell.id_ << " " << tradedQuantity << " shares sold at price " << executionPrice << "\n\n";
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
		outputFile_ << endl << "order " << order.id_ << " " << order.quantity_ << " shares unexecuted";
	}
}