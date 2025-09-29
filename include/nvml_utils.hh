#ifndef NVML_UTILS_HH
#define NVML_UTILS_HH

#include <time.h>
#include <memory>
#include <nvml.h>

#define MAX_GPUS 4

namespace nvml_utils
{
    struct EnergyAux
    {
        // Timestamp when this struct was last updated
        struct timespec time;
        // Last measured energy per CUDA GPU
        // We need a counter for each GPU node in the system, this assumes
        // a maximum of 4 GPUs, but may be increased or decreased as needed
        float energy[MAX_GPUS];
    };

    // Stores the average power consumption and energy consumption during the last
    // measurement interval, and total energy consumption. Sum of all GPUs
    struct EnergyData
    {
        double power{0};
        double energy{0};
        double total_energy{0};
    };

    extern std::unique_ptr<nvmlDevice_t[]> device_handles;
    extern unsigned int num_GPUs;

    /*
    Initialize the number of GPUs in the machine and get their nvml handles
    */
    void init();

    /*
    Updates the input EnergyAux struct with the last per-gpu energy readings in Joules
    */
    void update_gpu_energy(EnergyAux &data);

    /*
    Uses the measurements from two EnergyAux structs to update the provided EnergyData struct. Sets the computed power
    usage and energy consumption, and updates the total energy consumption measured in this EnergyData struct
    */
    void update_energy_data(EnergyData &output_data, const EnergyAux &previous_data, const EnergyAux &current_data);
}

#endif