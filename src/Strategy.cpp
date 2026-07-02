#include "Strategy.h"
#include <numeric>

SMACrossoverStrategy::SMACrossoverStrategy(DataHandler& dataHandler, int shortWindow, int longWindow)
    : dataHandler_(dataHandler), shortWindow_(shortWindow), longWindow_(longWindow) {}

void SMACrossoverStrategy::calculateSignals(const MarketEvent& event, EventQueue& queue) {
    auto bars = dataHandler_.getLatestBars(event.symbol, longWindow_);
    if (static_cast<int>(bars.size()) < longWindow_) return; // not enough history yet

    double shortSum = 0.0, longSum = 0.0;
    for (int i = 0; i < longWindow_; ++i) longSum += bars[bars.size() - 1 - i].adjClose;
    for (int i = 0; i < shortWindow_; ++i) shortSum += bars[bars.size() - 1 - i].adjClose;

    double smaShort = shortSum / shortWindow_;
    double smaLong  = longSum / longWindow_;

    int newState = (smaShort > smaLong) ? 1 : -1;
    int oldState = prevState_.count(event.symbol) ? prevState_[event.symbol] : 0;

    if (oldState != 1 && newState == 1) {
        queue.push(std::make_shared<SignalEvent>(event.symbol, event.date, SignalType::LONG));
    } else if (oldState == 1 && newState == -1) {
        queue.push(std::make_shared<SignalEvent>(event.symbol, event.date, SignalType::EXIT));
    }

    prevState_[event.symbol] = newState;
}

void BuyAndHoldStrategy::calculateSignals(const MarketEvent& event, EventQueue& queue) {
    if (!bought_[event.symbol]) {
        queue.push(std::make_shared<SignalEvent>(event.symbol, event.date, SignalType::LONG));
        bought_[event.symbol] = true;
    }
}
