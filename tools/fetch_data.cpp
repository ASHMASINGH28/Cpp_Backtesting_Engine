// Optional: fetches real historical daily OHLCV data from Stooq and saves it
// in the same CSV layout the engine expects. Requires libcurl:
//   sudo apt install libcurl4-openssl-dev   (Linux)
//   brew install curl                       (Mac)
// Build:
//   g++ -std=c++17 tools/fetch_data.cpp -o fetch_data -lcurl
// Run:
//   ./fetch_data AAPL data
//
// NOTE: Stooq's CSV columns are Date,Open,High,Low,Close,Volume (no Adj Close).
// CSVDataHandler already falls back to using Close as Adj Close when only 6
// columns are present, so no extra conversion is needed.
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* out) {
    size_t total = size * nmemb;
    out->append(static_cast<char*>(contents), total);
    return total;
}

static std::string fetchCSV(const std::string& tickerLower) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (!curl) return response;

    std::string url = "https://stooq.com/q/d/l/?s=" + tickerLower + ".us&i=d";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
    }
    curl_easy_cleanup(curl);
    return response;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TICKER> [outDir]\n";
        return 1;
    }
    std::string ticker = argv[1];
    std::string outDir = (argc > 2) ? argv[2] : "data";

    std::string tickerLower = ticker;
    std::transform(tickerLower.begin(), tickerLower.end(), tickerLower.begin(), ::tolower);

    std::string csv = fetchCSV(tickerLower);
    if (csv.empty() || csv.find("Date") == std::string::npos) {
        std::cerr << "Failed to fetch data for " << ticker << " (check ticker/network)\n";
        return 1;
    }

    std::string path = outDir + "/" + ticker + ".csv";
    std::ofstream out(path);
    out << csv;
    std::cout << "Saved " << path << "\n";
    return 0;
}
