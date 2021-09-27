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
// DCMTK Read Functions                                                       //
// (DcmtkRead.hpp)                                                            //
//                                                                            //
// Steven Dolly                                                               //
// June 4, 2020                                                               //
//                                                                            //
// This file contains utility functions to read DICOM files using DCMTK.      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DCMTKREAD_HPP
#define DCMTKREAD_HPP

#include <string>
#include <functional>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>

#include "../SolutioItk.hpp"
#include "../RTStructureSet.hpp"

namespace solutio
{
  // DCMTK helper functions
  template<class T>
  T GetDicomValue(DcmDataset * data, DcmTagKey key)
  {
    T output;
    OFString v;
    if(data->findAndGetOFString(key, v).good()) std::stringstream(v.c_str()) >> output;
    return output;
  }

  template<class T>
  std::vector<T> GetDicomArray(DcmDataset * data, DcmTagKey key, int length)
  {
    std::vector<T> output;
    for(int l = 0; l < length; l++)
    {
      T val;
      OFString s_val;
      data->findAndGetOFString(key, s_val, l);
      std::stringstream(s_val.c_str()) >> val;
      output.push_back(val);
    }
    return output;
  }

  ItkImageF3::Pointer ReadImageSeries(std::vector<std::string> file_list,
    std::function<void(float)> progress_function =
      [](float p){ std::cout << 100.0*p << "%\n"; });

  ItkImageF3::Pointer ReadRTDose(std::string file_name,
    std::function<void(float)> progress_function =
      [](float p){ std::cout << "Loading RT Dose: " << 100.0*p << "%\n"; });

  RTStructureSet ReadRTS(std::string file_name,
    std::function<void(float)> progress_function =
      [](float p){ std::cout << "Loading RT Structures: " << 100.0*p << "%\n"; });
}

#endif
