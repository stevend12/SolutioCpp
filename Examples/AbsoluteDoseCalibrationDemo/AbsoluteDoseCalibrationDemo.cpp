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
// AbsoluteDoseCalibrationDemo.cpp                                            //
// Demonstration of SolutioCpp AbsoluteDoseCalibration Features               //
// Created June 7, 2023 (Steven Dolly)                                        //
//                                                                            //
// This file demonstrates various uses of the Solutio C++ library for         //
// absolute dose calibrations of medical linear accelerator beams. Results    //
// can be viewed using Octave/MATLAB and the script                           //
// AbsoluteDoseCalibrationDemo.m (in same folder).                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ headers
#include <string>
#include <fstream>

// Solutio library headers
#include "Therapy/AbsoluteDoseCalibration.hpp"

int main()
{
  solutio::AbsoluteDoseCalibration Test;

  ///////////////////////////////////////////////////////////////////
  // Primary Use: Calculate absorbed dose for different algorithms //
  ///////////////////////////////////////////////////////////////////

  // Set chamber and electrometer parameters
  solutio::IonChamberElectrometerPair ExradinA19;
  ExradinA19.SetDoseToWaterCalibration(4.723e7); // N_Dw
  ExradinA19.SetElectrometerCorrection(1.0); // P_elec
  // Set chamber properties from model name
  if(!ExradinA19.SetChamber("Exradin A19"))
  {
    std::cout << "Error: chamber model not found, see list below:\n";
    std::vector<std::string> list = ExradinA19.GetChamberModelList();
    for(auto it : list) std::cout << it << '\n';
    return 0;
  }

  // Set photon calibration beam (Linac-based 6 MV)
  double pdd_10 = 66.3;
  solutio::CalibrationBeam Photon6x("6x", "Photon");
  Photon6x.SetQuality("PDD 10", pdd_10);
  Photon6x.IsPulsed(true); // False by default, use false for Co-60 beam
  Photon6x.SetProfileCorrection(0.999); // P_rp (1.0 by default)
  // False by default, uncomment if beam is Co-60
  //Photon6x.IsCobalt60(true);
  if(!Photon6x.IsValid())
  {
    std::cout << Photon6x.GetErrorMessage() << '\n';
    return 0;
  }

  // Set photon measurement conditions
  double m_raw = 1.414e-08; // Main reading for absolute calculation (C)
  double m_low = 1.408e-08; // Reading at lower voltage for P_ion (C)
  double m_opp = 1.416e-08; // Reading at opposite voltage for P_pol (C)
  double v_ratio = 300.0 / 150.0; // Voltage ratio for P_ion
  double mu = 100.0; // Machine monitor units (MU)

  solutio::IonChamberElectrometerMeasurment PhotonMeas;
  PhotonMeas.SetTemperaturePressure(21.1, 743.8); // T: Celcius, P: mmHg
  PhotonMeas.SetMeasurement(m_raw, m_low, m_opp, v_ratio);

  // Calculate absolute dose for photon measurement
  std::cout << "TG-51 Photons\n";
  std::cout << "-------------\n";
  // A value of "true" for the 4th parameter means the algorithm will try to use
  // fit/table values from the TG-51 and Addendum reports, while a "false" value
  // will perform a manual calculation of the correction factors (see manual
  // calculations below)
  std::cout << "Dose rate @ depth (TG-51) = " <<
    100.0 * Test.DoseTG51(ExradinA19, Photon6x, PhotonMeas, true) / mu << " (cGy/MU)\n";
  std::cout << "Dose rate @ d_max (TG-51) = " <<
    (100.0 * Test.DoseTG51(ExradinA19, Photon6x, PhotonMeas, true) / mu) /
    (0.01*pdd_10) << " (cGy/MU)\n\n";

  // Set electron calibration beam (Linac-based 6 MeV)
  double r50 = 2.33;
  double pdd_dref = 99.93;
  solutio::CalibrationBeam Electron6e("6e", "Electron");
  Electron6e.SetQuality("R50", r50);
  Electron6e.IsPulsed(true);
  if(!Electron6e.IsValid())
  {
    std::cout << Electron6e.GetErrorMessage() << '\n';
    return 0;
  }

  // Set photon measurement conditions
  m_raw = 2.251e-08; // Main reading for absolute calculation (C)
  m_low = 2.2215e-08; // Reading at lower voltage for P_ion (C)
  m_opp = 2.2515e-08; // Reading at opposite voltage for P_pol (C)
  v_ratio = 300.0 / 150.0; // Voltage ratio for P_ion
  double m_gr = 2.2455e-08; // Reading at d_ref + 0.5*r_cav for P_gr (C)
  mu = 100.0; // Machine monitor units (MU)

  solutio::IonChamberElectrometerMeasurment ElectronMeas;
  ElectronMeas.SetTemperaturePressure(21.1, 743.8); // T: Celcius, P: mmHg
  ElectronMeas.SetMeasurement(m_raw, m_low, m_opp, v_ratio, m_gr);

  // Calculate absolute dose for electron measurement
  std::cout << "TG-51 Electrons\n";
  std::cout << "---------------\n";
  // A value of "true" for the 4th parameter means the algorithm will try to use
  // fit/table values from the TG-51 report, while a "false" value will perform
  // a manual calculation of the correction factors (see manual calculations
  // below)
  std::cout << "Dose rate @ depth (TG-51) = " <<
    100.0 * Test.DoseTG51(ExradinA19, Electron6e, ElectronMeas, true) / mu << " (cGy/MU)\n";
  std::cout << "Dose rate @ d_max (TG-51) = " <<
    (100.0 * Test.DoseTG51(ExradinA19, Electron6e, ElectronMeas, true) / mu) /
    (0.01*pdd_dref) << " (cGy/MU)\n\n";

  ////////////////////////////////////////////////////////////////////////
  // Secondary Use Case: Calculate photon beam quality factors manually //
  ////////////////////////////////////////////////////////////////////////
  std::cout << "Photon Correction Factors\n";
  std::cout << "-------------------------\n";
  std::cout << "P_ion = " << Test.P_ion(Photon6x, PhotonMeas) << '\n';
  std::cout << "P_TP = " << Test.P_TP(PhotonMeas) << '\n';
  std::cout << "P_pol = " << Test.P_pol(PhotonMeas) << '\n';
  std::cout << "P_wall = " << Test.P_wall(ExradinA19, Photon6x) << '\n';
  std::cout << "P_fl = " << Test.P_fl(ExradinA19, Photon6x) << '\n';
  std::cout << "P_gr = " << Test.P_gr(ExradinA19, Photon6x) << '\n';
  std::cout << "P_cel = " << Test.P_cel(ExradinA19, Photon6x) << '\n';
  std::cout << "k_Q = " << Test.k_Q(ExradinA19, Photon6x) << '\n'; // Manual calculation
  std::cout << "k_Q (TG-51 Addendum Fit) = " <<
    Test.k_Q_fit(ExradinA19, Photon6x) << "\n\n"; // TG-51 Addendum polynomial fit

  //////////////////////////////////////////////////////////////////////////
  // Secondary Use Case: Calculate electron beam quality factors manually //
  //////////////////////////////////////////////////////////////////////////
  std::cout << "Electron Correction Factors\n";
  std::cout << "---------------------------\n";
  std::cout << "P_ion = " << Test.P_ion(Electron6e, ElectronMeas) << '\n';
  std::cout << "P_TP = " << Test.P_TP(ElectronMeas) << '\n';
  std::cout << "P_pol = " << Test.P_pol(ElectronMeas) << '\n';
  std::cout << "P_wall = " << Test.P_wall(ExradinA19, Electron6e) << '\n';
  std::cout << "P_fl = " << Test.P_fl(ExradinA19, Electron6e) << '\n';
  std::cout << "P_gr = " << Test.P_gr(ElectronMeas) << '\n';
  std::cout << "P_cel = " << Test.P_cel(ExradinA19, Electron6e) << '\n';
  std::cout << "k_R50_prime = " << Test.k_R50_prime(ExradinA19, Electron6e) << '\n'; // Manual calculation
  std::cout << "k_R50_prime (TG-51 Fit) = " <<
    Test.k_R50_prime_fit(Electron6e) << "\n"; // TG-51 fit equation
  std::cout << "k_ecal = " << Test.k_ecal(ExradinA19) << '\n'; // Manual calculation
  std::cout << "k_ecal (TG-51 Table) = " <<
    Test.k_ecal_table(ExradinA19) << "\n\n"; // TG-51 table

  ///////////////////////////////////////////////////
  // Secondary Use Case: Other ancillary functions //
  ///////////////////////////////////////////////////
  std::cout << pdd_10 << " (PDD_10) = " << Test.PDD10_To_TPR2010(pdd_10) <<
    " TPR_20_10\n";
  std::cout << "0.668 (TPR_20_10) = " << Test.TPR2010_To_PDD10(0.668) <<
    " PDD_10\n";
  std::cout << "Restricted stopping power ratio (water-to-air) for a photon" <<
    " beam with PDD_10 = " << pdd_10 << ": " << Test.RSPR_Water_Air_Photons(pdd_10)
    << '\n';
  std::cout << "Restricted stopping power ratio (water-to-air) for an electron" <<
    " beam with R50 = " << r50 << ": " << Test.RSPR_Water_Air_Electrons(r50)
    << '\n';

  ////////////////////////////////////////////////////////
  // Validation Plots for AbsoluteDoseCalibrationDemo.m //
  ////////////////////////////////////////////////////////
  std::ofstream fout;

  // k_Q (Exradin A19 only)
  fout.open("k_Q.txt");
  solutio::CalibrationBeam PhotonTest("Test", "Photon");
  for(int en = 0; en < 24; en++)
  {
    double pdd = 63.0 + double(en);
    PhotonTest.SetQuality("PDD 10", pdd);
    fout << pdd << "," << Test.k_Q(ExradinA19, PhotonTest) << "," <<
      Test.k_Q_fit(ExradinA19, PhotonTest) << '\n';
  }
  fout.close();

  // k_R50_prime (selected chambers)
  fout.open("k_R50_prime.txt");
  solutio::IonChamberElectrometerPair ValidationTest;
  solutio::CalibrationBeam ElectronTest("Test", "Electron");
  std::vector<std::string> chamber_list = {"Exradin A12", "IBA CC13", "NE2561",
    "PTW 30013"};
  for(int n = 0; n < chamber_list.size(); n++)
  {
    ValidationTest.SetChamber(chamber_list[n]);
    for(int en = 2; en < 10; en++)
    {
      ElectronTest.SetQuality("R50", double(en));
      fout << Test.k_R50_prime(ValidationTest, ElectronTest);
      if(en != 9) fout << ",";
    }
    fout << '\n';
  }
  for(int en = 2; en < 10; en++)
  {
    ElectronTest.SetQuality("R50", double(en));
    fout << Test.k_R50_prime_fit(ElectronTest);
    if(en != 9) fout << ",";
  }
  fout.close();

  // k_ecal (selected chambers)
  fout.open("k_ecal.txt");
  for(int n = 0; n < chamber_list.size(); n++)
  {
    ValidationTest.SetChamber(chamber_list[n]);
    fout << Test.k_ecal(ValidationTest) << "," <<
      Test.k_ecal_table(ValidationTest) << '\n';
  }
  fout.close();

  return 0;
}
