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
// DICOM Attribute Classes                                                    //
// (DicomAttributes.hpp)                                                      //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the header for a set of classes which define several    //
// common DICOM attribute types. These attributes can be set by the user and  //
// then written to DICOM files, or they can be read directly from DICOM       //
// files. Read/write functions are accomplished using Grassroots DICOM        //
// (GDCM). Attributes are combined together to form modules (see              //
// DicomModules.hpp)                                                          //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef DICOMATTRIBUTES_HPP
#define DICOMATTRIBUTES_HPP

#include <string>
#include <vector>
#include <sstream>

#include <gdcmAttribute.h>
#include <gdcmItem.h>
#include <gdcmSequenceOfItems.h>

namespace solutio {
  // Base attribute class; enables use of read/insert functions for all attributes
  class BaseAttribute
  {
    public:
      virtual void ReadAttribute(const gdcm::DataSet &data)
      {
        std::cout << "Warning: BaseAttribute ReadAttribute not defined\n";
      }
      virtual void InsertAttribute(gdcm::DataSet &data)
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
  template <class T, uint16_t G, uint16_t E>
  class SingleValueAttribute : public BaseAttribute
  {
    public:
      SingleValueAttribute(){ value = ""; }
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
      void ReadAttribute(const gdcm::DataSet &data)
      {
        gdcm::Attribute<G,E> att;
        if(data.FindDataElement(att.GetTag()))
        {
          att.Set(data);
          std::stringstream ss;
          ss << att.GetValue();
          value = ss.str();
        }
        else value = "";
      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::Attribute<G,E> att;
        if(value != "")
        {
          gdcm::DataElement de = att.GetAsDataElement();
          gdcm::VR vr = de.GetVR();
          switch(vr)
          {
            case (gdcm::VR::US):
            {
              uint16_t o_ui16;
              std::stringstream(value) >> o_ui16;
              de.SetByteValue(reinterpret_cast<char*>(&o_ui16),sizeof(uint16_t));
              break;
            }
            default:
              if((value.length() % 2) == 1) value.push_back(' ');
              de.SetByteValue(value.c_str(), value.length());
              break;
          }
          data.Insert(de);
        }
      }
      std::pair<std::string, std::string> Print()
      {
        gdcm::Attribute<G,E> att;
        std::pair<std::string, std::string> att_print;
        std::string tag = '('+att.GetTag().PrintAsPipeSeparatedString()+')';
        att_print.first = tag.replace(5, 1, ", ");
        att_print.second = value;
        return att_print;
      }
    private:
      std::string value;
  };

  // Multi-value attribute; so far only double type is allowed, may extend in
  // the future
  template <uint16_t G, uint16_t E>
  class MultiNumberAttribute : public BaseAttribute
  {
    public:
      MultiNumberAttribute()
      {
        values.clear();
        length = 0;
      }
      MultiNumberAttribute(double val, int l)
      {
        length = l;
        for(int n = 0; n < length; n++) values.push_back(val);
      }
      double GetValue(int n)
      {
        if(n >= length) return 0.0;
        return values[n];
      }
      void SetValue(double val, int n){ if(n < length) values[n] = val; }
      void SetValue(std::vector<double> vals)
      {
        values = vals;
        length = vals.size();
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {
        gdcm::Attribute<G,E> att;
        values.clear();
        length = 0;
        if(data.FindDataElement(att.GetTag()))
        {
          att.Set(data);
          for(unsigned int n = 0; n < att.GetNumberOfValues(); n++)
          {
            values.push_back(att.GetValue(n));
          }
          length = values.size();
        }
      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::Attribute<G,E> att;
        std::stringstream ss;
        for(int n = 0; n < values.size(); n++)
        {
          ss << std::fixed << std::setprecision(4) << values[n];
          if(n != (values.size()-1)) ss << "\\";
        }
        std::string output = ss.str();
        if((output.length() % 2) == 1) output.push_back(' ');
        gdcm::DataElement de = att.GetAsDataElement();
        de.SetByteValue(output.c_str(), output.length());
        data.Insert(de);
      }
      std::pair<std::string, std::string> Print()
      {
        gdcm::Attribute<G,E> att;
        std::pair<std::string, std::string> att_print;
        std::string tag = '('+att.GetTag().PrintAsPipeSeparatedString()+')';
        att_print.first = tag.replace(5, 1, ", ");
        std::stringstream ss;
        ss << "(";
        if(values.size() == 0) ss << "None)";
        for(int v = 0; v < values.size(); v++)
        {
          ss << values[v];
          if(v == values.size()-1) ss << ")";
          else ss << ", ";
        }
        att_print.second = ss.str();
        return att_print;
      }
    private:
      std::vector<double> values;
      int length;
  };

