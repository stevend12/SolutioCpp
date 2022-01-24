/******************************************************************************/
/*                                                                            */
/* Copyright 2021 Steven Dolly                                                */
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
// RT Plan                                                                    //
// (RTPlan.cpp)                                                               //
//                                                                            //
// Steven Dolly                                                               //
// March 19, 2021                                                             //
//                                                                            //
// This is the main file for the classes which load, store, and manipulate RT //
// plan data, both for external beam and brachytherapy.                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "RTPlan.hpp"

#include <iostream>

#include "Dicom/DcmtkRead.hpp"

#include <dcmtk/dcmrt/drmplan.h>

namespace solutio
{
  void BrachySource::Print()
  {
    std::cout << "Source Number: " << Number << '\n';
    std::cout << "Source Type: " << Type << '\n';
    std::cout << "Isotope: " << IsotopeName << '\n';
    std::cout << "Half Life (Days): " << IsotopeHalfLife << '\n';
    std::cout << "Source Strength Units: " << StrengthUnits << '\n';
    std::cout << "Source Strength: " << Strength << '\n';
    std::cout << "Reference Date-Time: " << asctime(&StrengthReferenceDateTime) << '\n';
  }

  void BrachyChannel::Print()
  {
    std::cout << "Channel ID: " << Number << '\n';
    std::cout << "Channel Time (s): " << TotalTime << '\n';
    std::cout << "Movement Type: " << SourceMovementType << '\n';
    std::cout << "Referenced Source ID: " << ReferencedSourceNumber << '\n';
    std::cout << "Cumulative Weight: " << FinalCumulativeTimeWeight << '\n';
  }

  void BrachyApplicator::Print()
  {
    std::cout << "Applicator ID: " << Number << '\n';
    std::cout << "Applicator Setup Type: " << Type << '\n';
    std::cout << "Applicator Total Air Kerma: " << TotalStrength << '\n';
    std::cout << "Channels:\n";
    for(int c = 0; c < Channels.size(); c++)
    {
      Channels[c].Print();
      for(int p = 0; p < Channels[c].ControlPoints.size(); p++)
      {
        std::cout << Channels[c].ControlPoints[p].Index << ": (" <<
          Channels[c].ControlPoints[p].Position.x << ", " <<
          Channels[c].ControlPoints[p].Position.y << ", " <<
          Channels[c].ControlPoints[p].Position.z << "); " <<
          Channels[c].ControlPoints[p].RelativePosition << "; " <<
          Channels[c].ControlPoints[p].Weight << "\n";
      }
    }
  }

  BrachyPlan::BrachyPlan()
  {
    TreatmentMachineName = "";
  }

