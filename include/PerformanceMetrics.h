#pragma once
#include <vector>
#include <string>
#include "Portfolio.h"

class PerformanceMetrics {
public:
    static std::vector<double> dailyReturns(const std::vector<EquityPoint>& equity);

    static double totalReturnPct(const std::vector<EquityPoint>& equity);
    static double cagrPct(const std::vector<EquityPoint>& equity, double tradingDaysPerYear = 252.0);
    static double sharpeRatio(const std::vector<EquityPoint>& equity, double riskFreeRateAnnual = 0.0);
    static double maxDrawdownPct(const std::vector<EquityPoint>& equity);

    static void printReport(const std::vector<EquityPoint>& equity, const std::string& strategyName);
};
