#include "helpers.h"

static void runSerial(const char* stanicePath, const char* mereniPath) {
    //creating maps for preprocessed data
    std::unordered_map<int, std::vector<Measurement>> stations;
    std::unordered_map<int, StationLocation> stations_location;

    //loading the file
    loadCsv(mereniPath, stations);

    //filtering the data
    filterStations(stations);

    //structs for data
    std::unordered_map<int, StationStats> processed_stations;
    double global_min = 0.0;
    double global_max = 0.0;

    //calculating averages for other calculations
    calculateAverages(stations, processed_stations, global_min, global_max);

    //detecting fluctuations
    detectFluctuations(processed_stations);

    //loading locations of stations
    stations_location = loadStationsCSV(stanicePath);

    //drawing the values into svg
    generateMonthSVGs(stations_location, processed_stations, global_min, global_max);
}

static void runParallel(const char* stanicePath, const char* mereniPath) {
    //creating maps for preprocessed data
    std::unordered_map<int, std::vector<Measurement>> stations;
    std::unordered_map<int, StationLocation> stations_location;

    //loading stations and data at the same time
    std::thread t_load_mereni([&]() {
        loadCsv(mereniPath, stations);
        });
    std::thread t_load_stanice([&]() {
        stations_location = loadStationsCSV(stanicePath);
        });

    //waitting to finish
    t_load_mereni.join();
    t_load_stanice.join();

    //preparing variables for processing
    std::unordered_map<int, StationStats> processed_stations;
    double global_min = 9999.0;
    double global_max = -9999.0;

    //calculating averages multithreaded
    calculateAveragesParallel(stations, processed_stations, global_min, global_max);

    //drawing svg and calculating fluctuations at the same time
    std::thread t_fluctuations([&]() {
        detectFluctuations(processed_stations);
        });

    std::thread t_svgs([&]() {
        generateMonthSVGs(stations_location, processed_stations, global_min, global_max);
        });

    //waiting to finish
    t_fluctuations.join();
    t_svgs.join();
}

int main(int argc, const char* argv[]) {

    if (argc < 4) {
        std::cout << "Missing arguments! \n" << "Use format program_name stations.csv data.csv --serial/parallel" << std::endl;
        return 1;
    }

    const char* path_stanice = argv[1];
    const char* path_mereni = argv[2];
    std::string_view mode = argv[3];

    auto start_time = std::chrono::high_resolution_clock::now();

    if (mode == "--serial") {
        std::cout << "Running serial version." << std::endl;
        runSerial(path_stanice, path_mereni);
    }
    else if (mode == "--parallel") {
        std::cout << "Running parallel version." << std::endl;
        runParallel(path_stanice, path_mereni);
    }
    else {
        std::cout << "Wrong arguments! Use --parallel or --serial" << std::endl;
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Time to finish: " << duration.count() << " ms." << std::endl;

	return 0;
}