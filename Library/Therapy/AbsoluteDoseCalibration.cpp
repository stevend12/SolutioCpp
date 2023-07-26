/******************************************************************************/
/*                                                                            */
/* Copyright 2023 Steven Dolly                                                */
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
// AbsoluteDoseCalibration.cpp                                                //
// Absolute Dose Calibration Calculation                                      //
// Created May 26, 2023 (Steven Dolly)                                        //
//                                                                            //
// This main file defines the classes and algorithms for calculating absolute //
// dose calibrations for medical linear accelerators. Currently only          //
// calculations for calibrated ion chambers are supported.                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "AbsoluteDoseCalibration.hpp"

// C++ headers
#include <iostream>
#include <cmath>

// Solutio library headers
#include "../Utilities/DataInterpolation.hpp"

namespace solutio
{
  IonChamberElectrometerPair::IonChamberElectrometerPair()
  {
    model_name = "None";
    N_Dw_Co60 = p_elec = wall_thickness = sheath_thickness = inner_diameter = 0.0;
    wall_name = sheath_name = "Water";
    aluminum_electrode = false;
    // List of common ion chambers
    ion_chamber_list =
    {
      {"Exradin A1","Cylindrical","0.057","C-552","0.176","Water","0.0","0.2","No"},
      {"Exradin A12","Cylindrical","0.65","C-552","0.088","Water","0.0","0.305","No"},
      {"Exradin A12S","Cylindrical","0.25","C-552","0.088","Water","0.0","0.305","No"},
      {"Exradin A18","Cylindrical","0.125","C-552","0.176","Water","0.0","0.245","No"},
      {"Exradin A19","Cylindrical","0.62","C-552","0.088","Water","0.0","0.305","No"},
      {"Exradin A1SL","Cylindrical","0.057","C-552","0.1936","Water","0.0","0.2025","No"},
      {"IBA CC04","Cylindrical","0.04","C-552","0.07","Water","0.0","0.2","No"},
      {"IBA CC08","Cylindrical","0.08","C-552","0.07","Water","0.0","0.3","No"},
      {"IBA CC13","Cylindrical","0.13","C-552","0.07","Water","0.0","0.3","No"},
      {"IBA CC25","Cylindrical","0.250","C-552","0.07","Water","0.0","0.3","No"},
      {"IBA FC23-C","Cylindrical","0.23","C-552","0.07","Water","0.0","0.31","No"},
      {"IBA FC65-G","Cylindrical","0.65","Graphite","0.073","Water","0.0","0.31","Yes"},
      {"IBA FC65-P","Cylindrical","0.65","Delrin","0.057","Water","0.0","0.31","Yes"},
      {"NE2561","Cylindrical","0.3","Graphite","0.0901","Water","0.0","0.37","Yes"},
      {"NE2571","Cylindrical","0.6","Graphite","0.0612","Water","0.0","0.3140","Yes"},
      {"NE2581","Cylindrical","0.6","A-150","0.040572","Water","0.0","0.315","No"},
      {"NE2611","Cylindrical","0.3","Graphite","0.0901","Water","0.0","0.37","Yes"},
      {"PR-06C/G","Cylindrical","0.65","C-552","0.04928","Water","0.0","0.322","No"},
      {"PTW 30010","Cylindrical","0.6","PMMA","0.039865","Water","0.0","0.305","Yes"},
      {"PTW 30011","Cylindrical","0.6","Graphite","0.078625","Water","0.0","0.305","No"},
      {"PTW 30012","Cylindrical","0.6","Graphite","0.078625","Water","0.0","0.305","Yes"},
      {"PTW 30013","Cylindrical","0.6","PMMA","0.039865","Water","0.0","0.305","Yes"},
      {"PTW 31013","Cylindrical","0.3","PMMA","0.06545","Water","0.0","0.275","Yes"},
    };
  }
  bool IonChamberElectrometerPair::SetChamber(std::string name)
  {
    bool found = false;
    for(int n = 0; n < ion_chamber_list.size(); n++)
    {
      if(name == ion_chamber_list[n][0])
      {
        found = true;
        model_name = name;
        chamber_type = ion_chamber_list[n][1];
        chamber_volume = std::stod(ion_chamber_list[n][2]);
        wall_name = ion_chamber_list[n][3];
        wall_thickness = std::stod(ion_chamber_list[n][4]);
        sheath_name = ion_chamber_list[n][5];
        sheath_thickness = std::stod(ion_chamber_list[n][6]);
        inner_diameter = 2.0*std::stod(ion_chamber_list[n][7]);
        if(ion_chamber_list[n][8] == "Yes") aluminum_electrode = true;
      }
    }
    return found;
  }
  std::vector<std::string> IonChamberElectrometerPair::GetChamberModelList()
  {
    std::vector<std::string> list;
    for(int n = 0; n < ion_chamber_list.size(); n++)
    {
      list.push_back(ion_chamber_list[n][0]);
    }
    return list;
  }
  void IonChamberElectrometerPair::SetChamberWall(std::string n, double t)
  {
    wall_name = n;
    wall_thickness = t;
  }
  void IonChamberElectrometerPair::SetChamberSheath(std::string n, double t)
  {
    sheath_name = n;
    sheath_thickness = t;
  }
  std::vector<std::string> IonChamberElectrometerPair::PrintProperties()
  {
    std::vector<std::string> property_list;

    // Ion chamber parameters
    property_list.push_back("Model Name: " + model_name);
    property_list.push_back("Type: " + chamber_type);
    property_list.push_back("Volume (cc)" +
      std::to_string(chamber_volume));
    property_list.push_back("Wall Material: " + wall_name);
    property_list.push_back("Wall Thickness (g/cm^2): " +
      std::to_string(wall_thickness));
    property_list.push_back("Sheath Material: " + sheath_name);
    property_list.push_back("Sheath Thickness (g/cm^2): " +
      std::to_string(sheath_thickness));
    property_list.push_back("Inner Diameter (cm): " +
      std::to_string(inner_diameter));
    if(aluminum_electrode)
    {
      property_list.push_back("Chamber has aluminum electrode.");
    }
    else property_list.push_back("Chamber does not have aluminum electrode.");

    // Calibration factors
    property_list.push_back("Calibration Factor: " + std::to_string(N_Dw_Co60));
    property_list.push_back("Electrometer Correction Factor" +
      std::to_string(p_elec));

    return property_list;
  }

