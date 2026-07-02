#pragma once
#include <map>
#include <vector>
#include <string>
#include "Event.h"
#include "DataHandler.h"
#include "EventQueue.h"

struct EquityPoint {
    std::string date;
    double equity;
};

class Portfolio {
public:
    // riskPerTrade = fraction of *current cash* committed to a new long position.
    Portfolio(DataHandler& dataHandler, double initialCapital, double riskPerTrade = 0.95);

    void updateSignal(const SignalEvent& signal, EventQueue& queue);
    void updateFill(const FillEvent& fill);
    void updateTimeIndex(const std::string& date);

    const std::vector<EquityPoint>& getEquityCurve() const { return equityCurve_; }

private:
    DataHandler& dataHandler_;
    double initialCapital_;
    double cash_;
    double riskPerTrade_;
    std::map<std::string, int> positions_; // shares held, long-only (>=0)
    std::vector<EquityPoint> equityCurve_;
};