  // Attibute whose value consists of a DICOM tag, represnted as two unsigned
  // 16-bit integers
  template <uint16_t G, uint16_t E>
  class TagAttribute : public BaseAttribute
  {
    public:
      void SetValue(uint16_t gr, uint16_t el)
      {
        tag[0] = gr;
        tag[1] = el;
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {
        if(data.FindDataElement(att.GetTag()))
        {
          att.Set(data);
          const gdcm::DataElement de = att.GetAsDataElement();
          const gdcm::ByteValue *bv = de.GetByteValue();
          const char * temp_buffer = bv->GetPointer();
          for(int n = 0; n < 2; n++)
          {
            uint16_t raw16;
            *((char*)(&raw16)) = temp_buffer[(2*n)];
            *((char*)(&raw16) + 1) = temp_buffer[(2*n+1)];
            tag[n] = raw16;
          }
        }
      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de = att.GetAsDataElement();
        de.SetByteValue(reinterpret_cast<char*>(tag), 4);
        data.Insert(de);
      }
      std::pair<std::string, std::string> Print()
      {
        std::pair<std::string, std::string> att_print;
        std::string tag = '('+att.GetTag().PrintAsPipeSeparatedString()+')';
        att_print.first = tag.replace(5, 1, ", ");
        att_print.second = "Not defined";
        return att_print;
      }
    private:
      gdcm::Attribute<G,E> att;
      uint16_t tag[2];
  };

  // Special case class for the pixel data attribute (7FE0,0010)
  class PixelDataAttribute : public BaseAttribute
  {
    public:
      std::vector<char> GetValue(){ return pixel_data; }
      void SetValue(std::vector<char> buffer){ pixel_data = buffer; }
      void ReadAttribute(const gdcm::DataSet &data)
      {
        const gdcm::DataElement de = data.GetDataElement(gdcm::Tag(0x7fe0,0x0010));
        const gdcm::ByteValue *bv = de.GetByteValue();
        const char * temp_buffer = bv->GetPointer();

        pixel_data.clear();
        for(unsigned long int nb = 0; nb < bv->GetLength(); nb++)
        {
          pixel_data.push_back(temp_buffer[nb]);
        }
      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de(gdcm::Tag(0x7fe0,0x0010));
        de.SetByteValue(&pixel_data[0], pixel_data.size());
        data.Insert(de);
      }
      std::pair<std::string, std::string> Print()
      {
        std::pair<std::string, std::string> att_print;
        att_print.first = "(7FE0, 0010)";
        att_print.second = "Pixel data: "+std::to_string(pixel_data.size())+" bytes";
        return att_print;
      }
    private:
      std::vector<char> pixel_data;
  };

