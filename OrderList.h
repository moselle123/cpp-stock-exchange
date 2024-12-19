#include <queue>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

class Order {
	string id_;
	char type_;
	int quantity_;
	double limitPrice_;
	int arrivalTime_;

	bool isMarketOrder_() const;
public:
	Order(const std::string& id, char type, int qty, double price, int time);
	bool operator<(const Order& other) const;

	friend class OrderList;
};

class OrderList {
private:
	ofstream outputFile_;
	priority_queue<Order> buyOrders_;
	priority_queue<Order> sellOrders_;
	double lastTradedPrice_;

	void displayPendingOrders_() const;
	void matchOrder_(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue);
	float getExecutionPrice_(const Order& incoming, const Order& top) const;
	void reinsertIncompatibleOrders_(vector<Order> incompatibleOrders, priority_queue<Order>& queue);
	bool isMatch_(Order incoming, Order top) const;
	void outputTrade_(const Order& buy, const Order& sell, int tradedQuantity, double executionPrice);
	void outputUnmatchedOrders_();
public:
	explicit OrderList(double lastPrice);

	void processOrders(ifstream& inputFile, const string& outputFile);
};
