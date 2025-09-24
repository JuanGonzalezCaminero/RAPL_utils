#include "const.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////
//						  	   DATA
//////////////////////////////////////////////////////////////////////

struct PowerInfo {
  struct timespec time;
  // We need a counter for each NUMA node in the system, this assumes
  // a maximum of 8 nodes, but may be increased or decreased as needed
  float energy[8];
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
struct PowerInfo pkg_power_data;
struct PowerInfo *node_pkg_power_data;
struct PowerInfo cores_power_data;

/*
Store NUMA-related information (Number of nodes, cores per node, id of
the first core in each node)
*/
int numa_nodes;
int *first_node_core;
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
was called with that PowerInfo struct.
*/
float get_power(struct PowerInfo *data, int domain);

float get_node_power(int node, struct PowerInfo *data, int domain);

void update_data(struct PowerInfo *data, int domain);

void update_node_data(int node, struct PowerInfo *data, int domain);

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
The value returned is for the CPU in the specified node
*/
float get_node_package_energy(int node);

/*
Returns the average power consumed since the last time this function was called,
in Watts
*/
float get_node_package_power(int node);

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
system) in the supplied PowerInfo struct
*/
void start_package_power_interval(struct PowerInfo *data);

/*
Returns the average power consumption by the CPU package in Watts (Sum of all
CPUs in the system) since the last time the start_package_power_interval
function was called on the supplied PowerInfo struct
*/
float stop_package_power_interval(struct PowerInfo *data);

/*
Starts a measurement interval, storing the current energy reading for the
processor package (The sum of the energy readings for all CPUs in the system) in
the supplied EnergyInfo struct
*/
void start_package_energy_interval(struct PowerInfo *data);

/*
Returns the energy consumption by the CPU package in Joules (Sum of all CPUs in
the system) since the last time the start_package_power_interval function was
called on the supplied EnergyInfo struct
*/
float stop_package_energy_interval(struct PowerInfo *data);

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

/*
Returns the frequency in Hertz of the CPU. This function assumes the frequency
is only being modified using this library, so it returns the frequency of CPU 0
and assumes every core is running at the same frequency.
*/
int get_cores_freq();

/*
Returns the minimum possible frequency the hardware allows for this CPU
*/
int get_min_freq();

/*
Returns the maximum possible frequency the hardware allows for this CPU
*/
int get_max_freq();

//////////////////////////////////////////////////////////////////////
//					   MODYFYING FREQUENCIES
//////////////////////////////////////////////////////////////////////

/*
Changes the maximum frequency of all cores to the specified value, in Hz
*/
void set_cores_freq(int freq);

/*
Changes the maximum frequency of all cores. The new frequency is specified
using a percentage, where 0% is the minimum frequency the cores can have and
100% is the maximum
*/
void set_cores_freq_percentage(int percentage);

/*
Resets the frequency of all cores to the maximum possible
*/
void reset_cores_freq();

/*
Prints the current frequency of each core to the standard output
*/
void print_cores_freq();