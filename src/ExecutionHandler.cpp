#include "ExecutionHandler.h"

SimulatedExecutionHandler::SimulatedExecutionHandler(DataHandler& dataHandler,
                                                       double slippagePct,
                                                       double commissionPerTrade)
    : dataHandler_(dataHandler), slippagePct_(slippagePct), commissionPerTrade_(commissionPerTrade) {}

void SimulatedExecutionHandler::executeOrder(const OrderEvent& order, EventQueue& queue) {
    auto bars = dataHandler_.getLatestBars(order.symbol, 1);
    if (bars.empty()) return;
    const Bar& bar = bars.back();

    double fillPrice = (order.direction == Direction::BUY)
        ? bar.adjClose * (1.0 + slippagePct_)
        : bar.adjClose * (1.0 - slippagePct_);

    queue.push(std::make_shared<FillEvent>(
        order.symbol, bar.date, order.quantity, order.direction, fillPrice, commissionPerTrade_));
}
