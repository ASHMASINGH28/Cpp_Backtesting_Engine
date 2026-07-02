#pragma once
#include "Event.h"
#include "EventQueue.h"
#include "DataHandler.h"

class ExecutionHandler {
public:
    virtual ~ExecutionHandler() = default;
    virtual void executeOrder(const OrderEvent& order, EventQueue& queue) = 0;
};

// NOTE on realism / lookahead: this simplified version fills at the *same*
// bar's adjusted close (plus slippage), i.e. it assumes the signal computed
// from bar t's close is executable at bar t's close. That is a common
// simplification for daily-bar backtests but is technically a small amount
// of lookahead. For a stricter no-lookahead engine, change this to fill at
// the *next* bar's open instead - that's the natural "v2" extension.
class SimulatedExecutionHandler : public ExecutionHandler {
public:
    SimulatedExecutionHandler(DataHandler& dataHandler,
                               double slippagePct = 0.0005,   // 5 bps
                               double commissionPerTrade = 1.0); // flat fee per trade

    void executeOrder(const OrderEvent& order, EventQueue& queue) override;

private:
    DataHandler& dataHandler_;
    double slippagePct_;
    double commissionPerTrade_;
};