  CalibrationBeam::CalibrationBeam(std::string nm, std::string mod)
  {
    name = nm;
    modality = mod;
    is_pulsed = false;
    is_cobalt_60 = false;
    p_rp = 1.0;
  }
  void CalibrationBeam::SetQuality(std::string specifier, double value)
  {
    quality_specifier = specifier;
    quality_value = value;
  }
  bool CalibrationBeam::IsValid()
  {
    if(modality != "Photon" && modality != "Electron")
    {
      error_message = "Invalid modality (" + modality +
        "): Only Photon and Electron are supported";
      return false;
    }
    if(modality == "Photon")
    {
      if(quality_specifier != "PDD 10" && quality_specifier != "TPR 20/10")
      {
        error_message = "Invalid quality specifier (" + quality_specifier +
          "): Only PDD 10 and TPR 20/10 are supported for photons";
        return false;
      }
    }
    if(modality == "Electron")
    {
      if(quality_specifier != "R50")
      {
        error_message = "Invalid quality specifier (" + quality_specifier +
          "): Only R50 is supported for electrons";
        return false;
      }
    }
    return true;
  }

  IonChamberElectrometerMeasurment::IonChamberElectrometerMeasurment()
  {
    temperature = pressure = m_raw = m_low = m_opp = v_ratio = m_gr = 0.0;
  }
  void IonChamberElectrometerMeasurment::SetTemperaturePressure(double t,
    double p)
  {
    temperature = t;
    pressure = p;
  }
  void IonChamberElectrometerMeasurment::SetMeasurement(double mr, double ml,
    double mo, double vr, double mg)
  {
    m_raw = mr;
    m_low = ml;
    m_opp = mo;
    v_ratio = vr;
    m_gr = mg;
  }

