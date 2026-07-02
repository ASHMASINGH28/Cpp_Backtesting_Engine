#include <iostream>
#include <fstream>
#include <memory>
#include <functional>
#include "DataHandler.h"
#include "Strategy.h"
#include "Portfolio.h"
#include "ExecutionHandler.h"
#include "EventQueue.h"
#include "PerformanceMetrics.h"

// Runs one full backtest end-to-end using the given strategy factory,
// prints a performance report, and dumps the equity curve to CSV.
static void runBacktest(const std::string& name,
                         const std::vector<std::string>& symbols,
                         const std::string& dataDir,
                         const std::function<std::unique_ptr<Strategy>(DataHandler&)>& strategyFactory,
                         double initialCapital) {
    CSVDataHandler dataHandler(symbols, dataDir);
    auto strategy = strategyFactory(dataHandler);
    Portfolio portfolio(dataHandler, initialCapital);
    SimulatedExecutionHandler execHandler(dataHandler);
    EventQueue queue;

    // ---- The event-driven main loop ----
    // One iteration = one calendar date. We fully drain the event queue
    // (market -> signal -> order -> fill) before marking-to-market and
    // moving to the next date, so nothing "sees the future".
    while (dataHandler.updateBars(queue)) {
        while (!queue.empty()) {
            EventPtr event = queue.pop();
            switch (event->type) {
                case EventType::MARKET: {
                    auto e = std::static_pointer_cast<MarketEvent>(event);
                    strategy->calculateSignals(*e, queue);
                    break;
                }
                case EventType::SIGNAL: {
                    auto e = std::static_pointer_cast<SignalEvent>(event);
                    portfolio.updateSignal(*e, queue);
                    break;
                }
                case EventType::ORDER: {
                    auto e = std::static_pointer_cast<OrderEvent>(event);
                    execHandler.executeOrder(*e, queue);
                    break;
                }
                case EventType::FILL: {
                    auto e = std::static_pointer_cast<FillEvent>(event);
                    portfolio.updateFill(*e);
                    break;
                }
            }
        }
        portfolio.updateTimeIndex(dataHandler.getCurrentDate());
    }

    PerformanceMetrics::printReport(portfolio.getEquityCurve(), name);

    std::string outPath = "output_" + name + "_equity.csv";
    std::ofstream out(outPath);
    out << "Date,Equity\n";
    for (const auto& p : portfolio.getEquityCurve()) {
        out << p.date << "," << p.equity << "\n";
    }
    std::cout << "Equity curve written to " << outPath << "\n";
}

int main(int argc, char** argv) {
    std::string dataDir = "data";
    std::vector<std::string> symbols = {"AAPL"};
    double initialCapital = 100000.0;

    if (argc > 1) dataDir = argv[1];
    if (argc > 2) symbols = {argv[2]};
    if (argc > 3) initialCapital = std::stod(argv[3]);

    std::cout << "Running backtests on symbol(s): ";
    for (auto& s : symbols) std::cout << s << " ";
    std::cout << "| data dir: " << dataDir << " | initial capital: " << initialCapital << "\n";

    runBacktest("SMA_Crossover", symbols, dataDir,
        [](DataHandler& dh) { return std::make_unique<SMACrossoverStrategy>(dh, 20, 50); },
        initialCapital);

    runBacktest("Buy_And_Hold", symbols, dataDir,
        [](DataHandler& dh) { return std::make_unique<BuyAndHoldStrategy>(dh); },
        initialCapital);

    return 0;
}
