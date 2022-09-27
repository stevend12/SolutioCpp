/******************************************************************************/
/*                                                                            */
/* Copyright 2020 Steven Dolly                                                */
/*                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License");            */
/* you may not use this file except in compliance with the License.           */
/* You may obtain a copy of the License at:                                   */
/*                                                                            */
/*     http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                            */
/* Unless required by applicable law or agreed to in writing, software        */
/* distributed under the License is distributed on an "AS IS" BASIS,          */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   */
/* See the License for the specific language governing permissions and        */
/* limitations under the License.                                             */
/*                                                                            */
/******************************************************************************/

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Varian Trajectory Log Manager                                              //
// (VarianTrajectoryLog.hpp)                                                  //
//                                                                            //
// Steven Dolly                                                               //
// November 30, 2020                                                          //
//                                                                            //
// This file contains the header for classes which read Varian trajectory log //
// files and manage a collection (i.e. database) of these files.              //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef VARIANTLOG_HPP
#define VARIANTLOG_HPP

#include <string>
#include <vector>

#include <ctime>

namespace solutio
{
  // Dictionary of different axis types (files only provide the integer value)
  extern std::pair<std::string, int> VarianAxisDictionary[21];
  // A data struct for a single axis snapshot
  struct SnapshotSample
  {
    std::string Label;
    float Expected;
    float Actual;
  };
  // The data for the trajectory log file, with read/calculation functions
  class VarianTrajectoryLog
  {
    public:
      bool IsTrajectoryLog(std::string filename);
      bool Read(std::string filename);
      void Print();
      std::vector<std::string> GetHeader();
      int GetNumSnapshots(){ return num_snapshots; }
      std::vector<SnapshotSample> GetSnapshot(int id);
      // Data Analysis
      struct AnalysisResults
      {
        std::string AxisName = "None";
        float MaxError = 0.0;
        float RmsError = 0.0;
        float MaxRate = 0.0;
        float AveAbsRate = 0.0;
      };
      std::vector<AnalysisResults> Analyze();
    private:
      // Data types
      struct axis_data
      {
        uint32_t axis_enum;
        std::string axis_name;
        uint32_t num_samples;
      };
      struct subbeam_data
      {
        uint32_t control_point;
        float monitor_units;
        float radiation_time;
        uint32_t sequence_number;
        std::string name;
      };
      // Header
      std::string signature;
      std::string version;
      uint32_t header_size;
      uint32_t sampling_interval;
      uint32_t num_axes_sampled;
      std::vector<axis_data> axes;
      uint32_t axis_scale;
      uint32_t num_subbeams;
      bool is_truncated;
      uint32_t num_snapshots;
      std::string mlc_model;
      // Subbeam and snapshot data
      std::vector<subbeam_data> subbeams;
      std::vector< std::vector<SnapshotSample> > snapshots;
  };
  // A database (collection) of trajectory log files
  class VarianTrajectoryLogDatabase
  {
    public:
      void MakeDatabase(std::string database_path);
      void Print();
      int NumLogFiles(){ return tlog_files.size(); }
      VarianTrajectoryLog GetLogFile(int n);
      std::vector<std::string> GetLogInfo(int n);
    private:
      struct file_data
      {
        std::string path = "NA";
        std::string id = "NA";
        std::string plan = "NA";
        std::string field = "NA";
        std::string date_text = "NA";
        struct std::tm date;
      };
      std::vector<file_data> tlog_files;
  };
}

#endif