  // Special case class for the RT structure referenced FOR sequence attribute
  class ReferencedFrameOfReferenceSequenceAttribute : public BaseAttribute
  {
    public:
      void SetValue(std::string class_uid, std::string study_uid,
        std::string series_uid, std::string for_uid, std::string in_base,
        int in_start)
      {
        image_sop_class_uid = class_uid;
        image_study_uid = study_uid;
        image_series_uid = series_uid;
        frame_of_ref_uid = for_uid;
        instance_uid_base = in_base;
        instance_uid_start = in_start;
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {

      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = new gdcm::SequenceOfItems;
        sqi->SetLengthToUndefined();
        int total = 252;
        for(int id = 0; id < total; ++id)
        {
          gdcm::Item item;
          item.SetVLToUndefined();
          gdcm::DataSet &subds = item.GetNestedDataSet();
          {
            gdcm::Attribute<0x0008,0x1150> at;
            at.SetValue(image_sop_class_uid);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x0008,0x1155> at;
            at.SetValue((instance_uid_base+std::to_string(instance_uid_start+id)).c_str());
            subds.Insert(at.GetAsDataElement());
          }
          sqi->AddItem(item);
        }

        gdcm::DataElement de1(gdcm::Tag(0x3006,0x0010));
        de1.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi1 = new gdcm::SequenceOfItems;
        sqi1->SetLengthToUndefined();
        de1.SetValue(*sqi1);
        de1.SetVLToUndefined();
        data.Insert(de1);

        gdcm::Item item1;
        item1.SetVLToUndefined();
        gdcm::DataSet &ds2 = item1.GetNestedDataSet();

        gdcm::Attribute<0x0020,0x052> frameofreferenceuid;
        frameofreferenceuid.SetValue(frame_of_ref_uid.c_str());
        ds2.Insert(frameofreferenceuid.GetAsDataElement());

        gdcm::DataElement de2(gdcm::Tag(0x3006,0x0012));
        de2.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi2 = new gdcm::SequenceOfItems;
        sqi2->SetLengthToUndefined();
        de2.SetValue(*sqi2);
        de2.SetVLToUndefined();
        ds2.Insert(de2);

        gdcm::Item item2;
        item2.SetVLToUndefined();
        gdcm::DataSet &ds3 = item2.GetNestedDataSet();

        gdcm::Attribute<0x0008,0x1150> refsopclassuid;
        refsopclassuid.SetValue("1.2.840.10008.5.1.4.1.1.481.3");
        ds3.Insert(refsopclassuid.GetAsDataElement());

        gdcm::Attribute<0x0008,0x1155> refsopinstuid;
        refsopinstuid.SetValue(image_study_uid);
        ds3.Insert(refsopinstuid.GetAsDataElement());

        gdcm::DataElement de3(gdcm::Tag(0x3006,0x0014));
        de3.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi3 = new gdcm::SequenceOfItems;
        sqi3->SetLengthToUndefined();
        de3.SetValue(*sqi3);
        de3.SetVLToUndefined();
        ds3.Insert(de3);

        gdcm::Item item3;
        item3.SetVLToUndefined();
        gdcm::DataSet &ds4 = item3.GetNestedDataSet();

        gdcm::Attribute<0x0020,0x000e> seriesinstanceuid;
        seriesinstanceuid.SetValue(image_series_uid);
        ds4.Insert(seriesinstanceuid.GetAsDataElement());

        gdcm::DataElement de4(gdcm::Tag(0x3006,0x0016));
        de4.SetVR(gdcm::VR::SQ);
        de4.SetValue(*sqi);
        de4.SetVLToUndefined();
        ds4.Insert(de4);

        sqi3->AddItem(item3);
        sqi2->AddItem(item2);
        sqi1->AddItem(item1);
      }
    private:
      // Parent image info
      std::string image_sop_class_uid;
      std::string image_study_uid;
      std::string image_series_uid;
      // Frame of Reference UID
      std::string frame_of_ref_uid;
      // Base of CT series instance UIDs
      std::string instance_uid_base;
      int instance_uid_start;
  };

