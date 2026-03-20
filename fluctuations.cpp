#include "helpers.h"
#include <fstream>
#include <cmath>
#include <iostream>


void detectFluctuations(const std::unordered_map<int, StationStats>& processed_stations) {
    //opening the file
    std::ofstream outfile("vykyvy.csv");
    if (!outfile.is_open()) {
        std::cerr << "Chyba: Nelze vytvorit soubor vykyvy.csv pro zapis!" << std::endl;
        return;
    }

    //writing the header
    outfile << "ID stanice;mesic detekce;druhy z roku detekce;hodnota rozdilu teplot\n";

    //iterating through stations
    for (const auto& pair : processed_stations) {
        int station_id = pair.first;
        const StationStats& st_stats = pair.second;

        //iterating every month
        for (const auto& month_pair : st_stats.months) {
            int month = month_pair.first;
            const MonthlyStats& m_stats = month_pair.second;

            //less that 2 months, cant happen
            if (m_stats.yearly_averages.size() < 2) continue;

            double threshold = 0.75 * (m_stats.max_yearly_avg - m_stats.min_yearly_avg);

            auto it = m_stats.yearly_averages.begin();
            int prev_year = it->first;
            double prev_avg = it->second;
            ++it;

            for (; it != m_stats.yearly_averages.end(); ++it) {
                int curr_year = it->first;
                double curr_avg = it->second;

                //years in row
                if (curr_year == prev_year + 1) {

                    //difference > treshhold, we save
                    double diff = std::abs(curr_avg - prev_avg);

                    if (diff > threshold) {
                        outfile << station_id << ";"
                            << month << ";"
                            << curr_year << ";"
                            << diff << "\n";
                    }
                }

                prev_year = curr_year;
                prev_avg = curr_avg;
            }
        }
    }

    //close the file
    outfile.close();

    std::cout << "Fluctuations detected! \n";
}