  bool BrachyPlan::ReadDicom(std::string file_name)
  {
    BrachyPlan bp;
    Sint32 id_num, ref_num;
    OFString type, name, units, txt;
    std::string ref_date, ref_time;
    Float64 half_life, strength, rel_pos, weight;
    Float64 pos[3];

    // Read in DICOM file using DCMTK
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file_name.c_str());
    if(status.good())
    {
      // Read DICOM plan data
      DcmDataset * data = fileformat.getDataset();
      DRTPlanIOD rtp_dcm;
      status = rtp_dcm.read(*data);
      if(status.good())
      {
        // Load dose points (if present)
        DRTDoseReferenceSequence dose_seq = rtp_dcm.getDoseReferenceSequence();
        if(dose_seq.isValid())
        {
          for(int n = 0; n < dose_seq.getNumberOfItems(); n++)
          {
            DRTDoseReferenceSequence::Item dose_seq_it = dose_seq.getItem(n);
            dose_seq_it.getDoseReferenceStructureType(type);
            if(type == "COORDINATES")
            {
              ReferenceDosePoint dp;
              dose_seq_it.getDoseReferenceNumber(id_num);
              dp.Index = id_num;
              for(int v = 0; v < 3; v++)
              {
                dose_seq_it.getDoseReferencePointCoordinates(pos[v], v);
              }
              dp.Position.x = pos[0];
              dp.Position.y = pos[1];
              dp.Position.z = pos[2];
              dose_seq_it.getTargetPrescriptionDose(strength);
              dp.Dose = double(strength);
              DosePoints.push_back(dp);
            }
          }
        }
        // Load treatment machine name
        DRTTreatmentMachineSequenceInRTBrachyApplicationSetupsModule tx_seq =
          rtp_dcm.getTreatmentMachineSequence();
        DRTTreatmentMachineSequenceInRTBrachyApplicationSetupsModule::Item
          tx_seq_it = tx_seq.getItem(0);
        tx_seq_it.getTreatmentMachineName(name);
        TreatmentMachineName = std::string(name.c_str());
        // Load source sequence
        DRTSourceSequence source_seq = rtp_dcm.getSourceSequence();
        if(!source_seq.isValid())
        {
          std::cout << "Error: Source Sequence not valid\n";
          return false;
        }
        for(int n = 0; n < source_seq.getNumberOfItems(); n++)
        {
          BrachySource bs;
          DRTSourceSequence::Item source_seq_it = source_seq.getItem(n);
          source_seq_it.getSourceNumber(id_num);
          source_seq_it.getSourceType(type);
          source_seq_it.getSourceIsotopeName(name);
          source_seq_it.getSourceIsotopeHalfLife(half_life);
          source_seq_it.getSourceStrengthUnits(units);
          source_seq_it.getReferenceAirKermaRate(strength);
          source_seq_it.getSourceStrengthReferenceDate(txt);
          ref_date = std::string(txt.c_str());
          source_seq_it.getSourceStrengthReferenceTime(txt);
          ref_time = std::string(txt.c_str());
          struct std::tm ref_dt;
          ref_dt.tm_sec = std::stoi(ref_time.substr(4,2));
          ref_dt.tm_min = std::stoi(ref_time.substr(2,2));
          ref_dt.tm_hour = std::stoi(ref_time.substr(0,2));
          ref_dt.tm_mday = std::stoi(ref_date.substr(6,2));
          ref_dt.tm_mon = std::stoi(ref_date.substr(4,2)) - 1;
          ref_dt.tm_year = std::stoi(ref_date.substr(0,4)) - 1900;
          bs.Number = id_num;
          bs.Type = std::string(type.c_str());
          bs.IsotopeName = std::string(name.c_str());
          bs.IsotopeHalfLife = double(half_life);
          bs.StrengthUnits = std::string(units.c_str());
          bs.Strength = double(strength);
          bs.StrengthReferenceDateTime = ref_dt;
          Sources.push_back(bs);
        }
        // Load application sequence
        DRTApplicationSetupSequence app_seq =
          rtp_dcm.getApplicationSetupSequence();
        if(!app_seq.isValid())
        {
          std::cout << "Error: Application Setup Sequence not valid\n";
          return false;
        }
        for(int n = 0; n < app_seq.getNumberOfItems(); n++)
        {
          BrachyApplicator ba;
          DRTApplicationSetupSequence::Item app_seq_it = app_seq.getItem(n);
          app_seq_it.getApplicationSetupNumber(id_num);
          app_seq_it.getApplicationSetupType(type);
          app_seq_it.getTotalReferenceAirKerma(strength);
          ba.Number = id_num;
          ba.Type = std::string(type.c_str());
          ba.TotalStrength = double(strength);
          DRTChannelSequence ch_seq = app_seq_it.getChannelSequence();
          if(!ch_seq.isValid())
          {
            std::cout << "Warning: Channel Sequence not valid for Applicator ID: "
              << id_num << "\n";
            Applicators.push_back(ba);
            continue;
          }
          for(int c = 0; c < ch_seq.getNumberOfItems(); c++)
          {
            DRTChannelSequence::Item ch_seq_it = ch_seq.getItem(c);
            if(ch_seq_it.isValid())
            {
              BrachyChannel bc;
              ch_seq_it.getChannelNumber(id_num);
              ch_seq_it.getChannelTotalTime(strength);
              ch_seq_it.getSourceMovementType(type);
              ch_seq_it.getReferencedSourceNumber(ref_num);
              ch_seq_it.getFinalCumulativeTimeWeight(weight);
              bc.Number = id_num;
              bc.TotalTime = strength;
              bc.SourceMovementType = std::string(type.c_str());
              bc.ReferencedSourceNumber = ref_num;
              bc.FinalCumulativeTimeWeight = weight;
              DRTBrachyControlPointSequence cp_seq = ch_seq_it.getBrachyControlPointSequence();
              if(cp_seq.isValid())
              {
                for(int p = 0; p < cp_seq.getNumberOfItems(); p++)
                {
                  BrachyControlPoint bcp;
                  cp_seq[p].getControlPointIndex(id_num);
                  bcp.Index = id_num;
                  for(int v = 0; v < 3; v++)
                  {
                    cp_seq[p].getControlPoint3DPosition(pos[v], v);
                  }
                  bcp.Position.x = pos[0];
                  bcp.Position.y = pos[1];
                  bcp.Position.z = pos[2];
                  cp_seq[p].getControlPointRelativePosition(rel_pos);
                  bcp.RelativePosition = double(rel_pos);
                  cp_seq[p].getCumulativeTimeWeight(strength);
                  bcp.Weight = double(strength);
                  bc.ControlPoints.push_back(bcp);
                }
              }
              ba.Channels.push_back(bc);
            }
          }
          Applicators.push_back(ba);
        }
      }
      return true;
    }
    else return false;
  }

  void BrachyPlan::Print()
  {
    std::cout << "Brachy Source List (" << Sources.size() << " sources)\n";
    for(int n = 0; n < Sources.size(); n++)
    {
      Sources[n].Print();
      std::cout << '\n';
    }
    std::cout << "Brachy Applicator List (" << Applicators.size() << " applicators)\n";
    for(int n = 0; n < Applicators.size(); n++)
    {
      Applicators[n].Print();
      std::cout << '\n';
    }
  }
}
