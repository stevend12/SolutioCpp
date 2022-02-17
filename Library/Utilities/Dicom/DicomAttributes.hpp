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
// DICOM Attribute Classes                                                    //
// (DicomAttributes.hpp)                                                      //
//                                                                            //
// Steven Dolly                                                               //
// April 22, 2021                                                             //
//                                                                            //
// This file contains the header for a set of classes which define several    //
// common DICOM attribute types. These attributes can be set by the user and  //
// then written to DICOM files, or they can be read directly from DICOM       //
// files. Read/write functions are accomplished using DCMTK. Attributes are   //
// combined together to form modules (see DicomModules.hpp)                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DICOMATTRIBUTES_HPP
#define DICOMATTRIBUTES_HPP

#include <string>
#include <vector>
#include <sstream>

#include "DcmtkRead.hpp"

namespace solutio {
  // Base attribute class; enables use of read/insert functions for all attributes
  class BaseAttribute
  {
    public:
      virtual void ReadAttribute(DcmDataset * data)
      {
        std::cout << "Warning: BaseAttribute ReadAttribute not defined\n";
      }
      virtual void InsertAttribute(DcmDataset * data)
      {
        std::cout << "Warning: BaseAttribute InsertAttribute not defined\n";
      }
      virtual std::pair<std::string, std::string> Print()
      {
        std::pair<std::string, std::string> att_print;
        std::cout << "Warning: BaseAttribute Print not defined\n";
        att_print.first = "NA"; att_print.second = "NA";
        return att_print;
      }
  };

  // Simple single value attribute; class T determines Get/Set value type for
  // user convenience, but all values are ultimately stored as strings
  template <class T>
  class SingleValueAttribute : public BaseAttribute
  {
    public:
      SingleValueAttribute(){ value = ""; }
      SingleValueAttribute(DcmTagKey k)
      {
        value = "";
        key = k;
      }
      T GetValue()
      {
        T output;
        std::stringstream(value) >> output;
        return output;
      }
      void SetValue(T val)
      {
        std::stringstream ss;
        ss << val;
        value = ss.str();
      }
      void ReadAttribute(DcmDataset * data)
      {
        OFString v;
        if(data->findAndGetOFString(key, v).good())
        {
          value = std::string(v.c_str());
        }
      }
      std::pair<std::string, std::string> Print()
      {
        std::pair<std::string, std::string> att_print;
        std::string tag(key.toString().c_str());
        att_print.first = tag;
        att_print.second = value;
        return att_print;
      }
    private:
      std::string value;
      DcmTagKey key;
  };

  template<>
  inline std::string SingleValueAttribute<std::string>::GetValue(){ return value; }
}

#endif
