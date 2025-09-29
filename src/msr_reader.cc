#include "msr_reader.hh"

#define BUFFER_SIZE 32

using namespace rapl_utils;

FILE *rapl_utils::open_msr(int core)
{
  char filename[BUFFER_SIZE];
  snprintf(filename, BUFFER_SIZE, "/dev/cpu/%d/msr", core);
  FILE *file = fopen(filename, "rb");

  if (!file)
  {
    fprintf(stderr, "Could not open %s\n", filename);
  }

  return file;
}

unsigned long long rapl_utils::read_msr(FILE *file, unsigned int address)
{
  // According to the specification, a long long is at least 64 bits long
  unsigned long long data;

  pread(fileno(file), &data, 8, address);

  return data;
}

void rapl_utils::read_msr_fields(int core, const unsigned int msr_address,
                                 const unsigned int msr_numfields,
                                 const unsigned int *msr_offsets,
                                 const unsigned int *msr_sizes,
                                 unsigned long long *msr_values)
{
  unsigned long long field = 0;

  FILE *file = open_msr(core);
  unsigned long long data = read_msr(file, msr_address);

  // Parse the fields and store their values
  for (unsigned int i = 0; i < msr_numfields; i++)
  {
    field = 0;
    field = data >> msr_offsets[i];
    field = field & get_mask(msr_sizes[i]);
    msr_values[i] = field;
  }

  fclose(file);
}

unsigned long long rapl_utils::get_mask(unsigned int size)
{
  unsigned long long mask = 0;
  for (unsigned int i = 0; i < size; i++)
  {
    mask = mask << 1;
    mask += 1;
  }
  return mask;
}