#include <iostream>
#include <thread>
#include <mutex>
#include <ctime>
#include <queue>

int getRandom(int min, int max)
{
    srand(time(nullptr));
    return rand() % (max - min + 1) + min;
}

struct OrderType
{
    int dish;
    int orderNumber{0};

    enum Dish {PIZZA, SOUP, STEAK, SALAD, SUSHI};

    static std::string typeDish(int dish) {
        if (dish == PIZZA) return "Pizza";
        else if (dish == SOUP) return "Soup";
        else if (dish == STEAK) return "Steak";
        else if (dish == SALAD) return "Salad";
        else return "Sushi";
    }

    friend std::ostream& operator << (std::ostream &out, OrderType &orderType);
};

std::ostream& operator << (std::ostream &out, OrderType &orderType)
{
    out << "#" << orderType.orderNumber << " - " << OrderType::typeDish(orderType.dish);
    return out;
}

class Kitchen
{
    static int numberDeliveries;
    int totalDelivery = 10;
    //bool kitchenIsFree = true;
    int timeOfDelivery = 30;

    std::queue<OrderType> orderQueue;
    std::queue<OrderType> deliveryQueue;

    std::mutex wait_orderQueue;
    std::mutex wait_deliveryQueue;

    void delivery()
    {
        while (totalDelivery > numberDeliveries)
        {
            std::this_thread::sleep_for(std::chrono::seconds(timeOfDelivery));

            if (!deliveryQueue.empty())
            {
                std::cout << "Order handed over to courier:" << std::endl;
                int sizeDeliveryQueue = deliveryQueue.size();
                for (int i = 0; i < sizeDeliveryQueue; ++i)
                {
                    std::cout << "\t" << i+1 << ")" << deliveryQueue.front() << std::endl;
                    wait_deliveryQueue.lock();
                    deliveryQueue.pop();
                    wait_deliveryQueue.unlock();
                }
                ++numberDeliveries;
            }
        }
    }

    void cooking()
    {
        while (totalDelivery > numberDeliveries)
        {
            if (!orderQueue.empty())
            {
                wait_orderQueue.lock();
                OrderType temp = orderQueue.front();
                orderQueue.pop();
                wait_orderQueue.unlock();

                std::cout << "The order has been sent to the kitchen: " << temp << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(getRandom(5,15)));
                std::cout << "The order: " << temp << " is ready!" << std::endl;

                wait_deliveryQueue.lock();
                deliveryQueue.push(temp);
                wait_deliveryQueue.unlock();
            }
        }
    }

    void order()
    {
        OrderType newOrder;
        while (totalDelivery > numberDeliveries)
        {
            std::this_thread::sleep_for(std::chrono::seconds(getRandom(5,10)));
            newOrder.dish = getRandom(0,4);
            ++newOrder.orderNumber;
            std::cout << "New order accepted: " << newOrder << std::endl;

            wait_orderQueue.lock();
            orderQueue.push(newOrder);
            wait_orderQueue.unlock();
        }
    }

public:
    void start()
    {
        std::thread orders(&Kitchen::order, this);
        std::thread cook(&Kitchen::cooking, this);
        std::thread deliver (&Kitchen::delivery, this);

        orders.detach();
        cook.detach();
        deliver.join();
    }
};

int Kitchen::numberDeliveries = 0;

int main()
{
    Kitchen kitchen;
    kitchen.start();

}
