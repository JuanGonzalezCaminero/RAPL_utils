#include "nvml_utils.hh"

#include <cstdio>
#include <time.h>

// Global variable definitions
namespace nvml_utils
{
    unsigned int num_GPUs{0};
    std::unique_ptr<nvmlDevice_t[]> device_handles;
}

void nvml_utils::init()
{
    nvmlDeviceGetCount_v2(&num_GPUs);
    device_handles = std::make_unique<nvmlDevice_t[]>(num_GPUs);
    for (unsigned int i = 0; i < num_GPUs; ++i)
    {
        nvmlDeviceGetHandleByIndex_v2(i, &device_handles[i]);
    }
    printf("POWER METER: Number of GPUs detected: %d\n", num_GPUs);
}

void nvml_utils::update_gpu_energy(EnergyAux &data)
{
    unsigned long long energy{0};
    for (unsigned int i = 0; i < num_GPUs; ++i)
    {
        nvmlDeviceGetTotalEnergyConsumption(device_handles[i], &energy);
        // Value returned by NVML is in mili Joules
        data.energy[i] = (float)energy / 1E3;
    }
    // Update the timestamp
    clock_gettime(CLOCK_REALTIME, &data.time);
}

void nvml_utils::update_energy_data(EnergyData &output_data, const EnergyAux &previous_data, const EnergyAux &current_data)
{
    double time_diff =
        (double)(current_data.time.tv_sec - previous_data.time.tv_sec) +
        ((double)(current_data.time.tv_nsec - previous_data.time.tv_nsec) / 1E9);
    // TODO: Implement this for a variable number of GPUs in the machine
    auto energy_diff = current_data.energy[0] - previous_data.energy[0];
    output_data.power = (float)(energy_diff / time_diff);
    output_data.energy = energy_diff;
    output_data.total_energy += energy_diff;
}