#include "power_meter.hh"
#include "rapl_utils.hh"
#include "nvml_utils.hh"
#include "msr_reader.hh"

#include <nvml.h>
#include <thread>
#include <chrono>
#include <algorithm>

// Initialize global variables
namespace power_meter
{
    bool do_monitoring{true};
    std::thread monitoring_thread;
    std::filesystem::path output_dir{"power_meter_out"};
    std::filesystem::path cpu_out_filename{"cpu"};
    std::filesystem::path gpu_out_filename{"gpu"};
    std::ofstream cpu_out;
    std::ofstream gpu_out;
}

void power_meter::launch_monitoring_loop(unsigned int sampling_interval_ms)
{   
    // Check whether we have access to the MSR files
    rapl_utils::open_msr(0);
    // Intel: Initialize internal counters
    if (rapl_utils::init() != 0)
    {
        fprintf(stderr, "POWER METER: An error was encountered during initialization\n");
        return;
    }
    // Open output files
    std::filesystem::create_directory(output_dir);
    cpu_out.open(output_dir / cpu_out_filename);
    gpu_out.open(output_dir / gpu_out_filename);

    // CUDA: Start nvml
    nvmlInit_v2();
    // Intel: Initialize number of GPUs and device handles
    nvml_utils::init();
    // Launch monitoring on a separate thread
    do_monitoring = true;
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
    // Structs used to take measurements from Intel/AMD's RAPL interface
    rapl_utils::EnergyAux cpu_pkg_data;
    rapl_utils::EnergyAux current_cpu_pkg_data;
    rapl_utils::EnergyData cpu_pkg_results;
    // Struct used to take measurements from Nvidia NVML
    nvml_utils::EnergyAux cuda_data;
    nvml_utils::EnergyAux current_cuda_data;
    nvml_utils::EnergyData cuda_results;

    // Get the initial energy readings
    // CPU: Get the current energy measurement for RAPL's package domain
    rapl_utils::update_package_energy(cpu_pkg_data);
    // CUDA
    nvml_utils::update_gpu_energy(cuda_data);

    // Write the header for the output files
    auto output_header = "Power, Energy, Total energy";
    cpu_out << output_header << std::endl;
    gpu_out << output_header << std::endl;

    while (do_monitoring)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sampling_interval_ms));
        // CPU: Update energy measurements
        rapl_utils::update_package_energy(current_cpu_pkg_data);
        // CPU: Compute energy and average power usage for this interval, update total energy consumption
        rapl_utils::update_energy_data(cpu_pkg_results, cpu_pkg_data, current_cpu_pkg_data);
        // CUDA: Update energy measurements
        nvml_utils::update_gpu_energy(current_cuda_data);
        // CUDA: Compute energy and average power usage for this interval, update total energy consumption
        nvml_utils::update_energy_data(cuda_results, cuda_data, current_cuda_data);

        // Swap structs for the next iteration
        std::swap(cpu_pkg_data, current_cpu_pkg_data);
        std::swap(cuda_data, current_cuda_data);

        cpu_out << cpu_pkg_results.power << "," << cpu_pkg_results.energy << "," << cpu_pkg_results.total_energy << std::endl;
        gpu_out << cuda_results.power << "," << cuda_results.energy << "," << cuda_results.total_energy << std::endl;
    }
}

void power_meter::set_output_dir(std::string dir)
{
    output_dir = dir;
}

void power_meter::set_cpu_out_filename(std::string filename)
{
    cpu_out_filename = filename;
}

void power_meter::set_gpu_out_filename(std::string filename)
{
    gpu_out_filename = filename;
}

