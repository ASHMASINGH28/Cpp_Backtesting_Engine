#include "Portfolio.h"
#include <cmath>

Portfolio::Portfolio(DataHandler& dataHandler, double initialCapital, double riskPerTrade)
    : dataHandler_(dataHandler), initialCapital_(initialCapital),
      cash_(initialCapital), riskPerTrade_(riskPerTrade) {}

void Portfolio::updateSignal(const SignalEvent& signal, EventQueue& queue) {
    auto bars = dataHandler_.getLatestBars(signal.symbol, 1);
    if (bars.empty()) return;
    double price = bars.back().adjClose;
    if (price <= 0.0) return;

    int currentShares = positions_.count(signal.symbol) ? positions_[signal.symbol] : 0;

    if (signal.signalType == SignalType::LONG && currentShares == 0) {
        double allocation = cash_ * riskPerTrade_;
        int qty = static_cast<int>(std::floor(allocation / price));
        if (qty > 0) {
            queue.push(std::make_shared<OrderEvent>(signal.symbol, OrderType::MARKET, qty, Direction::BUY));
        }
    } else if (signal.signalType == SignalType::EXIT && currentShares > 0) {
        queue.push(std::make_shared<OrderEvent>(signal.symbol, OrderType::MARKET, currentShares, Direction::SELL));
    }
}

void Portfolio::updateFill(const FillEvent& fill) {
    int& shares = positions_[fill.symbol]; // default-inits to 0
    if (fill.direction == Direction::BUY) {
        cash_ -= (fill.fillPrice * fill.quantity + fill.commission);
        shares += fill.quantity;
    } else {
        cash_ += (fill.fillPrice * fill.quantity - fill.commission);
        shares -= fill.quantity;
    }
}

void Portfolio::updateTimeIndex(const std::string& date) {
    double holdingsValue = 0.0;
    for (const auto& [symbol, qty] : positions_) {
        if (qty <= 0) continue;
        auto bars = dataHandler_.getLatestBars(symbol, 1);
        if (bars.empty()) continue;
        holdingsValue += qty * bars.back().adjClose;
    }
    equityCurve_.push_back({date, cash_ + holdingsValue});
}
