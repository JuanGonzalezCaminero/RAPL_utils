#include "energy_utils.hh"
#include "msr_reader.hh"

#include <stdlib.h>
#include <string.h>

using namespace rapl_utils;

//////////////////////////////////////////////////////////////////////
//						            UTILITY FUNCTIONS
//////////////////////////////////////////////////////////////////////

/*
Inititialize the increment variables. Detect if the machine has several NUMA
nodes and processors, in case it does, it will initialize the variables for each
one (In case the CPUs are different).

This function assumes there is a NUMA node per CPU, and a maximum of 8 CPUs per
motherboard

This function assumes that there may be different CPU models in each socket, but
that both CPUs will have the same reporting precissions for RAPL values. This is
probably a safe assumption since valid CPU combinations will use very similar
CPUs.
*/
void rapl_utils::energy_init()
{
  // Get the number of NUMA nodes. This file contains a list of node IDs
  // separated by "-". The length in characters of the file will be 2 for 1 node
  // (0 + \n), and increase by 2 for each succesive node
  FILE *nodes = fopen("/sys/devices/system/node/online", "r");
  char *nodelist = fgets((char *)malloc(16 * sizeof(char)), 16, nodes);
  fclose(nodes);
  numa_nodes = (int)strlen(nodelist) / 2;
  free(nodelist);

  // Allocate space for the variables of each node
  first_node_core = std::make_unique<int[]>(numa_nodes);

  // Read the values for each node
  for (int i = 0; i < numa_nodes; i++)
  {
    char filename[64];
    snprintf(filename, 64, "/sys/devices/system/node/node%d/cpulist", i);
    FILE *cpulist = fopen(filename, "r");
    // Reads the file and gets the first token (The id of the first core in the
    // NUMA node)
    first_node_core[i] = atoi(
        strtok(fgets((char *)malloc(16 * sizeof(char)), 16, cpulist), "-"));
    fclose(cpulist);
  }

  // Get the total number of cores, gets the number of the last online
  // core (should be all cores in the system), and adds 1
  FILE *onlinecores = fopen("/sys/devices/system/cpu/online", "r");
  numcores =
      atoi(strrchr(fgets((char *)malloc(16 * sizeof(char)), 16, onlinecores),
                   '-') +
           1) +
      1;
  fclose(onlinecores);

  read_INTEL_MSR_RAPL_POWER_UNIT(0);
  power_increment =
      1 / (float)(1 << (unsigned int)INTEL_MSR_RAPL_POWER_UNIT_VALUES[0]);
  energy_increment =
      1 / (float)(1 << (unsigned int)INTEL_MSR_RAPL_POWER_UNIT_VALUES[1]);
  time_increment =
      1 / (float)(1 << (unsigned int)INTEL_MSR_RAPL_POWER_UNIT_VALUES[2]);

  update_data(pkg_energy_aux, 0);
  update_data(cores_energy_aux, 1);

  // The maximum value of the energy counter is 2^32, stored here in joules
  energy_counter_max = ((long)1U << 32) * energy_increment;
}

float rapl_utils::get_node_energy(int node, int domain)
{
  switch (domain)
  {
  // Package
  case 0:
    read_INTEL_MSR_PKG_ENERGY_STATUS(first_node_core[node]);
    printf("%llu\n", INTEL_MSR_PKG_ENERGY_STATUS_VALUES[0]);
    return (float)INTEL_MSR_PKG_ENERGY_STATUS_VALUES[0] * energy_increment;
    break;
  // Cores
  case 1:
    read_INTEL_MSR_PP0_ENERGY_STATUS(first_node_core[node]);
    return (float)INTEL_MSR_PP0_ENERGY_STATUS_VALUES[0] * energy_increment;
    break;
  // Uncore
  case 2:
    fprintf(stderr, "Reading from RAPL's Uncore domain not yet implemented");
    return 0;
    break;
  // DRAM
  case 3:
    fprintf(stderr, "Reading from RAPL's DRAM domain not yet implemented");
    return 0;
    break;
  default:
    fprintf(stderr, "Bad RAPL domain (%d). Supported domains are 0-3", domain);
    return 0;
    break;
  }
}

float rapl_utils::get_energy(int domain)
{
  float energy = 0;

  for (int i = 0; i < numa_nodes; i++)
  {
    energy += get_node_energy(i, domain);
  }

  return energy;
}

float rapl_utils::get_energy_diff(float *current_energy, float *previous_energy)
{
  float energy_diff = 0;
  for (int i = 0; i < numa_nodes; i++)
  {
    float node_energy_diff = (float)(current_energy[i] - previous_energy[i]);
    /*
    If the energy counter has wrapped around for this node, we need to add the
    value before wrapping around to the diff. This is 2^32 per Intel's
    specification
    */
    if (node_energy_diff < 0)
    {
      node_energy_diff += energy_counter_max;
    }
    energy_diff += node_energy_diff;
  }
  return energy_diff;
}

