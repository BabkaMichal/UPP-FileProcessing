#include "helpers.h"

void accumulateStationData(const std::vector<Measurement>& measurements, YearAccumulator& year_acc, MonthAccumulator& month_acc, double& global_min, double& global_max) {
    for (const auto& m : measurements) {
        year_acc[m.month][m.year].first += m.value;
        year_acc[m.month][m.year].second += 1;

        month_acc[m.month].first += m.value;
        month_acc[m.month].second += 1;

        //global extremes
        if (m.value < global_min) global_min = m.value;
        if (m.value > global_max) global_max = m.value;
    }
}

void computeStationAverages(const YearAccumulator& year_acc, const MonthAccumulator& month_acc, StationStats& st_stats) {
    for (int month = 1; month <= 12; ++month) {
        //no data for this month, skip
        if (month_acc.find(month) == month_acc.end()) continue;

        MonthlyStats& m_stats = st_stats.months[month];

        //longterm average
        m_stats.overall_average = month_acc.at(month).first / month_acc.at(month).second;

        // yearly average
        if (year_acc.find(month) != year_acc.end()) {
            for (const auto& year_pair : year_acc.at(month)) {
                int year = year_pair.first;
                double sum = year_pair.second.first;
                int count = year_pair.second.second;

                double avg = sum / count;
                m_stats.yearly_averages[year] = avg;

                if (avg < m_stats.min_yearly_avg) m_stats.min_yearly_avg = avg;
                if (avg > m_stats.max_yearly_avg) m_stats.max_yearly_avg = avg;
            }
        }
    }
}

void processSingleStation(const std::vector<Measurement>& measurements, StationStats& st_stats, double& global_min, double& global_max) {
    //helper maps
    YearAccumulator year_acc;
    MonthAccumulator month_acc;

    //get sums
    accumulateStationData(measurements, year_acc, month_acc, global_min, global_max);

    //calculate averages
    computeStationAverages(year_acc, month_acc, st_stats);
}

void calculateAverages(const std::unordered_map<int, std::vector<Measurement>>& stations, std::unordered_map<int, StationStats>& processed_stations, double& global_min, double& global_max) {
    global_min = 9999.0;
    global_max = -9999.0;

    for (const auto& pair : stations) {
        int station_id = pair.first;
        const auto& measurements = pair.second;

        processSingleStation(measurements, processed_stations[station_id], global_min, global_max);
    }

    std::cout << "Averages calculated! " << std::endl;
}