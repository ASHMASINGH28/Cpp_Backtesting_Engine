#pragma once
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <set>
#include "Bar.h"
#include "EventQueue.h"

// Abstract interface so the engine could later swap CSV files for a live feed
// without touching Strategy/Portfolio/ExecutionHandler code.
class DataHandler {
public:
    virtual ~DataHandler() = default;

    // Advances the "clock" by one date across all symbols and pushes a
    // MarketEvent for every symbol that has a bar on that date.
    // Returns false once there is no more data to process.
    virtual bool updateBars(EventQueue& queue) = 0;

    // Returns up to the last n bars seen so far for a symbol (never future bars).
    virtual std::vector<Bar> getLatestBars(const std::string& symbol, int n) = 0;

    virtual std::string getCurrentDate() const = 0;
    virtual std::vector<std::string> getSymbols() const = 0;
};

class CSVDataHandler : public DataHandler {
public:
    // Expects files named "<SYMBOL>.csv" inside dataDir, with a header row:
    // Date,Open,High,Low,Close,Adj Close,Volume   (Yahoo Finance / yfinance format)
    CSVDataHandler(const std::vector<std::string>& symbols, const std::string& dataDir);

    bool updateBars(EventQueue& queue) override;
    std::vector<Bar> getLatestBars(const std::string& symbol, int n) override;
    std::string getCurrentDate() const override { return currentDate_; }
    std::vector<std::string> getSymbols() const override { return symbols_; }

private:
    std::vector<std::string> symbols_;
    std::string dataDir_;

    std::map<std::string, std::vector<Bar>> symbolData_;   // full history, sorted by date
    std::map<std::string, size_t> nextIndex_;               // next unseen bar per symbol
    std::map<std::string, std::deque<Bar>> latestBars_;     // bars revealed so far

    std::vector<std::string> allDates_;                     // sorted union of all dates
    size_t dateCursor_ = 0;
    std::string currentDate_;

    void loadCSV(const std::string& symbol);
};
