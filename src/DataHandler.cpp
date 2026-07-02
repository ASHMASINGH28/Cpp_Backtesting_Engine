#include "DataHandler.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>

static std::vector<std::string> splitCSVLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) tokens.push_back(token);
    return tokens;
}

CSVDataHandler::CSVDataHandler(const std::vector<std::string>& symbols, const std::string& dataDir)
    : symbols_(symbols), dataDir_(dataDir) {
    for (const auto& sym : symbols_) {
        loadCSV(sym);
        nextIndex_[sym] = 0;
    }

    // Build sorted union of all dates across symbols so multi-symbol runs
    // stay aligned on a shared calendar (handles holidays / missing rows).
    std::set<std::string> dateSet;
    for (const auto& [sym, bars] : symbolData_) {
        for (const auto& b : bars) dateSet.insert(b.date);
    }
    allDates_.assign(dateSet.begin(), dateSet.end());
    std::sort(allDates_.begin(), allDates_.end());
}

void CSVDataHandler::loadCSV(const std::string& symbol) {
    std::string path = dataDir_ + "/" + symbol + ".csv";
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open data file: " + path);
    }

    std::string line;
    std::getline(file, line); // header row - discarded

    std::vector<Bar> bars;
    size_t skipped = 0;

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        auto tokens = splitCSVLine(line);
        // Expect: Date,Open,High,Low,Close,Adj Close,Volume  (7 cols)
        // Fall back gracefully if Adj Close column is missing (6 cols).
        if (tokens.size() < 6) { ++skipped; continue; }

        try {
            Bar bar;
            bar.date  = tokens[0];
            bar.open  = std::stod(tokens[1]);
            bar.high  = std::stod(tokens[2]);
            bar.low   = std::stod(tokens[3]);
            bar.close = std::stod(tokens[4]);
            if (tokens.size() >= 7) {
                bar.adjClose = std::stod(tokens[5]);
                bar.volume   = std::stoll(tokens[6]);
            } else {
                bar.adjClose = bar.close;
                bar.volume   = std::stoll(tokens[5]);
            }
            bars.push_back(bar);
        } catch (const std::exception&) {
            // Malformed row (e.g. "null" values on a data-vendor outage day) - skip it
            // rather than crashing or silently corrupting the series.
            ++skipped;
        }
    }

    std::sort(bars.begin(), bars.end(),
              [](const Bar& a, const Bar& b) { return a.date < b.date; });

    if (skipped > 0) {
        std::cerr << "[CSVDataHandler] " << symbol << ": skipped " << skipped
                  << " malformed row(s)\n";
    }

    symbolData_[symbol] = std::move(bars);
}

bool CSVDataHandler::updateBars(EventQueue& queue) {
    if (dateCursor_ >= allDates_.size()) return false;

    currentDate_ = allDates_[dateCursor_];
    ++dateCursor_;

    bool anyBarPushed = false;
    for (const auto& sym : symbols_) {
        auto& idx = nextIndex_[sym];
        auto& data = symbolData_[sym];
        if (idx < data.size() && data[idx].date == currentDate_) {
            latestBars_[sym].push_back(data[idx]);
            ++idx;
            queue.push(std::make_shared<MarketEvent>(sym, currentDate_));
            anyBarPushed = true;
        }
    }
    // Even if this particular date had no bar for any symbol (shouldn't
    // happen since allDates_ is built from the data itself), we still
    // advanced the clock, so return true as long as data remains.
    return anyBarPushed || dateCursor_ < allDates_.size();
}

std::vector<Bar> CSVDataHandler::getLatestBars(const std::string& symbol, int n) {
    auto it = latestBars_.find(symbol);
    if (it == latestBars_.end() || it->second.empty()) return {};

    const auto& dq = it->second;
    int count = std::min<int>(n, static_cast<int>(dq.size()));
    return std::vector<Bar>(dq.end() - count, dq.end());
}