  AbsoluteDoseCalibration::AbsoluteDoseCalibration()
  {
    // Build P_wall table
    p_wall_alpha_thickness = {0.00, 0.04, 0.10, 0.20, 0.40, 0.55};
    p_wall_alpha_tpr = {0.500, 0.640, 0.665, 0.690, 0.706,
      0.722, 0.743, 0.796, 0.810, 0.825, 0.840};
    p_wall_alpha_table =
    {
      {0.000, 0.449, 0.698, 0.886, 0.992, 1.000},
      {0.000, 0.280, 0.535, 0.690, 0.870, 0.945},
      {0.000, 0.240, 0.430, 0.595, 0.752, 0.845},
      {0.000, 0.210, 0.360, 0.530, 0.680, 0.780},
      {0.000, 0.195, 0.320, 0.475, 0.630, 0.730},
      {0.000, 0.180, 0.295, 0.440, 0.600, 0.690},
      {0.000, 0.160, 0.260, 0.390, 0.540, 0.630},
      {0.000, 0.120, 0.190, 0.300, 0.430, 0.520},
      {0.000, 0.110, 0.170, 0.260, 0.390, 0.470},
      {0.000, 0.105, 0.160, 0.240, 0.360, 0.440},
      {0.000, 0.100, 0.150, 0.230, 0.340, 0.420}
    };

    p_wall_mat_tpr = {0.50, 0.53, 0.56, 0.59, 0.62, 0.65,
      0.68, 0.70, 0.72, 0.74, 0.76, 0.78, 0.80, 0.82, 0.84};
    std::string mat_names[8] = {"Water", "A-150", "C-552", "Delrin", "Graphite",
      "Nylon 66", "PMMA", "Polystyrene"};
    double mat_density[8] = {1.0, 1.127, 1.76, 1.425, 1.7, 1.14, 1.19, 1.06};
    double rspr_isotope[2][8] = {
      {1.136,1.149,0.999,1.085,1.011,1.150,1.107,1.116},
      {1.133,1.142,0.995,1.080,1.002,1.142,1.102,1.110}
    };
    double mu_en_isotope[2][8] = {
      {1.000,1.010,1.111,1.042,1.111,1.013,1.029,1.032},
      {1.000,1.011,1.110,1.042,1.113,1.015,1.030,1.034}
    };
    for(int n = 0; n < 8; n++)
    {
      ChamberWallMaterialData temp;
      temp.Name = mat_names[n];
      temp.density = mat_density[n];
      temp.rspr_isotope_data[0] = rspr_isotope[0][n];
      temp.rspr_isotope_data[1] = rspr_isotope[1][n];
      temp.mu_en_isotope_data[0] = mu_en_isotope[0][n];
      temp.mu_en_isotope_data[1] = mu_en_isotope[1][n];
      chamber_wall_materials.push_back(temp);
    }

    chamber_wall_materials[0].rspr_medium_air_data =
      {1.135,1.134,1.133,1.130,1.127,1.123,1.119,1.116,1.111,1.105,1.099,1.090,
        1.080,1.069,1.059};
    chamber_wall_materials[1].rspr_medium_air_data =
      {1.147,1.145,1.143,1.138,1.135,1.130,1.124,1.121,1.115,1.108,1.101,1.091,
        1.080,1.069,1.058};
    chamber_wall_materials[2].rspr_medium_air_data =
      {0.997,0.997,0.995,0.992,0.990,0.986,0.983,0.980,0.976,0.970,0.964,0.956,
        0.948,0.938,0.929};
    chamber_wall_materials[3].rspr_medium_air_data =
      {1.083,1.082,1.081,1.077,1.074,1.070,1.065,1.062,1.057,1.051,1.044,1.035,
        1.026,1.015,1.005};
    chamber_wall_materials[4].rspr_medium_air_data =
      {1.008,1.007,1.004,1.000,0.996,0.992,0.987,0.984,0.979,0.973,0.967,0.959,
        0.950,0.941,0.932};
    chamber_wall_materials[5].rspr_medium_air_data =
      {1.148,1.146,1.144,1.139,1.135,1.130,1.125,1.121,1.115,1.108,1.100,1.090,
        1.080,1.068,1.057};
    chamber_wall_materials[6].rspr_medium_air_data =
      {1.105,1.104,1.102,1.099,1.096,1.091,1.087,1.084,1.079,1.073,1.066,1.057,
        1.047,1.037,1.027};
    chamber_wall_materials[7].rspr_medium_air_data =
      {1.114,1.113,1.111,1.107,1.104,1.100,1.095,1.092,1.087,1.080,1.074,1.065,
        1.055,1.044,1.034};

    chamber_wall_materials[0].mu_en_water_wall_data =
      {1.000,1.000,1.000,1.000,1.000,1.000,1.000,1.000,1.000,1.000,1.000,1.000,
        1.000,1.000,1.000};
    chamber_wall_materials[1].mu_en_water_wall_data =
      {1.011,1.011,1.011,1.012,1.012,1.013,1.015,1.016,1.019,1.023,1.028,1.035,
        1.043,1.051,1.059};
    chamber_wall_materials[2].mu_en_water_wall_data =
      {1.110,1.110,1.110,1.110,1.110,1.110,1.109,1.108,1.107,1.105,1.103,1.100,
        1.096,1.093,1.089};
    chamber_wall_materials[3].mu_en_water_wall_data =
      {1.043,1.043,1.043,1.042,1.042,1.043,1.043,1.043,1.043,1.044,1.045,1.046,
        1.048,1.049,1.051};
    chamber_wall_materials[4].mu_en_water_wall_data =
      {1.114,1.114,1.113,1.113,1.113,1.114,1.115,1.115,1.117,1.119,1.121,1.125,
        1.130,1.134,1.139};
    chamber_wall_materials[5].mu_en_water_wall_data =
      {1.015,1.015,1.015,1.015,1.015,1.016,1.018,1.019,1.021,1.026,1.030,1.037,
        1.045,1.054,1.062};
    chamber_wall_materials[6].mu_en_water_wall_data =
      {1.031,1.031,1.030,1.030,1.031,1.031,1.032,1.033,1.035,1.038,1.041,1.045,
        1.051,1.056,1.062};
    chamber_wall_materials[7].mu_en_water_wall_data =
      {1.035,1.035,1.034,1.034,1.035,1.036,1.038,1.040,1.042,1.048,1.053,1.061,
        1.071,1.081,1.090};

    // Build P_gr table (photons)
    p_gr_diameter = {0, 2, 4, 6, 8, 10, 12};
    p_gr_tpr = {0.535, 0.660, 0.703, 0.731, 0.750, 0.776, 0.811};
    p_gr_table =
    {
      {1.0000, 0.9975, 0.9949, 0.9922, 0.9894, 0.9866, 0.9836},
      {1.0000, 0.9976, 0.9951, 0.9926, 0.9899, 0.9872, 0.9844},
      {1.0000, 0.9977, 0.9953, 0.9929, 0.9904, 0.9878, 0.9852},
      {1.0000, 0.9979, 0.9958, 0.9935, 0.9912, 0.9888, 0.9863},
      {1.0000, 0.9981, 0.9961, 0.9940, 0.9918, 0.9895, 0.9872},
      {1.0000, 0.9982, 0.9964, 0.9945, 0.9924, 0.9904, 0.9882},
      {1.0000, 0.9983, 0.9966, 0.9948, 0.9930, 0.9911, 0.9892}
    };

    // Build P_fl table (electrons)
    p_fl_diameter = {3, 5, 6, 7};
    p_fl_energy = {2, 3, 5, 7, 10, 15, 20, 30};
    p_fl_table =
    {
      {0.977, 0.962, 0.956, 0.949},
      {0.978, 0.966, 0.959, 0.952},
      {0.982, 0.971, 0.965, 0.960},
      {0.986, 0.977, 0.972, 0.967},
      {0.990, 0.985, 0.981, 0.978},
      {0.995, 0.992, 0.991, 0.990},
      {0.997, 0.996, 0.995, 0.995},
      {1.000, 1.000, 1.000, 1.000}
    };

    // TG-51 k_ecal table with updated chambers
    k_ecal_tg51_table =
    {
      {"Exradin A1", "0.915"},
      {"Exradin A12", "0.906"},
      {"Exradin A19", "0.906"},
      {"IBA CC13", "0.904"},
      {"NE2561", "0.904"},
      {"NE2571", "0.903"},
      {"NE2611", "0.904"},
      {"PR-06C/G", "0.900"},
      {"PTW 30010", "0.897"},
      {"PTW 30011", "0.900"},
      {"PTW 30012", "0.905"},
      {"PTW 30013", "0.897"},
    };

    // TG-51 Addendum k_Q polynomial coefficient table
    addendum_coefficients =
    {
      {"Exradin A1","N","1.0029","1.0230","-1.8030"},
      {"Exradin A12","N","1.0146","0.7770","-1.6660"},
      {"Exradin A12S","N","0.9692","1.9740","-2.4480"},
      {"Exradin A18","N","0.9944","1.2860","-1.9800"},
      {"Exradin A19","N","0.9934","1.3840","-2.1250"},
      {"Exradin A1SL","N","0.9896","1.4100","-2.0490"},
      {"IBA CC08","N","0.9430","2.6370","-2.8840"},
      {"IBA CC13","N","0.9515","2.4550","-2.7680"},
      {"IBA CC25","N","0.9551","2.3530","-2.6870"},
      {"IBA FC23-C","N","0.9820","1.5790","-2.1660"},
      {"IBA FC65-G","N","0.9708","1.9720","-2.4800"},
      {"IBA FC65-P","N","0.9828","1.6640","-2.2960"},
      {"NE2561","Y","0.9722","1.9770","-2.4630"},
      {"NE2571","Y","0.9882","1.4860","-2.1400"},
      {"NE2611","Y","0.9722","1.9770","-2.4630"},
      {"PR-06C/G","Y","0.9519","2.4320","-2.7040"},
      {"PTW 30010","Y","1.0093","0.9260","-1.7710"},
      {"PTW 30011","Y","0.9676","2.0610","-2.5280"},
      {"PTW 30012","Y","0.9537","2.4400","-2.7500"},
      {"PTW 30013","N","0.9652","2.1410","-2.6230"},
      {"PTW 31013","N","0.9725","1.9570","-2.4980"}
    };
  }

