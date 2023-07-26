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
// AbsoluteDoseCalibration.hpp                                                //
// Absolute Dose Calibration Calculation                                      //
// Created May 26, 2023 (Steven Dolly)                                        //
//                                                                            //
// This header file defines the classes and algorithms for calculating        //
// absolute dose calibrations for medical linear accelerators. Currently only //
// calculations for calibrated ion chambers are supported.                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guards
#ifndef ABSOLUTEDOSECALIBRATION_HPP
#define ABSOLUTEDOSECALIBRATION_HPP

// C++ headers
#include <iostream>
#include <vector>

// Solutio library headers

namespace solutio
{
  // Initial declaration of main class
  class AbsoluteDoseCalibration;

  // Ion chamber-electrometer pair
  class IonChamberElectrometerPair
  {
    public:
      IonChamberElectrometerPair();
      // Set calibration factors
      void SetDoseToWaterCalibration(double n_dw){ N_Dw_Co60 = n_dw; }
      void SetElectrometerCorrection(double p){ p_elec = p; }
      // Set chamber properties from chamber model name
      bool SetChamber(std::string name);
      std::vector<std::string> GetChamberModelList();
      // Set chamber properties manually
      void SetChamberWall(std::string n, double t);
      void SetChamberSheath(std::string n, double t);
      void SetChamberInnerDiameter(double d){ inner_diameter = d; }
      void SetAluminumElectrode(bool yn){ aluminum_electrode = yn; }
      // Print chamber properties to string vector
      std::vector<std::string> PrintProperties();
    private:
      friend class AbsoluteDoseCalibration;
      // Calibration factors
      double N_Dw_Co60;
      double p_elec;
      // Ion chamber parameters
      std::string model_name;
      std::string chamber_type;
      double chamber_volume;
      std::string wall_name;
      double wall_thickness;
      std::string sheath_name;
      double sheath_thickness;
      double inner_diameter;
      bool aluminum_electrode;
      // Ion chamber list
      std::vector< std::vector<std::string> > ion_chamber_list;
  };

  // Calibration beam container
  class CalibrationBeam
  {
    public:
      CalibrationBeam(std::string nm, std::string mod);
      void SetQuality(std::string specifier, double value);
      std::string GetName(){ return name; }
      void IsPulsed(bool p){ is_pulsed = p; }
      void IsCobalt60(bool c){ is_cobalt_60 = c; }
      void SetProfileCorrection(double p){ p_rp = p; }
      // Check for valid configuration
      bool IsValid();
      std::string GetErrorMessage(){ return error_message; }
    private:
      friend class AbsoluteDoseCalibration;
      std::string name;
      std::string modality;
      std::string quality_specifier;
      double quality_value;
      bool is_pulsed;
      bool is_cobalt_60;
      double p_rp;
      std::string error_message;
  };

  // Ion chamber-electrometer measurement
  class IonChamberElectrometerMeasurment
  {
    public:
      IonChamberElectrometerMeasurment();
      void SetTemperaturePressure(double t, double p);
      void SetMeasurement(double mr, double ml, double mo, double vr,
        double mg = 0.0);
    private:
      friend class AbsoluteDoseCalibration;
      // Measurement data
      double temperature;
      double pressure;
      double m_raw;
      double m_low;
      double m_opp;
      double m_gr;
      double v_ratio;
  };

  // Main class
  class AbsoluteDoseCalibration
  {
    public:
      // Constructor
      AbsoluteDoseCalibration();
      // Absolute dose calculations for various protocols
      double DoseTG51(IonChamberElectrometerPair icep, CalibrationBeam beam,
        IonChamberElectrometerMeasurment meas, bool use_fit = false);
      // Beam quality correction factors
      double k_Q(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double k_Q_fit(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double k_R50_prime(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double k_R50_prime_fit(CalibrationBeam beam);
      double k_ecal(IonChamberElectrometerPair icep);
      double k_ecal_table(IonChamberElectrometerPair icep);
      // Beam quality correction sub-factors
      double P_wall(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double P_fl(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double P_gr(IonChamberElectrometerPair icep, CalibrationBeam beam);
      double P_gr(IonChamberElectrometerMeasurment meas);
      double P_cel(IonChamberElectrometerPair icep, CalibrationBeam beam);
      // Ion chamber correction factors
      double P_ion(CalibrationBeam beam, IonChamberElectrometerMeasurment meas);
      double P_TP(IonChamberElectrometerMeasurment meas);
      double P_pol(IonChamberElectrometerMeasurment meas);
      // Ancillary functions
      double PDD10_To_TPR2010(double pdd_10);
      double TPR2010_To_PDD10(double tpr_20_10);
      double RSPR_Water_Air_Photons(double pdd10);
      double RSPR_Water_Air_Electrons(double r50);
    private:
      // P_wall alpha factor table
      std::vector<double> p_wall_alpha_thickness;
      std::vector<double> p_wall_alpha_tpr;
      std::vector< std::vector<double> > p_wall_alpha_table;
      // P_wall material tables and data containers
      std::vector<double> p_wall_mat_tpr;
      struct ChamberWallMaterialData
      {
        std::string Name;
        double density;
        double rspr_isotope_data[2];
        std::vector<double> rspr_medium_air_data;
        double mu_en_isotope_data[2];
        std::vector<double> mu_en_water_wall_data;
      };
      std::vector<ChamberWallMaterialData> chamber_wall_materials;
      // P_gr table (photons)
      std::vector<double> p_gr_diameter;
      std::vector<double> p_gr_tpr;
      std::vector< std::vector<double> > p_gr_table;
      // P_fl table (electrons)
      std::vector<double> p_fl_diameter;
      std::vector<double> p_fl_energy;
      std::vector< std::vector<double> > p_fl_table;
      // TG-51 k_ecal table with updated chambers
      std::vector< std::vector<std::string> > k_ecal_tg51_table;
      // TG-51 Addendum k_Q polynomial coefficient table
      std::vector< std::vector<std::string> > addendum_coefficients;
  };
}

// End header guard
#endif
