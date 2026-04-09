#include "helpers.h"

void runSerial(const char* stanicePath, const char* mereniPath) {
    //creating maps for preprocessed data
    std::unordered_map<int, std::vector<Measurement>> stations;
    std::unordered_map<int, StationLocation> stations_location;

    //loading the file
    loadCsv(stanicePath, stations);

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
    stations_location = loadStationsCSV(mereniPath);

    //drawing the values into svg
    generateMonthSVGs(stations_location, processed_stations, global_min, global_max);
}

void runParallel(const char* stanicePath, const char* mereniPath) {

}

int main(int argc, const char* argv[]) {

    if (argc < 4) {
        std::cout << "Missing arguments! \n" << "Use format program_name stations.csv data.csv --serial/parallel" << std::endl;
        return 1;
    }

    const char* path_stanice = argv[2];
    const char* path_mereni = argv[1];

    std::string_view mode = argv[3];

    auto start_time = std::chrono::high_resolution_clock::now();

    if (mode == "--serial") {
        std::cout << "Running serial version." << std::endl;
        runSerial(path_stanice, path_mereni);
    }
    else if (mode == "--parallel") {
        std::cout << "Running paralell version." << std::endl;
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