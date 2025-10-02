#ifndef POWER_METER_HH
#define POWER_METER_HH

#include <thread>
#include <filesystem>
#include <fstream>

namespace power_meter
{
    // Flag used to stop the monitoring loop
    extern bool do_monitoring;
    extern std::thread monitoring_thread; // Default-constructed thread, will get replaced when we launch an actual thread

    // Output
    extern std::filesystem::path output_dir;
    extern std::filesystem::path cpu_out_filename;
    extern std::filesystem::path gpu_out_filename;
    extern std::ofstream cpu_out;
    extern std::ofstream gpu_out;

    /*
    Launch a thread that will take measurements in the background
    */
    void launch_monitoring_loop(unsigned int sampling_interval_ms);

    void stop_monitoring_loop();

    /*
    Power measurement loop, intended to run on a separate thread
    */
    void monitoring_loop(unsigned int sampling_interval_ms);

    /*
    Output configuration
    */
    void set_output_dir(std::string dir);
    void set_cpu_out_filename(std::string filename);
    void set_gpu_out_filename(std::string filename);
}

#endif