  class StructureSetROISequenceAttribute : public BaseAttribute
  {
    public:
      void SetValue(std::vector<int> numbers, std::string for_uid,
        std::vector<std::string> names)
      {
        roi_number = numbers;
        ref_for_uid = for_uid;
        roi_name = names;
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {

      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de(gdcm::Tag(0x3006,0x0020));
        de.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = new gdcm::SequenceOfItems;
        sqi->SetLengthToUndefined();
        de.SetValue(*sqi);
        de.SetVLToUndefined();
        data.Insert(de);

        int total = roi_number.size();
        for(int n = 0; n < total; n++)
        {
          gdcm::Item item;
          item.SetVLToUndefined();
          gdcm::DataSet &subds = item.GetNestedDataSet();
          {
            gdcm::Attribute<0x3006,0x0022> at;
            at.SetValue(roi_number[n]);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x3006,0x0024> at;
            at.SetValue(ref_for_uid.c_str());
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x3006,0x0026> at;
            at.SetValue(roi_name[n].c_str());
            subds.Insert(at.GetAsDataElement());
          }
          sqi->AddItem(item);
        }
      }
    private:
      // ROI Number (3006,0022)
      std::vector<int> roi_number;
      // Referenced Frame of Reference UID (3006,0024)
      std::string ref_for_uid;
      // ROI Name (3006,0026)
      std::vector<std::string> roi_name;
  };

  class ROIContourSequenceAttribute : public BaseAttribute
  {
    public:
      void SetValue(std::vector<int> numbers, std::vector<int> colors,
        std::vector< std::vector< std::vector<float> > > data)
      {
        ref_roi_number = numbers;
        roi_color = colors;
        contour_data = data;
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {

      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de(gdcm::Tag(0x3006,0x0039));
        de.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = new gdcm::SequenceOfItems;
        sqi->SetLengthToUndefined();
        de.SetValue(*sqi);
        de.SetVLToUndefined();
        data.Insert(de);

        int total = ref_roi_number.size();
        for(int n = 0; n < total; n++)
        {
          gdcm::Item item;
          item.SetVLToUndefined();
          gdcm::DataSet &subds = item.GetNestedDataSet();
          {
            gdcm::Attribute<0x3006,0x0084> at;
            at.SetValue(ref_roi_number[n]);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x3006,0x002a> at;
            int intcolor[3];
            intcolor[0] = roi_color[(3*n)];
            intcolor[1] = roi_color[(3*n+1)];
            intcolor[2] = roi_color[(3*n+2)];
            at.SetValues(intcolor, 3);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::DataElement de1(gdcm::Tag(0x3006,0x0040));
            de1.SetVR(gdcm::VR::SQ);
            gdcm::SmartPointer<gdcm::SequenceOfItems> sqi1 = new gdcm::SequenceOfItems;
            sqi1->SetLengthToUndefined();
            de1.SetValue(*sqi1);
            de1.SetVLToUndefined();
            subds.Insert(de1);

            int total_contours = contour_data[n].size();
            for(int c = 0; c < total_contours; c++)
            {
              gdcm::Item item1;
              item1.SetVLToUndefined();
              gdcm::DataSet &subds1 = item1.GetNestedDataSet();
              {
                gdcm::Attribute<0x3006,0x0048> at;
                at.SetValue(c+1);
                subds1.Insert(at.GetAsDataElement());
              }
              {
                gdcm::Attribute<0x3006,0x0042> at;
                if(contour_data[n].size() == 1 && contour_data[n][0].size() == 3)
                {
                  at.SetValue("POINT");
                }
                else at.SetValue("CLOSED_PLANAR");
                subds1.Insert(at.GetAsDataElement());
              }
              {
                gdcm::Attribute<0x3006,0x0046> at;
                at.SetValue(contour_data[n][c].size() / 3);
                subds1.Insert(at.GetAsDataElement());
              }
              {
                gdcm::Attribute<0x3006,0x0050> at;
                std::stringstream ss;
                int total_points = contour_data[n][c].size();
                if(total_points > 0)
                {
                  for(int p = 0; p < total_points; p++)
                  {
                    ss << std::setprecision(5) << contour_data[n][c][p];
                    if(p != (total_points-1)) ss << "\\";
                  }
                  std::string output = ss.str();
                  gdcm::DataElement de2 = at.GetAsDataElement();
                  de2.SetByteValue(output.c_str(), output.length());
                  subds1.Insert(de2);
                }
              }
              sqi1->AddItem(item1);
            }
          }
          sqi->AddItem(item);
        }
      }
    private:
      // Referenced ROI Number (3006,0084)
      std::vector<int> ref_roi_number;
      // ROI Display Color (3006,002A)
      std::vector<int> roi_color;
      // Contour Sequence (3006,0040) Data:
      // Contour Geometric Type (3006,0042)
      //std::vector< std::vector< std::vector<std::string> > > contour_type;
      // Contour Data (3006,0050)
      std::vector< std::vector< std::vector<float> > > contour_data;
  };

