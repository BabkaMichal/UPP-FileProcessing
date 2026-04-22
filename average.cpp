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

void calculateAveragesParallel(const std::unordered_map<int, std::vector<Measurement>>& stations, std::unordered_map<int, StationStats>& processed_stations, double& global_min, double& global_max) {
    std::vector<int> station_ids;
    station_ids.reserve(stations.size());

    for (const auto& pair : stations) {
        station_ids.push_back(pair.first);
    }

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    std::mutex mtx;
    int chunk_size = station_ids.size() / num_threads;
    std::vector<std::thread> threads;

    auto worker_process = [&](int start_idx, int end_idx) {
        std::unordered_map<int, StationStats> local_processed;
        double local_min = 9999.0;
        double local_max = -9999.0;

        for (int i = start_idx; i < end_idx; ++i) {
            int id = station_ids[i];
            const auto& measurements = stations.at(id);

            StationStats st_stats;
            processSingleStation(measurements, st_stats, local_min, local_max);
            local_processed[id] = st_stats;
        }

        std::lock_guard<std::mutex> lock(mtx);
        if (local_min < global_min) global_min = local_min;
        if (local_max > global_max) global_max = local_max;

        for (const auto& pair : local_processed) {
            processed_stations[pair.first] = pair.second;
        }
        };

    for (unsigned int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? station_ids.size() : start + chunk_size;
        threads.emplace_back(worker_process, start, end);
    }

    for (auto& t : threads) t.join();
}