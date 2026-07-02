#pragma once
#include <map>
#include <string>
#include "Event.h"
#include "DataHandler.h"
#include "EventQueue.h"

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual void calculateSignals(const MarketEvent& event, EventQueue& queue) = 0;
};

// Classic dual moving-average crossover: go long when the short SMA crosses
// above the long SMA, exit when it crosses back below. Long-only.
class SMACrossoverStrategy : public Strategy {
public:
    SMACrossoverStrategy(DataHandler& dataHandler, int shortWindow, int longWindow);
    void calculateSignals(const MarketEvent& event, EventQueue& queue) override;

private:
    DataHandler& dataHandler_;
    int shortWindow_;
    int longWindow_;
    // -1 = short SMA below long SMA, +1 = above, 0 = not enough data yet
    std::map<std::string, int> prevState_;
};

// Buys on the very first bar and never sells. Used as the benchmark that
// every "real" strategy should be compared against.
class BuyAndHoldStrategy : public Strategy {
public:
    explicit BuyAndHoldStrategy(DataHandler& dataHandler) : dataHandler_(dataHandler) {}
    void calculateSignals(const MarketEvent& event, EventQueue& queue) override;

private:
    DataHandler& dataHandler_;
    std::map<std::string, bool> bought_;
};
