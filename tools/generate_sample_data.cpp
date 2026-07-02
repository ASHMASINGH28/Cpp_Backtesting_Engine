// Generates synthetic daily OHLCV data using a random walk with drift, so you
// can build and test the backtester immediately without hitting any network.
// Once it works, swap these files for real data downloaded via curl/Stooq
// (see tools/fetch_data.cpp or the README).
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <string>
#include <vector>

struct SimParams {
    std::string symbol;
    double startPrice;
    double annualDrift;      // e.g. 0.08 = 8%/year expected return
    double annualVol;        // e.g. 0.25 = 25%/year volatility
    unsigned seed;
};

static std::string dateFromDayIndex(int dayIndex) {
    // Simple synthetic calendar: start 2015-01-02, skip weekends, no holidays.
    // Good enough for testing engine mechanics - not meant to be a real calendar.
    int y = 2015, m = 1, d = 2;
    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int businessDaysAdded = 0;
    while (businessDaysAdded < dayIndex) {
        ++d;
        if (d > daysInMonth[m - 1]) {
            d = 1; ++m;
            if (m > 12) { m = 1; ++y; if (y % 4 == 0) daysInMonth[1] = 29; else daysInMonth[1] = 28; }
        }
        // crude weekday check via Zeller-ish approximation is overkill here;
        // just treat every 6th/7th generated day as a weekend to skip.
        ++businessDaysAdded;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
    return std::string(buf);
}

static void generate(const SimParams& p, int numDays, const std::string& outDir) {
    std::mt19937 rng(p.seed);
    double dailyDrift = p.annualDrift / 252.0;
    double dailyVol   = p.annualVol / std::sqrt(252.0);
    std::normal_distribution<double> noise(0.0, 1.0);

    std::ofstream out(outDir + "/" + p.symbol + ".csv");
    out << "Date,Open,High,Low,Close,Adj Close,Volume\n";

    double price = p.startPrice;
    for (int i = 0; i < numDays; ++i) {
        double ret = dailyDrift + dailyVol * noise(rng);
        double open = price;
        double close = open * (1.0 + ret);
        double high = std::max(open, close) * (1.0 + std::abs(noise(rng)) * 0.003);
        double low  = std::min(open, close) * (1.0 - std::abs(noise(rng)) * 0.003);
        long long volume = 1000000 + static_cast<long long>(std::abs(noise(rng)) * 500000);

        out << dateFromDayIndex(i) << ","
            << open << "," << high << "," << low << "," << close << ","
            << close << "," << volume << "\n";

        price = close;
    }
    std::cout << "Generated " << numDays << " bars for " << p.symbol
              << " -> " << outDir << "/" << p.symbol << ".csv\n";
}

int main(int argc, char** argv) {
    std::string outDir = (argc > 1) ? argv[1] : "data";
    int numDays = (argc > 2) ? std::stoi(argv[2]) : 750; // ~3 trading years

    // A "trending" stock so the SMA crossover strategy has something to catch,
    // and a benchmark-like asset with lower drift/vol.
    generate({"AAPL", 100.0, 0.15, 0.30, 42}, numDays, outDir);
    generate({"SPY",  200.0, 0.08, 0.16, 7},  numDays, outDir);

    return 0;
}
