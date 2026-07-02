#pragma once
#include <string>

// Represents a single OHLCV bar (e.g. one trading day).
struct Bar {
    std::string date;   // ISO format YYYY-MM-DD so string comparison == chronological order
    double open  = 0.0;
    double high  = 0.0;
    double low   = 0.0;
    double close = 0.0;
    double adjClose = 0.0; // split/dividend adjusted close - use this for signals & PnL
    long long volume = 0;
};
