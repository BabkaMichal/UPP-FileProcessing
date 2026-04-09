#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <cstdint>
#include <fstream>
#include <string>
#include <map>
#include <string_view>
#include <chrono>
#include <iomanip>

//constants
const double AVERAGE_FILTER = 100;
const int YEARS_IN_ROW_FILTER = 5;
const double LAT_MAX = 51.03806105663445;
const double LON_MIN = 12.102209054269062;
const double LAT_MIN = 48.521003814763994;
const double LON_MAX = 18.866923511078615;
const double SVG_WIDTH = 1412.0;
const double SVG_HEIGHT = 809.0;

//structs
struct Measurement
{
    //csv structure
    int station_id;
    int ordinal;
    int16_t year;
    int8_t month;
    int8_t day;
    float value;
};
struct MonthlyStats {
    //year - average
    std::map<int, double> yearly_averages;

    //average of averages - for svg map
    double overall_average = 0.0;

    //for fluctuations
    double min_yearly_avg = 9999.0;
    double max_yearly_avg = -9999.0;
};
struct StationStats {
    //saving the station with its monthly stats
    std::unordered_map<int, MonthlyStats> months;
};
struct StationLocation {
    int id;
    std::string name;
    double latitude;
    double longitude;
};

//aliases
using YearAccumulator = std::unordered_map<int, std::map<int, std::pair<double, int>>>;
using MonthAccumulator = std::unordered_map<int, std::pair<double, int>>;

//averages
void accumulateStationData(const std::vector<Measurement>& measurements, YearAccumulator& year_acc, MonthAccumulator& month_acc, double& global_min, double& global_max);
void computeStationAverages(const YearAccumulator& year_acc, const MonthAccumulator& month_acc, StationStats& st_stats);
void processSingleStation(const std::vector<Measurement>& measurements, StationStats& st_stats, double& global_min, double& global_max);
void calculateAverages(const std::unordered_map<int, std::vector<Measurement>>& stations, std::unordered_map<int, StationStats>& processed_stations, double& global_min, double& global_max);

//fluctuations
void detectFluctuations(const std::unordered_map<int, StationStats>& processed_stations);

//preprocessing
void loadCsv(const std::string& filename, std::unordered_map<int, std::vector<Measurement>>& stations);
void filterStations(std::unordered_map<int, std::vector<Measurement>>& stations);

//svg
std::unordered_map<int, StationLocation> loadStationsCSV(const std::string& filename);
std::string getColorForTemperature(double temp, double global_min, double global_max);
void generateMonthSVGs(const std::unordered_map<int, StationLocation>& locations, const std::unordered_map<int, StationStats>& stats, double global_min, double global_max);
