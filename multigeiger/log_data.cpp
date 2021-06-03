// measurement data logging

#include "version.h"
#include "log.h"
#include "log_data.h"

int Serial_Print_Mode;

static const char *Serial_Logging_Name = "Simple Multi-Geiger";
static const char *dashes = "------------------------------------------------------------------------------------------------------------------------";

static const char *Serial_Logging_Header = "     %10s %15s %10s %9s %9s %8s %9s %9s %9s %5s %5s %6s";
static const char *Serial_Logging_Body = "DATA %10d %15d %10f %9f %9d %8d %9d %9f %9f %5.1f %5.1f %6.0f";
static const char *Serial_One_Minute_Log_Header = "     %4s %10s %29s";
static const char *Serial_One_Minute_Log_Body = "DATA %4d %10d %29d";

void setup_log_data(int mode) {
  Serial_Print_Mode = mode;

  // Write Header of Table, depending on the logging mode:
  bool data_log_enabled = (Serial_Print_Mode == Serial_Logging) || (Serial_Print_Mode == Serial_One_Minute_Log) || (Serial_Print_Mode == Serial_Statistics_Log);
  if (data_log_enabled) {
    log(INFO, dashes);
    log(INFO, "%s, Version %s", Serial_Logging_Name, VERSION_STR);
    log(INFO, dashes);
  }
  if (Serial_Print_Mode == Serial_Logging) {
    log(INFO, Serial_Logging_Header,
        "GMC_counts", "Time_difference", "Count_Rate", "Dose_Rate", "HV Pulses", "Accu_GMC", "Accu_Time", "Accu_Rate", "Accu_Dose", "Temp", "Humi", "Press");
    log(INFO, Serial_Logging_Header,
        "[Counts]",   "[ms]",            "[cps]",      "[uSv/h]",   "[-]",       "[Counts]", "[ms]",      "[cps]",     "[uSv/h]",   "[°C]", "[%]",  "[hPa]");
  }
  if (Serial_Print_Mode == Serial_One_Minute_Log) {
    log(INFO, Serial_One_Minute_Log_Header,
        "Time", "Count_Rate", "Counts");
    log(INFO, Serial_One_Minute_Log_Header,
        "[s]",  "[cpm]",      "[Counts per last measurement]");
  }
  if (Serial_Print_Mode == Serial_Statistics_Log) {
    log(INFO, "Time between two impacts");
    log(INFO, "[usec]");
  }
  if (data_log_enabled)
    log(INFO, dashes);
}

void log_data(int GMC_counts, int time_difference, float Count_Rate, float Dose_Rate, int HV_pulse_count,
              int accumulated_GMC_counts, int accumulated_time, float accumulated_Count_Rate, float accumulated_Dose_Rate,
              float t, float h, float p) {
  log(INFO, Serial_Logging_Body,
      GMC_counts, time_difference, Count_Rate, Dose_Rate, HV_pulse_count,
      accumulated_GMC_counts, accumulated_time, accumulated_Count_Rate, accumulated_Dose_Rate,
      t, h, p);
}

void log_data_one_minute(int time_s, int cpm, int counts) {
  log(INFO, Serial_One_Minute_Log_Body,
      time_s, cpm, counts);
}

void log_data_statistics(int count_time_between) {
  log(INFO, "%d", count_time_between);
}
