#include "PerformanceMetrics.h"
#include <cmath>
#include <numeric>
#include <iostream>
#include <iomanip>

std::vector<double> PerformanceMetrics::dailyReturns(const std::vector<EquityPoint>& equity) {
    std::vector<double> returns;
    for (size_t i = 1; i < equity.size(); ++i) {
        double prev = equity[i - 1].equity;
        if (prev > 0.0) returns.push_back((equity[i].equity - prev) / prev);
    }
    return returns;
}

double PerformanceMetrics::totalReturnPct(const std::vector<EquityPoint>& equity) {
    if (equity.size() < 2 || equity.front().equity <= 0.0) return 0.0;
    return (equity.back().equity - equity.front().equity) / equity.front().equity * 100.0;
}

double PerformanceMetrics::cagrPct(const std::vector<EquityPoint>& equity, double tradingDaysPerYear) {
    if (equity.size() < 2 || equity.front().equity <= 0.0) return 0.0;
    double totalGrowth = equity.back().equity / equity.front().equity;
    double years = static_cast<double>(equity.size()) / tradingDaysPerYear;
    if (years <= 0.0) return 0.0;
    return (std::pow(totalGrowth, 1.0 / years) - 1.0) * 100.0;
}

double PerformanceMetrics::sharpeRatio(const std::vector<EquityPoint>& equity, double riskFreeRateAnnual) {
    auto returns = dailyReturns(equity);
    if (returns.size() < 2) return 0.0;

    double dailyRf = riskFreeRateAnnual / 252.0;
    double mean = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
    double excessMean = mean - dailyRf;

    double variance = 0.0;
    for (double r : returns) variance += (r - mean) * (r - mean);
    variance /= (returns.size() - 1);
    double stddev = std::sqrt(variance);

    if (stddev == 0.0) return 0.0;
    return (excessMean / stddev) * std::sqrt(252.0);
}

double PerformanceMetrics::maxDrawdownPct(const std::vector<EquityPoint>& equity) {
    double peak = -1e18;
    double maxDD = 0.0;
    for (const auto& p : equity) {
        peak = std::max(peak, p.equity);
        if (peak > 0.0) {
            double dd = (peak - p.equity) / peak;
            maxDD = std::max(maxDD, dd);
        }
    }
    return maxDD * 100.0;
}

void PerformanceMetrics::printReport(const std::vector<EquityPoint>& equity, const std::string& strategyName) {
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n===== " << strategyName << " =====\n";
    if (equity.empty()) {
        std::cout << "No equity data (check that your data files loaded correctly).\n";
        return;
    }
    std::cout << "Start Equity : " << equity.front().equity << "\n";
    std::cout << "End Equity   : " << equity.back().equity << "\n";
    std::cout << "Total Return : " << totalReturnPct(equity) << "%\n";
    std::cout << "CAGR         : " << cagrPct(equity) << "%\n";
    std::cout << "Sharpe Ratio : " << sharpeRatio(equity) << "\n";
    std::cout << "Max Drawdown : " << maxDrawdownPct(equity) << "%\n";
}
