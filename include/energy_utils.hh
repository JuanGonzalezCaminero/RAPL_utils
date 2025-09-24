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

    unsigned long long
        INTEL_MSR_RAPL_POWER_UNIT_VALUES[INTEL_MSR_RAPL_POWER_UNIT_NUMFIELDS];
    unsigned long long
        INTEL_MSR_PKG_ENERGY_STATUS_VALUES[INTEL_MSR_PKG_ENERGY_STATUS_NUMFIELDS];
    unsigned long long
        INTEL_MSR_PP0_ENERGY_STATUS_VALUES[INTEL_MSR_PP0_ENERGY_STATUS_NUMFIELDS];
    unsigned long long
        INTEL_MSR_PKG_POWER_INFO_VALUES[INTEL_MSR_PKG_POWER_INFO_NUMFIELDS];

    /*
    Store the increment for each unit in this machine
    */
    float power_increment = 0;
    float energy_increment = 0;
    float time_increment = 0;

    /*
    Store the value at which the energy counter wraps around
    */
    float energy_counter_max = 0;

    /*
    Store the time and value of the latest energy measurement
    */
    EnergyAux pkg_energy_aux;
    EnergyAux cores_energy_aux;

    /*
    Used to extract results from measurement intervals and to store total consumption per node
    */
    EnergyData pkg_energy_data;
    EnergyData cores_energy_data;

    /*
    Store NUMA-related information (Number of nodes, cores per node, id of
    the first core in each node)
    */
    int numa_nodes;

    std::unique_ptr<int[]> first_node_core;
    int numcores;

    /*
    Store cores frequency related information
    */
    int max_freq;
    int min_freq;

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
    The value returned is the sum of the energy consumed by all CPUs in the system
    */
    float get_energy(int domain);

    /*
    Returns the last energy reading of the specified RAPL domain in Joules
    The value returned is the sum of the energy consumed by the CPU in the specified
    NUMA node
    */
    float get_node_energy(int node, int domain);

    /*
    Returns the average power consumed by the specified RAPL domain, in Watts.
    This function will return the average power consumed since the last time it
    was called with that EnergyAux struct.
    */
    float get_power(struct EnergyAux &data, int domain);

    /*
    Updates the energy and time measurements for all nodes in the EnergyAux struct
    */
    void update_data(struct EnergyAux &data, int domain);

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
    float get_energy_diff(float *current_energy, float *previous_energy);

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

    //////////////////////////////////////////////////////////////////////
    //					GETTERS FOR SPECIFIC VALUES
    //////////////////////////////////////////////////////////////////////

    /*
    Returns the last energy reading of RAPL's Package domain in Joules
    The value returned is the sum of the energy consumed by all CPUs in the system
    */
    float get_package_energy();

    /*
    Returns the average power consumed since the last time this function was called,
    in Watts
    */
    float get_package_power();

    /*
    Starts a measurement interval, storing the current time and energy reading for
    the processor package (The sum of the energy readings for all CPUs in the
    system) in the supplied EnergyAux struct
    */
    void start_package_measurement_interval(EnergyAux &aux_data);

    /*
    Stops the measurement interval. Computes the average power consumption by the
    CPU package in Watts (Sum of all CPUs in the system) since the last time the
    start_package_power_interval function was called on the provided EnergyAux struct
    Also computes the energy consumption during the interval
    Both values are updated in the EnergyAux struct
    */
    void stop_package_measurement_interval(EnergyAux &aux_data, EnergyData &output_data);

    /*
    Returns the last energy reading of RAPL's Core domain in Joules
    The value returned is the sum of the energy consumed by all CPUs in the system
    */
    float get_cores_energy();

    /*
    Returns the average power consumed since the last time this function was called,
    in Watts
    */
    float get_cores_power();

    /*
    Returns the TDP of the CPU in Watts
    The value returned is the aggregate TDP of all the CPUs in the system
    */
    float get_processor_tdp();

} // namespace rapl_utils

#endif