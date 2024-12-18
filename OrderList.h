#include <queue>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

class Order {
	string id;
	char type;
	int quantity;
	double limitPrice;
	int arrivalTime;

public:
	Order(const std::string& id, char type, int qty, double price, int time);
	bool operator<(const Order& other) const;
	bool isMarketOrder() const;

	friend class OrderList;
};

class OrderList {
private:
	priority_queue<Order> buyOrders;
	priority_queue<Order> sellOrders;
	double lastTradedPrice;

public:
	explicit OrderList(double lastPrice);

	void processOrders(ifstream& inputFile, const string& outputFile);
	void displayPendingOrders() const;
	void matchOrder(Order incoming, priority_queue<Order>& potentialMatches, priority_queue<Order>& sameQueue);
	float determineExecutionPrice(const Order& incoming, const Order& top) const;
	void reinsertIncompatibleOrders(vector<Order> incompatibleOrders, priority_queue<Order>& queue);
	bool isMatch(Order incoming, Order top) const;
};
