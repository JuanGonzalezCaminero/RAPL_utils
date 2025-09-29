#include "power_meter.hh"
#include "energy_utils.hh"

#include <nvml.h>
#include <thread>
#include <chrono>
#include <algorithm>

// Initialize global variables
namespace power_meter
{
    bool do_monitoring{true};
    std::thread monitoring_thread;
}

void power_meter::launch_monitoring_loop(unsigned int sampling_interval_ms)
{
    do_monitoring = true;
    // Intel: Initialize internal counters
    rapl_utils::energy_init();
    // CUDA: Start nvml
    nvmlInit_v2();
    // Launch monitoring on a separate thread
    monitoring_thread = std::thread(monitoring_loop, sampling_interval_ms);
}

void power_meter::stop_monitoring_loop()
{
    // Stop monitoring thread
    do_monitoring = false;
    monitoring_thread.join();
    // CUDA: Stop nvml
    nvmlShutdown();
}

/*
Power measurement loop, intended to run on a separate thread
*/
void power_meter::monitoring_loop(unsigned int sampling_interval_ms)
{
    // Structs used to take measurements from Intel's RAPL interface
    rapl_utils::EnergyAux intel_pkg_data;
    rapl_utils::EnergyAux current_intel_pkg_data;
    rapl_utils::EnergyData intel_pkg_results;
    // Struct used to take measurements from Nvidia NVML

    // Get the initial energy readings
    // Intel: Get the current energy measurement for RAPL's package domain
    rapl_utils::update_package_energy(intel_pkg_data);
    while (do_monitoring)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sampling_interval_ms));
        // Intel: Update energy measurements
        rapl_utils::update_package_energy(current_intel_pkg_data);
        // Intel: Compute energy and average power usage for this interval, update total energy consumption
        rapl_utils::update_energy_data(intel_pkg_results, intel_pkg_data, current_intel_pkg_data);
        // Swap structs for the next iteration
        std::swap(intel_pkg_data, current_intel_pkg_data);

        printf("Power: %lf, Energy: %lf, Total energy: %lf\n", intel_pkg_results.power, intel_pkg_results.energy, intel_pkg_results.total_energy);
    }
}