  class RTROIObservationsSequenceAttribute : public BaseAttribute
  {
    public:
      void SetValue(std::vector<int> numbers, std::vector<std::string> types)
      {
        ref_roi_number = numbers;
        interpreted_type = types;
      }
      void ReadAttribute(const gdcm::DataSet &data)
      {

      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de(gdcm::Tag(0x3006,0x0080));
        de.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = new gdcm::SequenceOfItems;
        sqi->SetLengthToUndefined();
        de.SetValue(*sqi);
        de.SetVLToUndefined();
        data.Insert(de);

        int total = ref_roi_number.size();
        for(int n = 0; n < total; n++)
        {
          gdcm::Item item;
          item.SetVLToUndefined();
          gdcm::DataSet &subds = item.GetNestedDataSet();
          {
            gdcm::Attribute<0x3006,0x0082> at;
            at.SetValue(n);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x3006,0x0084> at;
            at.SetValue(ref_roi_number[n]);
            subds.Insert(at.GetAsDataElement());
          }
          {
            gdcm::Attribute<0x3006,0x00a4> at;
            at.SetValue(interpreted_type[n].c_str());
            subds.Insert(at.GetAsDataElement());
          }
          sqi->AddItem(item);
        }
      }
    private:
      // Observation Number (3006,0082)
      std::vector<int> observation_number;
      // Referenced ROI Number (3006,0084)
      std::vector<int> ref_roi_number;
      // RT ROI Interpreted Type (3006,00A4)
      std::vector<std::string> interpreted_type;
  };

  class ReferencedRTPlanSequenceAttribute : public BaseAttribute
  {
    public:
      void SetValue(std::string p_uid){ plan_uid = p_uid; }
      void ReadAttribute(const gdcm::DataSet &data)
      {
        if(data.FindDataElement(gdcm::Tag(0x300c,0x0002)))
        {
          gdcm::DataElement de = data.GetDataElement(gdcm::Tag(0x300c,0x0002));
        }
      }
      void InsertAttribute(gdcm::DataSet &data)
      {
        gdcm::DataElement de(gdcm::Tag(0x300c,0x0002));
        de.SetVR(gdcm::VR::SQ);
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = new gdcm::SequenceOfItems;
        sqi->SetLengthToUndefined();
        de.SetValue(*sqi);
        de.SetVLToUndefined();
        data.Insert(de);

        gdcm::Item item;
        item.SetVLToUndefined();
        gdcm::DataSet &subds = item.GetNestedDataSet();
        {
          gdcm::Attribute<0x0008,0x1150> at;
          at.SetValue("1.2.840.10008.5.1.4.1.1.481.5");
          subds.Insert(at.GetAsDataElement());
        }
        {
          gdcm::Attribute<0x0008,0x1155> at;
          at.SetValue(plan_uid.c_str());
          subds.Insert(at.GetAsDataElement());
        }
        sqi->AddItem(item);
      }
      std::pair<std::string, std::string> Print()
      {
        std::pair<std::string, std::string> att_print;
        att_print.first = "(300C, 0002)";
        att_print.second = "Not defined";
        return att_print;
      }
    private:
      std::string plan_uid;
  };
}

#endif