float rapl_utils::get_power(struct EnergyAux &data, int domain)
{
  struct EnergyAux previous_data;
  previous_data.time = data.time;
  for (int i = 0; i < numa_nodes; i++)
  {
    previous_data.energy[i] = data.energy[i];
  }

  update_data(data, domain);

  double time_diff =
      (double)(data.time.tv_sec - previous_data.time.tv_sec) +
      ((double)(data.time.tv_nsec - previous_data.time.tv_nsec) / 1000000000);
  float energy_diff = get_energy_diff(data.energy, previous_data.energy);

  // Power = Energy delta in Joules / Time delta in seconds
  float power = (float)(energy_diff / time_diff);

  return power;
}

void rapl_utils::update_data(struct EnergyAux &data, int domain)
{
  for (int i = 0; i < numa_nodes; i++)
  {
    data.energy[i] = get_node_energy(i, domain);
  }
  clock_gettime(CLOCK_REALTIME, &data.time);
}

//////////////////////////////////////////////////////////////////////
//						                GETTERS
//////////////////////////////////////////////////////////////////////

// float rapl_utils::get_package_energy() { return get_energy(0); }

// float rapl_utils::get_package_power() { return get_power(pkg_energy_aux, 0); }

void rapl_utils::start_package_measurement_interval(struct EnergyAux &aux_data)
{
  update_data(aux_data, 0);
}

void rapl_utils::start_package_measurement_interval()
{
  start_package_measurement_interval(pkg_energy_aux);
}

void rapl_utils::stop_package_measurement_interval(EnergyAux &aux_data, EnergyData &output_data)
{
  // Store average power consumption for this interval
  output_data.power = get_power(aux_data, 0);

  struct EnergyAux previous_data;
  for (int i = 0; i < numa_nodes; i++)
  {
    previous_data.energy[i] = aux_data.energy[i];
  }
  update_data(aux_data, 0);

  auto energy_diff = get_energy_diff(aux_data.energy, previous_data.energy);

  // Store energy consumed during this interval
  output_data.energy = energy_diff;

  // Update the total energy consumed by this node
  output_data.total_energy += energy_diff;
}

void rapl_utils::stop_package_measurement_interval()
{
  stop_package_measurement_interval(pkg_energy_aux, pkg_energy_data);
}

float rapl_utils::get_cores_energy() { return get_energy(1); }

float rapl_utils::get_cores_power() { return get_power(cores_energy_aux, 1); }

float rapl_utils::get_processor_tdp()
{
  float total_tdp = 0;

  for (int i = 0; i < numa_nodes; i++)
  {
    read_INTEL_MSR_PKG_POWER_INFO(first_node_core[i]);
    total_tdp += (float)INTEL_MSR_PKG_POWER_INFO_VALUES[0];
  }

  return total_tdp * power_increment;
}

//////////////////////////////////////////////////////////////////////
//						          READING MSR FIELDS
//////////////////////////////////////////////////////////////////////

void rapl_utils::read_INTEL_MSR_RAPL_POWER_UNIT(int core)
{
  read_msr_fields(
      core, INTEL_MSR_RAPL_POWER_UNIT, INTEL_MSR_RAPL_POWER_UNIT_NUMFIELDS,
      INTEL_MSR_RAPL_POWER_UNIT_OFFSETS, INTEL_MSR_RAPL_POWER_UNIT_SIZES,
      INTEL_MSR_RAPL_POWER_UNIT_VALUES);
}

void rapl_utils::read_INTEL_MSR_PKG_ENERGY_STATUS(int core)
{
  read_msr_fields(
      core, INTEL_MSR_PKG_ENERGY_STATUS, INTEL_MSR_PKG_ENERGY_STATUS_NUMFIELDS,
      INTEL_MSR_PKG_ENERGY_STATUS_OFFSETS, INTEL_MSR_PKG_ENERGY_STATUS_SIZES,
      INTEL_MSR_PKG_ENERGY_STATUS_VALUES);
}

void rapl_utils::read_INTEL_MSR_PP0_ENERGY_STATUS(int core)
{
  read_msr_fields(
      core, INTEL_MSR_PP0_ENERGY_STATUS, INTEL_MSR_PP0_ENERGY_STATUS_NUMFIELDS,
      INTEL_MSR_PP0_ENERGY_STATUS_OFFSETS, INTEL_MSR_PP0_ENERGY_STATUS_SIZES,
      INTEL_MSR_PP0_ENERGY_STATUS_VALUES);
}

void rapl_utils::read_INTEL_MSR_PKG_POWER_INFO(int core)
{
  read_msr_fields(
      core, INTEL_MSR_PKG_POWER_INFO, INTEL_MSR_PKG_POWER_INFO_NUMFIELDS,
      INTEL_MSR_PKG_POWER_INFO_OFFSETS, INTEL_MSR_PKG_POWER_INFO_SIZES,
      INTEL_MSR_PKG_POWER_INFO_VALUES);
}