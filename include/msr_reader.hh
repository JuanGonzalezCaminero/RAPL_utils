#ifndef MSR_READER_HH
#define MSR_READER_HH

#include <stdio.h>
#include <unistd.h>

namespace rapl_utils
{
    /*
    Returns an open file for the MSRs of the specified core

    Needs read permissions for /dev/cpu/[core]/msr
    */
    FILE *open_msr(int core);

    /*
    Returns the value of the MSR at the specified address in the specified MSR file
    */
    unsigned long long read_msr(FILE *file, unsigned int address);

    /*
    Reads all fields from the msr at msr_address and stores their values in the
    msr_values array

    msr_address: The offset in bytes from the start of the MSR file to the beginning
    of the MSR msr_numfields: The number of values to be extracted from the MSR
    msr_offsets: An array containing the offset in bits from the start of the MSR to
    each of the fields msr_sizes: An array containing the size in bits of each field
    msr_values: The array in which the values of each field will be stored.
    */
    void read_msr_fields(int core, const unsigned int msr_address,
                         const unsigned int msr_numfields,
                         const unsigned int *msr_offsets,
                         const unsigned int *msr_sizes,
                         unsigned long long *msr_values);

    /*
    Returns a variable with the last "size" least significant bits set to 1
    */
    unsigned long long get_mask(unsigned int size);
} // namespace rapl_utils

#endif