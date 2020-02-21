// measurement data logging

#ifndef _LOG_DATA_H_
#define _LOG_DATA_H_

// Values for Serial_Print_Mode to configure Serial (USB) output mode.
#define Serial_None 0            // No Serial output
#define Serial_Debug 1           // Only debug and error messages
#define Serial_Logging 2         // Log measurements as a table
#define Serial_One_Minute_Log 3  // One Minute logging
#define Serial_Statistics_Log 4  // Logs time [us] between two events

extern int Serial_Print_Mode;

void setup_log_data(int mode);
void log_data(int GMC_counts, int time_difference, float Count_Rate, float Dose_Rate, int HV_pulse_count,
              int accumulated_GMC_counts, int accumulated_time, float accumulated_Count_Rate, float accumulated_Dose_Rate);
void log_data_one_minute(int time_s, int cpm, int counts);
void log_data_statistics(int count_time_between);

#endif // _LOG_DATA_H_
