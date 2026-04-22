#include "helpers.h"

void loadCsv(const std::string& filename, std::unordered_map<int, std::vector<Measurement>>& stations) {
    //opening the file for reading
    std::ifstream file(filename);

    //sanity check
    if (!file.is_open()) {
        std::cerr << "Error, cant open file " << filename << "!" << std::endl;
        return;
    }

    //variables
    std::string line;

    //skiping header
    std::getline(file, line);

    //reading until end of file
    while (std::getline(file, line)) {
        //skiping empty lines
        if (line.empty()) continue;

        //variables
        Measurement m;
        size_t pos = 0;
        size_t next_pos = 0;

        //station_id
        next_pos = line.find(';', pos);
        m.station_id = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //ordinal
        next_pos = line.find(';', pos);
        m.ordinal = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //year
        next_pos = line.find(';', pos);
        m.year = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //month
        next_pos = line.find(';', pos);
        m.month = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //day
        next_pos = line.find(';', pos);
        m.day = std::stoi(line.substr(pos, next_pos - pos));
        pos = next_pos + 1;

        //value 
        m.value = std::stof(line.substr(pos));

        //adding the struct into the map
        stations[m.station_id].push_back(m);
    }

    std::cout << "File loaded! " << std::endl;
}

//parsing one chunk
static void parseChunk(const std::string& buf, size_t start, size_t end, std::unordered_map<int, std::vector<Measurement>>& out) {
    size_t pos = start;
    while (pos < end) {
        Measurement m;
        size_t next_pos;

        // station_id
        next_pos = buf.find(';', pos);
        m.station_id = std::atoi(buf.data() + pos);
        pos = next_pos + 1;

        // ordinal
        next_pos = buf.find(';', pos);
        m.ordinal = std::atoi(buf.data() + pos);
        pos = next_pos + 1;

        // year
        next_pos = buf.find(';', pos);
        m.year = std::atoi(buf.data() + pos);
        pos = next_pos + 1;

        // month
        next_pos = buf.find(';', pos);
        m.month = std::atoi(buf.data() + pos);
        pos = next_pos + 1;

        // day
        next_pos = buf.find(';', pos);
        m.day = std::atoi(buf.data() + pos);
        pos = next_pos + 1;

        // value
        next_pos = buf.find('\n', pos);
        m.value = std::atof(buf.data() + pos);
        pos = next_pos + 1;

        out[m.station_id].push_back(m);
    }
}

void loadCsvParallel(const std::string& filename, std::unordered_map<int, std::vector<Measurement>>& stations) {
    std::cout << "Loading file" << std::endl;

    //find file size
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    size_t file_size = file.tellg();
    file.seekg(0);

    //skip header
    std::string header;
    std::getline(file, header);
    size_t data_start = file.tellg();

    //n chunks
    int num_threads = std::thread::hardware_concurrency();
    size_t chunk_size = (file_size - data_start) / num_threads;

    //load whole file
    std::string buffer(file_size - data_start, '\0');
    file.read(buffer.data(), buffer.size());

    //each thread one chunk
    std::vector<std::unordered_map<int, std::vector<Measurement>>> local_maps(num_threads);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; i++) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? buffer.size() : (i + 1) * chunk_size;

        if (i != 0) {
            while (start < buffer.size() && buffer[start - 1] != '\n') start++;
        }
        while (end < buffer.size() && buffer[end] != '\n') end++;

        threads.emplace_back([&, i, start, end]() {
            parseChunk(buffer, start, end, local_maps[i]);
            });
    }

    for (auto& t : threads) t.join();

    //join into one map
    for (auto& local : local_maps) {
        for (auto& [id, vec] : local) {
            auto& dest = stations[id];
            dest.insert(dest.end(), vec.begin(), vec.end());
        }
    }
}

void filterStations(std::unordered_map<int, std::vector<Measurement>>& stations) {
    //iterating through map
    for (auto it = stations.begin(); it != stations.end(); ) {
        const auto& measurements = it->second;

        //getting unique years due to set logic
        std::set<int> unique_years;
        for (const auto& m : measurements) {
            unique_years.insert(m.year);
        }

        //sanity check
        if (unique_years.empty()) {
            it = stations.erase(it);
            continue;
        }

        //calculating the average per year with all measurements / amount of years
        double avg_per_year = static_cast<double>(measurements.size()) / unique_years.size();

        //remove if less than 100
        if (avg_per_year < AVERAGE_FILTER) {
            it = stations.erase(it);
            continue;
        }

        bool has_n_consecutive = false;

        //if we have more than years in row we need, try to find if consequential
        if (unique_years.size() >= YEARS_IN_ROW_FILTER) {
            int streak = 1;
            auto year_it = unique_years.begin();
            int prev_year = *year_it;
            ++year_it;

            //previous = current + 1, pass
            for (; year_it != unique_years.end(); ++year_it) {
                if (*year_it == prev_year + 1) {
                    streak++;
                    if (streak >= YEARS_IN_ROW_FILTER) {
                        has_n_consecutive = true;
                        break;
                    }
                }
                //reset streak
                else {
                    streak = 1;
                }
                prev_year = *year_it;
            }
        }

        //if failed:remove, else ++iterator
        if (!has_n_consecutive) {
            it = stations.erase(it);
        }
        else {
            ++it;
        }
    }
    std::cout << "Stations filtered! " << std::endl;
}

bool isValidStation(const std::vector<Measurement>& measurements) {
    std::set<int> unique_years;
    for (const auto& m : measurements) {
        unique_years.insert(m.year);
    }
    if (unique_years.empty()) return false;

    double avg_per_year = static_cast<double>(measurements.size()) / unique_years.size();
    if (avg_per_year < AVERAGE_FILTER) return false;

    bool has_5_consecutive = false;
    if (unique_years.size() >= YEARS_IN_ROW_FILTER) {
        int streak = 1;
        auto year_it = unique_years.begin();
        int prev_year = *year_it;
        ++year_it;
        for (; year_it != unique_years.end(); ++year_it) {
            if (*year_it == prev_year + 1) {
                streak++;
                if (streak >= 5) {
                    has_5_consecutive = true;
                    break;
                }
            }
            else {
                streak = 1;
            }
            prev_year = *year_it;
        }
    }
    return has_5_consecutive;
}

void filterStationsParallel(std::unordered_map<int, std::vector<Measurement>>& stations) {
    // collect keys
    std::vector<int> ids;
    ids.reserve(stations.size());
    for (const auto& pair : stations)
        ids.push_back(pair.first);

    std::vector<int> to_remove;
    std::mutex mtx;

    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    int chunk_size = ids.size() / num_threads;

    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? ids.size() : start + chunk_size;

        threads.emplace_back([&, start, end]() {
            std::vector<int> local_remove;

            for (int j = start; j < end; ++j) {
                int id = ids[j];
                if (!isValidStation(stations.at(id)))
                    local_remove.push_back(id);
            }

            std::lock_guard<std::mutex> lock(mtx);
            to_remove.insert(to_remove.end(), local_remove.begin(), local_remove.end());
        });
    }

    for (auto& t : threads) t.join();

    for (int id : to_remove)
        stations.erase(id);

    std::cout << "Stations filtered (parallel)!" << std::endl;
}