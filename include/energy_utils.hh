#ifndef ENERGY_UTILS_HH
#define ENERGY_UTILS_HH

#include "rapl_const.hh"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <memory>

#define MAX_NUMA_NODES 8
namespace rapl_utils
{
    //////////////////////////////////////////////////////////////////////
    //						  	   DATA
    //////////////////////////////////////////////////////////////////////

    // This struct contains per-node energy measurements along with the time they were taken
    struct EnergyAux
    {
        // Timestamp when this struct was last updated
        struct timespec time;
        // Last measured energy per NUMA node
        // We need a counter for each NUMA node in the system, this assumes
        // a maximum of 8 nodes, but may be increased or decreased as needed
        float energy[MAX_NUMA_NODES];
    };

    // Stores the machine's average power consumption and energy consumption during the last
    // measurement interval, and total energy consumption.
    struct EnergyData
    {
        double power{0};
        double energy{0};
        double total_energy{0};
    };

    extern unsigned long long
        INTEL_MSR_RAPL_POWER_UNIT_VALUES[INTEL_MSR_RAPL_POWER_UNIT_NUMFIELDS];
    extern unsigned long long
        INTEL_MSR_PKG_ENERGY_STATUS_VALUES[INTEL_MSR_PKG_ENERGY_STATUS_NUMFIELDS];
    extern unsigned long long
        INTEL_MSR_PP0_ENERGY_STATUS_VALUES[INTEL_MSR_PP0_ENERGY_STATUS_NUMFIELDS];
    extern unsigned long long
        INTEL_MSR_PKG_POWER_INFO_VALUES[INTEL_MSR_PKG_POWER_INFO_NUMFIELDS];

    /*
    Store the increment for each unit in this machine
    */
    extern float power_increment;
    extern float energy_increment;
    extern float time_increment;

    /*
    Store the value at which the energy counter wraps around
    */
    extern float energy_counter_max;

    /*
    Store NUMA-related information (Number of nodes, cores per node, id of
    the first core in each node)
    */
    extern int numa_nodes;

    extern std::unique_ptr<int[]> first_node_core;
    extern int numcores;

    //////////////////////////////////////////////////////////////////////
    //						  UTILITY FUNCTIONS
    //////////////////////////////////////////////////////////////////////

    /*
    Initializes the program, reading the energy units MSR and computing the
    increments for each unit in this machine
    */
    void energy_init();

    /*
    Returns the last energy reading of the specified RAPL domain in Joules
    The value returned is the sum of the energy consumed by the CPU in the specified
    NUMA node
    */
    float get_node_energy(int node, int domain);

    /*
    Updates the input EnergyAux struct with the last per-node energy readings of the specified RAPL domain in Joules
    */
    void update_aux_data(EnergyAux &data, int domain);

    /*
    Updates the input EnergyAux struct with the last per-node energy readings of RAPL's Package domain in Joules
    */
    void update_package_energy(EnergyAux &data);

    /*
    Updates the input EnergyAux struct with the last per-node energy readings of RAPL's Cores domain in Joules
    */
    void update_cores_energy(EnergyAux &data);

    /*
    Uses the measurements in the two provided EnergyAux structs to compute the average power consumed by the specified RAPL domain, in Watts.
    */
    float get_power(const EnergyAux &previous_data, const EnergyAux &current_data);

    /*
    Uses the measurements from two EnergyAux structs to update the provided EnergyData struct. Sets the computed power
    usage and energy consumption, and updates the total energy consumption measured in this EnergyData struct
    */
    void update_energy_data(EnergyData &output_data, const EnergyAux &previous_data, const EnergyAux &current_data);

    /*
    Receives two arrays, one with current energy measurements for each NUMA node in
    the system and another with old ones. Returns the energy consumed between both
    measurements.

    This function takes into account possible hardware counter wraparounds. For each
    NUMA node, the hardware counter that stores the energy consumed will reset back
    to 0 when it reaches its maximum value (2^32 per specification). Whenever this
    event happens between the taking of two measurements, it needs to be detected
    and corrected.
    */
    float get_energy_diff(const float *current_energy, const float *previous_energy);

    /*
    Returns the TDP of the CPU in Watts
    The value returned is the aggregate TDP of all the CPUs in the system
    */
    float get_processor_tdp();

    //////////////////////////////////////////////////////////////////////
    //						 READING MSR FIELDS
    //////////////////////////////////////////////////////////////////////

    /*
    Reads the MSR and stores its values in INTEL_MSR_RAPL_POWER_UNIT_VALUES
    */
    void read_INTEL_MSR_RAPL_POWER_UNIT(int core);

    /*
    Reads the MSR and stores its values in INTEL_MSR_PKG_ENERGY_STATUS
    */
    void read_INTEL_MSR_PKG_ENERGY_STATUS(int core);

    void read_INTEL_MSR_PP0_ENERGY_STATUS(int core);

    void read_INTEL_MSR_PKG_POWER_INFO(int core);

} // namespace rapl_utils

#endif