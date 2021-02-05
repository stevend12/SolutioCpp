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
// (VarianTrajectoryLog.cpp)                                                  //
//                                                                            //
// Steven Dolly                                                               //
// November 30, 2020                                                          //
//                                                                            //
// This is the main file for classes which read Varian trajectory log files   //
// and manage a collection (i.e. database) of these files.                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "VarianTrajectoryLog.hpp"

#include "FileIO.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <iterator>

#include <cmath>

namespace solutio
{
  std::pair<std::string, int> VarianAxisDictionary[21] =
  {
    {"Coll Rtn", 0},
    {"Gantry Rtn", 1},
    {"Y1", 2},
    {"Y2", 3},
    {"X1", 4},
    {"X2", 5},
    {"Couch Vrt", 6},
    {"Couch Lng", 7},
    {"Couch Lat", 8},
    {"Couch Rtn", 9},
    {"Couch Pit", 10},
    {"Couch Rol", 11},
    {"MU", 40},
    {"Beam Hold", 41},
    {"Control Point", 42},
    {"MLC", 50},
    {"TargetPosition", 60},
    {"TrackingTarget", 61},
    {"TrackingBase", 62},
    {"TrackingPhase", 63},
    {"TrackingConformityIndex", 64}
  };

  bool VarianTrajectoryLog::IsTrajectoryLog(std::string filename)
  {
    char bytes[16];
    std::ifstream fin;
    fin.open(filename.c_str(), std::ios::binary);
    fin.read(bytes, 16);
    std::string signature = std::string(bytes);
    fin.close();
    if(signature != "VOSTL") return false;
    return true;
  }

