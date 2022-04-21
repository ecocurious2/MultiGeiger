// measurement data logging

#ifndef _LOG_DATA_H_
#define _LOG_DATA_H_

// Values for Serial_Print_Mode to configure Serial (USB) output mode.
/* TR, 20.04.2022 :
Serial_Print_Mode should be included in NOLOG...DEBUG sequence (--> settable through the loglevel!).
   It's not clear to me, why Serial_Print_Mode is set fix at compilation time ...
   Current settings are :
Serial_None 0            // No Serial output
Serial_Debug 1           // Only debug and error messages
Serial_Logging 2         // Log measurements as a table (default)
Serial_One_Minute_Log 3  // One Minute logging
Serial_Statistics_Log 4  // Logs time [us] between two events, 1/s

Proposal:
rename standard log levels and include Serial_xxx :
NOLOG     0 --> incl. Serial_None, display ALARMS, else turn off logging
ERROR     1 --> incl.  0
MIN_INFO  2 --> incl. Serial_Statistics_Log + 0 + 1
MED_INFO  3 --> incl. Serial_One_Minute_Log + 0 + 1
MAX_INFO  4 --> incl. Serial_Logging + 0 + 1
DEBUG     5 --> incl. Serial_Debug + 0 + 1 + 4
*/
#define Serial_None 0            // No Serial output
#define Serial_Debug 5           // Only debug and error messages
#define Serial_Logging 4         // Log measurements as a table
#define Serial_One_Minute_Log 3  // One Minute logging
#define Serial_Statistics_Log 2  // Logs time [us] between two events

extern int Serial_Print_Mode;

void setup_log_data(int mode);
void log_data(int GMC_counts, int time_difference, float Count_Rate, float Dose_Rate, int HV_pulse_count,
              int accumulated_GMC_counts, int accumulated_time, float accumulated_Count_Rate, float accumulated_Dose_Rate,
              float t, float h, float p);
void log_data_one_minute(int time_s, int cpm, int counts);
void log_data_statistics(int count_time_between);
void write_log_header(void);
#endif // _LOG_DATA_H_
