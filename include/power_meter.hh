#ifndef POWER_METER_HH
#define POWER_METER_HH

#include <thread>

namespace power_meter
{
    /*
    Flag used to stop the monitoring loop
    */
    extern bool do_monitoring;
    extern std::thread monitoring_thread; // Default-constructed thread, will get replaced when we launch an actual thread

    /*
    Launch a thread that will take measurements in the background
    */
    void launch_monitoring_loop(unsigned int sampling_interval_ms);

    void stop_monitoring_loop();

    /*
    Power measurement loop, intended to run on a separate thread
    */
    void monitoring_loop(unsigned int sampling_interval_ms);
}

#endif