  bool VarianTrajectoryLog::Read(std::string filename)
  {
    char bytes[16], buffer[512];
    std::string txt;
    uint32_t raw32;
    float rawf;

    std::ifstream fin;
    fin.open(filename.c_str(), std::ios::binary);

    ///////////////
    // A. Header //
    ///////////////

    // Read signature
    fin.read(bytes, 16); signature = std::string(bytes);
    if(signature != "VOSTL") return false;
    // Read version
    fin.read(bytes, 16); version = std::string(bytes);
    // Read header size
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    header_size = raw32;
    // Read sampling interval
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    sampling_interval = raw32;
    // Read number of axes sampled
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    num_axes_sampled = raw32;
    // Make axes list
    for(int n = 0; n < num_axes_sampled; n++)
    {
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
      int index = 0;
      while(index < 21 && raw32 != VarianAxisDictionary[index].second) index++;
      if(index != 21)
      {
        axis_data temp_axis;
        temp_axis.axis_enum = VarianAxisDictionary[index].second;
        temp_axis.axis_name = VarianAxisDictionary[index].first;
        temp_axis.num_samples = 0;
        axes.push_back(temp_axis);
      }
    }
    // Make samples per axis
    for(int n = 0; n < num_axes_sampled; n++)
    {
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
      axes[n].num_samples = raw32;
    }
    // Axis Scale
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    axis_scale = raw32;
    // Number of sub-beams
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    num_subbeams = raw32;
    // Is data truncated?
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    if(raw32 == 1) is_truncated = true;
    else is_truncated = false;
    // Number of snapshots
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    num_snapshots = raw32;
    // MLC model
    fin.read(bytes, 4);
    for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
    if(raw32 == 0) mlc_model = "NDS 80";
    else if(raw32 == 2) mlc_model = "NDS 120";
    else if(raw32 == 3) mlc_model = "NDS 120 HD";
    else mlc_model = "NA";
    // Finish header
    int bytes_remaining = 1024 - (64 + 8*num_axes_sampled);
    fin.ignore(bytes_remaining);

    /////////////////////
    // B. Subbeam Data //
    /////////////////////

    for(int s = 0; s < num_subbeams; s++)
    {
      subbeam_data temp_sbd;

      // Control point
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&rawf) + b) = bytes[b];
      temp_sbd.control_point = rawf;
      // Monitor units
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&rawf) + b) = bytes[b];
      temp_sbd.monitor_units = rawf;
      // Radiation time
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&rawf) + b) = bytes[b];
      temp_sbd.radiation_time = rawf;
      // Sequence number of the subbeam
      fin.read(bytes, 4);
      for(int b = 0; b < 4; b++) *((char*)(&raw32) + b) = bytes[b];
      temp_sbd.sequence_number = raw32;
      // Subbeam name
      fin.read(buffer, 512); temp_sbd.name = std::string(buffer);

      subbeams.push_back(temp_sbd);
      fin.ignore(32);
    }

    ///////////////////////////
    // C. Axis Snapshot Data //
    ///////////////////////////

    for(int s = 0; s < num_snapshots; s++)
    {
      std::vector<SnapshotSample> temp_snap;
      for(int a = 0; a < num_axes_sampled; a++)
      {
        for(int n = 0; n < axes[a].num_samples; n++)
        {
          SnapshotSample temp_samp;
          if(axes[a].axis_name == "MLC")
          {
            if(n == 0) temp_samp.Label = "MLC Carriage A";
            else if(n == 1) temp_samp.Label = "MLC Carriage B";
            else
            {
              std::string l = "MLC Leaf ";
              int bank_size = (axes[a].num_samples-2) / 2;
              if(n-2 < bank_size) l += "A";
              else l += "B";
              l += std::to_string(((n-2) % bank_size) + 1);
              temp_samp.Label = l;
            }
          }
          else temp_samp.Label = axes[a].axis_name;
          fin.read(bytes, 4);
          for(int b = 0; b < 4; b++) *((char*)(&rawf) + b) = bytes[b];
          temp_samp.Expected = rawf;
          fin.read(bytes, 4);
          for(int b = 0; b < 4; b++) *((char*)(&rawf) + b) = bytes[b];
          temp_samp.Actual = rawf;
          temp_snap.push_back(temp_samp);
        }
      }
      snapshots.push_back(temp_snap);
    }

    // Close file and return
    fin.close();
    return true;
  }

  void VarianTrajectoryLog::Print()
  {
    std::cout << "\nVarian Trajectory Log File\n";
    std::cout << "--------------------------\n";
    std::cout << "Signature: " << signature << '\n';
    std::cout << "Version: " << version << '\n';
    std::cout << "Header Size (bytes): " << header_size << '\n';
    std::cout << "Sampling Interval (ms): " << sampling_interval << '\n';
    std::cout << "Number of axes sampled: " << num_axes_sampled << '\n';

    std::cout << "\nAxis List:\n";
    for(int n = 0; n < num_axes_sampled; n++)
    {
      std::cout << axes[n].axis_enum << ": " << axes[n].axis_name << " (" <<
        axes[n].num_samples << ")\n";
    }
    std::cout << '\n';

    std::cout << "Axis Scale: " << axis_scale << " (";
    if(axis_scale == 1) std::cout << "Machine Scale";
    else if(axis_scale == 2) std::cout << "Modified IEC 61217";
    else std::cout << "NA";
    std::cout << ")\n";

    std::cout << "Number of sub-beams: " << num_subbeams << '\n';
    if(is_truncated) std::cout << "This data is truncated.\n";
    else std::cout << "This data is not truncated.\n";
    std::cout << "Number of snapshots: " << num_snapshots << '\n';
    std::cout << "MLC Model: " << mlc_model << '\n';

    std::cout << "\nSubbeams:\n";
    for(int n = 0; n < num_subbeams; n++)
    {
      std::cout << n << ") " << subbeams[n].name << ": " <<
        subbeams[n].control_point << ", " << subbeams[n].monitor_units << " MU, "
        << subbeams[n].radiation_time << " s, " << subbeams[n].sequence_number <<
        '\n';
    }
  }

  std::vector<std::string> VarianTrajectoryLog::GetHeader()
  {
    std::vector<std::string> header;
    header.push_back("File Version: "+version);
    header.push_back("Sampling Interval (ms): "+
      std::to_string(sampling_interval));
    if(is_truncated) header.push_back("File Truncated: Yes");
    else header.push_back("File Truncated: No");
    header.push_back("Number of snapshots: "+std::to_string(num_snapshots));
    header.push_back("MLC Model: "+mlc_model);
    header.push_back("Number of sub-beams: "+std::to_string(num_subbeams));
    for(int n = 0; n < num_subbeams; n++)
    {
      header.push_back("Sub-beam "+std::to_string(n+1)+": "+subbeams[n].name+
        ": "+std::to_string(subbeams[n].control_point)+", "+
        std::to_string(subbeams[n].monitor_units)+" MU, "+
        std::to_string(subbeams[n].radiation_time)+" s, "+
        std::to_string(subbeams[n].sequence_number));
    }
    return header;
  }

  std::vector<SnapshotSample> VarianTrajectoryLog::GetSnapshot(int id)
  {
    if(id < num_snapshots) return snapshots[id];
    else
    {
      std::cout << "Warning: returning empty snapshot\n";
      std::vector<SnapshotSample> snap;
      return snap;
    }
  }

  std::vector<VarianTrajectoryLog::AnalysisResults>
    VarianTrajectoryLog::Analyze()
  {
    // Variable initialization
    std::vector<VarianTrajectoryLog::AnalysisResults> results;
    VarianTrajectoryLog::AnalysisResults entry;
    float mlc_max = 0.0, mlc_rms = 0.0, max_rate = 0.0, ave_rate = 0.0;
    int num_mlc = 0;
    // If log data is empty, return
    if(snapshots.size() == 0)
    {
      std::cout << "Trajectory log file not loaded, returning empty results\n";
      return results;
    }
    // Set intial values
    for(int n = 0; n < snapshots[0].size(); n++)
    {
      entry.AxisName = snapshots[0][n].Label;
      entry.MaxError = snapshots[0][n].Actual-snapshots[0][n].Expected;
      entry.RmsError =
        pow(snapshots[0][n].Actual-snapshots[0][n].Expected, 2.0);

      if(entry.AxisName.find("MLC Leaf") != std::string::npos)
      {
        num_mlc++;
        mlc_rms += entry.RmsError;
      }

      results.push_back(entry);
    }
    // Compare snapshots
    for(int s = 1; s < snapshots.size(); s++)
    {
      for(int n = 0; n < snapshots[s].size(); n++)
      {
        // Max error
        entry.MaxError = snapshots[s][n].Actual - snapshots[s][n].Expected;
        if(fabs(entry.MaxError) > fabs(results[n].MaxError))
        {
          results[n].MaxError = entry.MaxError;
        }
        // RMS error
        results[n].RmsError +=
          pow(snapshots[s][n].Actual-snapshots[s][n].Expected, 2.0);
        if(results[n].AxisName.find("MLC Leaf") != std::string::npos)
        {
          mlc_rms += pow(snapshots[s][n].Actual-snapshots[s][n].Expected, 2.0);
        }
        // Max rate
        entry.MaxRate = snapshots[s][n].Actual - snapshots[(s-1)][n].Actual;
        if(fabs(entry.MaxRate) > fabs(results[n].MaxRate))
        {
          results[n].MaxRate = entry.MaxRate;
        }
        // Average absolute rate
        results[n].AveAbsRate += fabs(entry.MaxRate);
        if(results[n].AxisName.find("MLC Leaf") != std::string::npos)
        {
          ave_rate += fabs(entry.MaxRate);
        }
      }
    }
    // Normalize
    for(int n = 0; n < snapshots[0].size(); n++)
    {
      results[n].RmsError = sqrt(results[n].RmsError / float(snapshots.size()));
      results[n].MaxRate /= 0.001*float(sampling_interval);
      results[n].AveAbsRate /=
        0.001*float(sampling_interval)*float(snapshots.size());
    }
    // If file has MLC leaves, aggregrate results for all MLC leaves into a new
    // entry
    if(mlc_model != "NA")
    {
      for(int n = 0; n < results.size(); n++)
      {
        if(results[n].AxisName.find("MLC Leaf") != std::string::npos)
        {
          if(fabs(results[n].MaxError) > fabs(mlc_max))
          {
            mlc_max = results[n].MaxError;
          }
          if(fabs(results[n].MaxRate) > fabs(max_rate))
          {
            max_rate = results[n].MaxRate;
          }
        }
      }
      entry.AxisName = "All MLC Leaves";
      entry.MaxError = mlc_max;
      entry.RmsError = sqrt(mlc_rms / float((snapshots.size()-1)*num_mlc));
      entry.MaxRate = max_rate;
      entry.AveAbsRate = ave_rate /
        (0.001*float(sampling_interval)*float(snapshots.size()-1)*num_mlc);
      results.push_back(entry);
    }

    return results;
  }
  
  void VarianTrajectoryLogDatabase::MakeDatabase(std::string database_path)
  {
    for(const auto & entry : std::filesystem::recursive_directory_iterator(database_path))
    {
      if(!std::filesystem::is_directory(entry.path()))
      {
        std::string file_path = entry.path().string();
        std::string file_name = entry.path().filename().string();

        VarianTrajectoryLog tls;
        file_data temp;
        if(tls.IsTrajectoryLog(file_path))
        {
          temp.path = file_path;
          size_t p1 = (file_name).find_last_of('.');
          std::vector<std::string> el = LineRead(file_name.substr(0, p1), '_');
          int ind = el.size()-1;
          if(el.size() > 0) temp.id = el[0];
          if(el.size() > 1) temp.plan = el[1];
          if(el.size() > 2)
          {
            std::string field_txt = "";
            for(int n = 2; n < ind; n++)
            {
              field_txt += el[n];
              if(n < (ind-1)) field_txt += " ";
            }
            temp.field = field_txt;
          }
          if(el[ind].length() == 14)
          {
            temp.date_text = el[ind].substr(4,2)+'/'+el[ind].substr(6,2)+'/'+
              el[ind].substr(0,4)+" "+el[ind].substr(8,2)+":"+el[ind].substr(10,2)+
              ":"+el[ind].substr(12,2);
            // Set tm struct fields from timestamp
            temp.date.tm_sec = stoi(el[ind].substr(12,2));
            temp.date.tm_min = stoi(el[ind].substr(10,2));
            temp.date.tm_hour = stoi(el[ind].substr(8,2));
            temp.date.tm_mday = stoi(el[ind].substr(6,2));
            temp.date.tm_mon = stoi(el[ind].substr(4,2)) - 1;
            temp.date.tm_year = stoi(el[ind].substr(0,4)) - 1900;
            // Calculate remaining tm struct fields
            time_t timer = mktime(&temp.date);
            temp.date = *(localtime(&timer));
          }
          std::cout << "Made it here\n";
          tlog_files.push_back(temp);
        }
      }
    }
  }

  void VarianTrajectoryLogDatabase::Print()
  {
    for(int n = 0; n < tlog_files.size(); n++)
    {
      std::cout << n+1 << ") " << tlog_files[n].id << ", " << tlog_files[n].plan
        << ", " << tlog_files[n].field << ", " << asctime(&tlog_files[n].date);
    }
  }

  VarianTrajectoryLog VarianTrajectoryLogDatabase::GetLogFile(int n)
  {
    VarianTrajectoryLog tlg;
    if(n >= tlog_files.size())
    {
      std::cout << "Index not found, returning blank log file\n";
    }
    else
    {
      bool did_read = tlg.Read(tlog_files[n].path);
      if(!did_read)
      {
        std::cout << "Warning: Log file could not be read!\n";
      }
    }
    return tlg;
  }

  std::string VarianTrajectoryLogDatabase::GetLogInfo(int n)
  {
    std::string info = "";
    if(n >= tlog_files.size())
    {
      std::cout << "Index not found, returning blank string\n";
    }
    else
    {
      info = tlog_files[n].id+", "+tlog_files[n].plan+", "+
        tlog_files[n].field+", "+std::string(asctime(&tlog_files[n].date));
    }
    return info;
  }
}
