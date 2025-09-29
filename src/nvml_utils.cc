#include "nvml_utils.hh"

#include <time.h>
#include <nvml.h>

void nvml_utils::update_gpu_energy(EnergyAux &data)
{
    // TODO: Implement this for a variable number of GPUs in the machine
    unsigned long long energy{0};
    nvmlDeviceGetTotalEnergyConsumption(0, &energy);
    // Value returned by NVML is in mili Joules
    data.energy[0] = (float)energy / 1E3;
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