  double AbsoluteDoseCalibration::DoseTG51(IonChamberElectrometerPair icep,
    CalibrationBeam beam, IonChamberElectrometerMeasurment meas, bool use_fit)
  {
    double dose;
    double m_corr = meas.m_raw * P_ion(beam, meas) * P_TP(meas) * icep.p_elec *
      P_pol(meas) * beam.p_rp;
    if(beam.modality == "Electron") m_corr *= P_gr(meas);
    if(use_fit) dose = icep.N_Dw_Co60 * k_Q_fit(icep, beam) * m_corr;
    else dose = (icep.N_Dw_Co60 * k_Q(icep, beam) * m_corr);
    return dose;
  }

  double AbsoluteDoseCalibration::k_Q(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double kq = 0.0;
    if(beam.modality == "Photon")
    {
      // Numerator: calibration beam quality
      double rspr_water_air = RSPR_Water_Air_Photons(beam.quality_value);
      double numer = rspr_water_air * P_wall(icep, beam) * P_fl(icep, beam) *
        P_gr(icep, beam) * P_cel(icep, beam);
      // Denominator: Co-60
      CalibrationBeam Co60("Co-60", "Photon");
      Co60.SetQuality("PDD 10", 58.4);
      Co60.IsCobalt60(true);
      double denom = RSPR_Water_Air_Photons(58.4) * P_wall(icep, Co60) *
        P_fl(icep, Co60) * P_gr(icep, Co60) * P_cel(icep, Co60);
      kq = (numer / denom);
    }
    else if(beam.modality == "Electron")
    {
      kq = k_R50_prime(icep, beam) * k_ecal(icep);
    }
    else
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: incorrect beam modality"
      );
    }
    return kq;
  }

  double AbsoluteDoseCalibration::k_Q_fit(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double kq = 0.0;
    bool found;
    if(icep.model_name == "None")
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: chamber not set"
      );
    }
    if(beam.modality == "Photon")
    {
      found = false;
      for(int n = 0; n < addendum_coefficients.size(); n++)
      {
        if(icep.model_name == addendum_coefficients[n][0])
        {
          found = true;
          kq = std::stod(addendum_coefficients[n][2]) +
            std::stod(addendum_coefficients[n][3])*0.001*beam.quality_value +
            std::stod(addendum_coefficients[n][4])*0.00001*beam.quality_value*beam.quality_value;
        }
      }
      if(!found)
      {
        throw std::runtime_error(
          std::string("AbsoluteDoseCalibration Error: TG-51 k_ecal table value not")
            + std::string(" found for chamber: ") + icep.model_name
        );
      }
    }
    else if(beam.modality == "Electron")
    {
      kq = k_R50_prime_fit(beam) * k_ecal_table(icep);
    }
    else
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: incorrect beam modality"
      );
    }
    return kq;
  }

  double AbsoluteDoseCalibration::k_R50_prime(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    if(beam.modality != "Electron")
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: cannot calculate k_R50_prime for this beam modality"
      );
    }
    // Numerator: calibration beam quality
    double rspr_water_air = RSPR_Water_Air_Electrons(beam.quality_value);
    double numer = rspr_water_air * P_wall(icep, beam) *
      P_fl(icep, beam) * P_cel(icep, beam);
    // Denominator: default electron beam quality (R_50 = 7.5 cm)
    CalibrationBeam ElectronRef("Ref", "Electron");
    ElectronRef.SetQuality("R50", 7.5);
    ElectronRef.IsPulsed(true); // False by default, use false for Co-60 beam
    rspr_water_air = RSPR_Water_Air_Electrons(7.5);
    double denom = rspr_water_air * P_wall(icep, ElectronRef) *
      P_fl(icep, ElectronRef) * P_cel(icep, ElectronRef);
    return (numer / denom);
  }

  double AbsoluteDoseCalibration::k_R50_prime_fit(CalibrationBeam beam)
  {
    if(beam.modality != "Electron")
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: cannot calculate k_R50_prime for this beam modality"
      );
    }
    // R_50 limits from 2 to 9
    if(beam.quality_value < 2.0 || beam.quality_value > 9.0)
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: cannot calculate k_R50_prime_fit when R50 is not between 2-9"
      );
    }
    return (0.9905 + 0.0710*exp(-beam.quality_value/3.67));
  }

  double AbsoluteDoseCalibration::k_ecal(IonChamberElectrometerPair icep)
  {
    // Numerator: default electron beam quality (R_50 = 7.5 cm)
    CalibrationBeam ElectronRef("Ref", "Electron");
    ElectronRef.SetQuality("R50", 7.5);
    double rspr_water_air = RSPR_Water_Air_Electrons(7.5);
    double numer = rspr_water_air * P_wall(icep, ElectronRef) *
      P_fl(icep, ElectronRef) * P_cel(icep, ElectronRef);
    // Denominator: Co-60
    CalibrationBeam Co60("Co-60", "Photon");
    Co60.SetQuality("PDD 10", 58.4);
    Co60.IsCobalt60(true);
    double denom = RSPR_Water_Air_Photons(58.4) * P_wall(icep, Co60) *
      P_fl(icep, Co60) * P_gr(icep, Co60) * P_cel(icep, Co60);
    return (numer / denom);
  }

  double AbsoluteDoseCalibration::k_ecal_table(IonChamberElectrometerPair icep)
  {
    double k = 0.0;
    bool found = false;
    if(icep.model_name == "None")
    {
      throw std::runtime_error(
        "AbsoluteDoseCalibration Error: chamber not set"
      );
    }
    for(int n = 0; n < k_ecal_tg51_table.size(); n++)
    {
      if(icep.model_name == k_ecal_tg51_table[n][0])
      {
        found = true;
        k = std::stod(k_ecal_tg51_table[n][1]);
      }
    }
    if(!found)
    {
      throw std::runtime_error(
        std::string("AbsoluteDoseCalibration Error: TG-51 k_ecal table value not")
          + std::string(" found for chamber: ") + icep.model_name
      );
    }
    return k;
  }

  double AbsoluteDoseCalibration::P_wall(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double p_wall = 1.0;
    if(beam.modality == "Photon")
    {
      double pdd_10, tpr_20_10;
      if(beam.quality_specifier == "PDD 10")
      {
        pdd_10 = beam.quality_value;
        tpr_20_10 = PDD10_To_TPR2010(pdd_10);
      }
      else if(beam.quality_specifier == "TPR 20/10")
      {
        tpr_20_10 = beam.quality_value;
        pdd_10 = TPR2010_To_PDD10(tpr_20_10);
      }
      else
      {
        throw std::runtime_error(
          "AbsoluteDoseCalibration Error: invalid photon beam quality specifier"
        );
      }

      double rspr_water_air = RSPR_Water_Air_Photons(pdd_10);

      int w_id = 0;
      int s_id = 0;
      while(icep.wall_name != chamber_wall_materials[w_id].Name &&
        w_id < chamber_wall_materials.size()) w_id++;
      if(w_id >= 8)
      {
        throw std::runtime_error(
          "AbsoluteDoseCalibration Error: chamber wall material not found"
        );
      }
      while(icep.wall_name != chamber_wall_materials[s_id].Name &&
        s_id < chamber_wall_materials.size()) s_id++;
      if(s_id >= 8)
      {
        throw std::runtime_error(
          "AbsoluteDoseCalibration Error: chamber sheath material not found"
        );
      }

      double alpha = LinearInterpolation(p_wall_alpha_tpr, p_wall_alpha_thickness,
        p_wall_alpha_table, tpr_20_10, icep.wall_thickness);
      double tau = LinearInterpolation(p_wall_alpha_tpr, p_wall_alpha_thickness,
        p_wall_alpha_table, tpr_20_10, icep.sheath_thickness);
      double rspr_wall_air, mu_en_water_wall, rspr_sheath_air,
        mu_en_water_sheath;
      if(beam.is_cobalt_60)
      {
        rspr_wall_air = chamber_wall_materials[w_id].rspr_isotope_data[1];
        mu_en_water_wall = chamber_wall_materials[w_id].mu_en_isotope_data[1];
        rspr_sheath_air = chamber_wall_materials[s_id].rspr_isotope_data[1];
        mu_en_water_sheath = chamber_wall_materials[s_id].mu_en_isotope_data[1];
      }
      else
      {
        rspr_wall_air = LinearInterpolation(p_wall_mat_tpr,
          chamber_wall_materials[w_id].rspr_medium_air_data, tpr_20_10);
        mu_en_water_wall = LinearInterpolation(p_wall_mat_tpr,
          chamber_wall_materials[w_id].mu_en_water_wall_data, tpr_20_10);
        rspr_sheath_air = LinearInterpolation(p_wall_mat_tpr,
          chamber_wall_materials[s_id].rspr_medium_air_data, tpr_20_10);
        mu_en_water_sheath = LinearInterpolation(p_wall_mat_tpr,
          chamber_wall_materials[s_id].mu_en_water_wall_data, tpr_20_10);
      }
      p_wall = (alpha*rspr_wall_air*mu_en_water_wall +
        tau*rspr_sheath_air*mu_en_water_sheath +
        (1.0-alpha-tau)*rspr_water_air) / rspr_water_air;
    }
    return p_wall;
  }

  double AbsoluteDoseCalibration::P_fl(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double p_fl = 1.0;
    if(beam.modality == "Electron")
    {
      double z = 0.6*beam.quality_value - 0.1;
      double r_p = 1.2709*beam.quality_value - 0.23;
      double e_z = 2.33*beam.quality_value*(1.0 - z/r_p);
      p_fl = LinearInterpolation(p_fl_energy, p_fl_diameter,
        p_fl_table, e_z, 10.0*icep.inner_diameter);
    }
    return p_fl;
  }

  double AbsoluteDoseCalibration::P_gr(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double p_gr = 1.0;
    if(beam.modality == "Photon")
    {
      double pdd_10, tpr_20_10;
      if(beam.quality_specifier == "PDD 10")
      {
        pdd_10 = beam.quality_value;
        tpr_20_10 = PDD10_To_TPR2010(pdd_10);
      }
      else if(beam.quality_specifier == "TPR 20/10")
      {
        tpr_20_10 = beam.quality_value;
        pdd_10 = TPR2010_To_PDD10(tpr_20_10);
      }
      else
      {
        throw std::runtime_error(
          "AbsoluteDoseCalibration Error: invalid photon beam quality specifier"
        );
      }
      p_gr = LinearInterpolation(p_gr_tpr, p_gr_diameter,
        p_gr_table, tpr_20_10, 10.0*icep.inner_diameter);
    }
    return p_gr;
  }

  double AbsoluteDoseCalibration::P_gr(IonChamberElectrometerMeasurment meas)
  {
    return (meas.m_gr / meas.m_raw);
  }

  double AbsoluteDoseCalibration::P_cel(IonChamberElectrometerPair icep,
    CalibrationBeam beam)
  {
    double p_cel = 1.0;
    if(icep.aluminum_electrode)
    {
      if(beam.modality == "Photon")
      {
        double pdd_10, tpr_20_10;
        if(beam.quality_specifier == "PDD 10")
        {
          pdd_10 = beam.quality_value;
          tpr_20_10 = PDD10_To_TPR2010(pdd_10);
        }
        else if(beam.quality_specifier == "TPR 20/10")
        {
          tpr_20_10 = beam.quality_value;
          pdd_10 = TPR2010_To_PDD10(tpr_20_10);
        }
        else
        {
          throw std::runtime_error(
            "AbsoluteDoseCalibration Error: invalid photon beam quality specifier"
          );
        }
        p_cel = 0.9862+0.000112*pdd_10;
      }
      if(beam.modality == "Electron")
      {
        if(beam.quality_value < 4.3) p_cel = 1.0;
        else if(beam.quality_value > 6.7) p_cel = 0.998;
        else
        {
          std::vector<double> x_data{4.3, 6.7};
          std::vector<double> y_data{1.0, 0.998};
          p_cel = LinearInterpolation(x_data, y_data, beam.quality_value);
        }
      }
    }
    return p_cel;
  }

  double AbsoluteDoseCalibration::P_ion(CalibrationBeam beam,
    IonChamberElectrometerMeasurment meas)
  {
    double p_ion;
    if(!beam.is_pulsed)
    {
      p_ion = (1.0 - meas.v_ratio*meas.v_ratio) /
        (meas.m_raw/meas.m_low - meas.v_ratio*meas.v_ratio);
    }
    else p_ion = (1.0 - meas.v_ratio) / (meas.m_raw/meas.m_low - meas.v_ratio);
    return p_ion;
  }

  double AbsoluteDoseCalibration::P_TP(IonChamberElectrometerMeasurment meas)
  {
    return ((273.2+meas.temperature)/295.2)*(760.0/meas.pressure);
  }

  double AbsoluteDoseCalibration::P_pol(IonChamberElectrometerMeasurment meas)
  {
    double diff;
    if((meas.m_raw > 0.0 && meas.m_opp > 0.0) ||
      (meas.m_raw < 0.0 && meas.m_opp < 0.0))
    {
      diff = meas.m_raw + meas.m_opp;
    }
    else diff = meas.m_raw - meas.m_opp;
    return fabs(diff / (2.0*meas.m_raw));
  }

  double AbsoluteDoseCalibration::PDD10_To_TPR2010(double pdd_10)
  {
    return (-0.8228 + 0.0342*pdd_10 - 0.0001776*pdd_10*pdd_10);
  }
  double AbsoluteDoseCalibration::TPR2010_To_PDD10(double tpr_20_10)
  {
    return (-430.62 + 2181.9*tpr_20_10 - 3318.3*tpr_20_10*tpr_20_10 +
      1746.5*tpr_20_10*tpr_20_10*tpr_20_10);
  }
  double AbsoluteDoseCalibration::RSPR_Water_Air_Photons(double pdd10)
  {
    double rspr;
    if(pdd10 >= 63.35) rspr = 1.275-0.00231*pdd10;
    else
    {
      std::vector<double> x_data{58.4, 63.35};
      std::vector<double> y_data{1.1335, 1.275-0.00231*63.35};
      rspr = LinearInterpolation(x_data, y_data, pdd10);
    }
    return rspr;
  }
  double AbsoluteDoseCalibration::RSPR_Water_Air_Electrons(double r50)
  {
    return (1.2534 - 0.1487*pow(r50, 0.2144));
  }
}
