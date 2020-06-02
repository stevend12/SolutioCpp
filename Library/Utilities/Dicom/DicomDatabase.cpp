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
// DICOM Database Manager                                                     //
// (DicomDatabase.cpp)                                                        //
//                                                                            //
// Steven Dolly                                                               //
// March 13, 2020                                                             //
//                                                                            //
// This file contains the main code for a class which manages a collection    //
// (i.e. database) of DICOM files.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "DicomDatabase.hpp"

#include <iostream>
#include <filesystem>
#include <vector>
#include <iterator>

#include <dcmtk/dcmiod/iodimage.h>
#include <dcmtk/dcmimgle/dcmimage.h>

namespace solutio {
  std::vector< std::tuple<std::string, std::string, std::string> >
    SupportedIODList {
      std::make_tuple("1.2.840.10008.5.1.4.1.1.2","CT","CT"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.1","RTIMAGE","RI"),
      std::make_tuple("1.2.840.10008.5.1.4.1.1.481.2","RTDOSE","RD"),
      //std::make_tuple("1.2.840.10008.5.1.4.1.1.481.3","RTSTRUCT","RS")
      /*
      // Single and multi-frame CT
      if(class_uid == "1.2.840.10008.5.1.4.1.1.2") modality_name = "CT";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.2.1") modality_name = "CT";
      // MRI
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.4") modality_name = "MR";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.4.1") modality_name = "MR";
      // PET
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.128") modality_name = "PET";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.130") modality_name = "PET";
      // US
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.6.1") modality_name = "US";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.3.1") modality_name = "US";
      // Radiation therapy data files
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.1") modality_name = "RT Image";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.2") modality_name = "RT Dose";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.3") modality_name = "RT Structure Set";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.4") modality_name = "RT Beams Treatment Record";
      else if(class_uid == "1.2.840.10008.5.1.4.1.1.481.5") modality_name = "RT Plan";
      else modality_name = "Not Supported";
      */
  };

  DicomDatabaseFile::DicomDatabaseFile()
  {
    modality_name = file_path = "";
  }

  bool DicomDatabaseFile::ReadDicomInfo(std::string file_name)
  {
    // Load DICOM file using DCMTK
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file_name.c_str());
    if (status.good())
    {
      // Read DICOM modules
      DcmDataset * data = fileformat.getDataset();
      std::string class_uid = GetDicomValue<std::string>(data, DCM_SOPClassUID);
      if(class_uid == "1.2.840.10008.1.3.10" || class_uid == "")
      {
        std::cout << "Skipping DICOMDIR for now...\n";
        return false;
      }
      patient_name = GetDicomValue<std::string>(data, DCM_PatientName);
      study_uid = GetDicomValue<std::string>(data, DCM_StudyInstanceUID);
      series_uid = GetDicomValue<std::string>(data, DCM_SeriesInstanceUID);
      // Assign modality label based on SOP class UID; match to supported list
      if(class_uid[(class_uid.length()-1)] == '\0') class_uid.erase(class_uid.length()-1);

      auto it = std::find_if(SupportedIODList.begin(), SupportedIODList.end(),
        [&class_uid](const std::tuple<std::string, std::string, std::string>& e)
          {return std::get<0>(e) == class_uid;});
      if (it != SupportedIODList.end()) modality_name = std::get<2>(*it);
      else modality_name = "Not Supported";
    }
    else
    {
      std::cerr << "Error: cannot read DICOM file (" << status.text() << ")" << '\n';
      return false;
    }
    // Get absolute file path
    std::filesystem::path p = file_name;
    file_path = std::filesystem::absolute(p).u8string();

    return true;
  }

  void DicomDatabaseSeries::SetInfo(std::string psuid, std::string suid, std::string mn)
  {
    parent_study_uid = psuid;
    series_uid = suid;
    modality_name = mn;
  }

  void DicomDatabase::MakeDatabase(std::string database_path)
  {
    OFCondition result;
    // Make list of files in directory and sub-directories
    std::vector<std::string> file_list;
    for(const auto & entry : std::filesystem::recursive_directory_iterator(database_path))
    {
      if(!std::filesystem::is_directory(entry.path()))
      {
        file_list.push_back(entry.path().string());
      }
    }
    // Read and organize each file
    std::vector<std::string>::iterator file_it;
    for(file_it = file_list.begin(); file_it != file_list.end(); file_it++)
    {
      // Attempt to read file
      DicomDatabaseFile temp_file;
      if(!temp_file.ReadDicomInfo(*file_it)) continue;
      dicom_files.push_back(temp_file);
      // Check patient
      std::string pt_text = temp_file.GetPatientName();
      if(std::find(patient_list.begin(), patient_list.end(),
        pt_text) == patient_list.end())
      {
        patient_list.push_back(pt_text);
      }
      // Check study
      std::string st_text = temp_file.GetStudyUID();
      auto study_it = std::find_if(study_list.begin(), study_list.end(),
        [&temp_file,st_text](const std::pair<std::string, std::string>& el){
          return el.first == st_text;} );
      if(study_it == study_list.end())
      {
        std::pair<std::string, std::string> stp(st_text, pt_text);
        study_list.push_back(stp);
      }
      // Check series and assign file
      std::string se_text = temp_file.GetSeriesUID();
      auto series_it = std::find_if(series_list.begin(), series_list.end(),
        [&temp_file,se_text](DicomDatabaseSeries& el){
          return el.GetSeriesUID() == se_text;} );
      if(series_it == series_list.end())
      {
        DicomDatabaseSeries temp_series;
        temp_series.SetInfo(st_text, se_text, temp_file.modality_name);
        temp_series.AddFileID(std::distance(file_list.begin(), file_it));
        series_list.push_back(temp_series);
      }
      else
      {
        series_list[(std::distance(series_list.begin(), series_it))].AddFileID(
          std::distance(file_list.begin(), file_it));
      }
    }
  }

  std::vector<std::string> DicomDatabase::GetSeriesFileNames(unsigned int series_id)
  {
    std::vector<std::string> file_list;
    if(series_id >= series_list.size())
    {
      std::cerr << "Warning: series number " << series_id << " is not available\n";
      return file_list;
    }

    DicomDatabaseSeries dds = GetSeries(series_id);
    for(unsigned int n = 0; n < dds.GetNumFiles(); n++)
    {
      DicomDatabaseFile ddf = GetFile(dds.GetFileID(n));
      file_list.push_back(ddf.GetPath());
    }
    return file_list;
  }

  GenericImage<float> DicomDatabase::GetImageSeries(unsigned int series_id)
  {
    // Output image
    GenericImage<float> output_image;

    // Global variables
    std::vector< std::pair<float,int> > image_z;
    std::vector<float> vals;
    float val;
    std::vector<float> temp_buffer;

    std::vector<std::string> file_list = GetSeriesFileNames(series_id);
    for(int n = 0; n < file_list.size(); n++)
    {
      // Read in DICOM image modules using DCMTK
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(file_list[n].c_str());
      if (status.good())
      {
        // Read DICOM image and data
        DcmDataset * data = fileformat.getDataset();
        DicomImage Im(data, EXS_Unknown);
        // Assign/check image properties

        // In future, check for multiframe before determining slices

        if(n == 0)
        {
          output_image.SetImageSize(Im.getHeight(), Im.getWidth(), file_list.size(), 1);

          vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
          val = GetDicomValue<float>(data, DCM_SliceThickness);
          output_image.SetPixelDimensions(vals[0], vals[1], val);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
          output_image.SetPixelOrigin(vals[0], vals[1], vals[2]);
          std::pair<float,int> zp(vals[2], n);
          image_z.push_back(zp);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
          output_image.SetDirectionCosines(vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
          vals.clear();
        }
        else
        {
          unsigned int * ims = output_image.GetImageSize();
          if(Im.getHeight() != ims[0] || Im.getWidth() != ims[1])
          {
            std::cerr << "Error: Image " << n << " size does not match "
              << "initial values\n";
            return output_image;
          }
          /* Other checks (implement later)
          double * pd = header.GetPixelDimensions();
          if(Im.getHeight() != ims[0] || Im.getWidth() != ims[1])
          {
            std::cerr << "Error: Image " << n << " size does not match "
              << "initial values\n";
            return output_image;
          }

          double * po = header.GetPixelOrigin();

          double * dc = GetDirectionCosines();
          */
          vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
          std::pair<float,int> zp(vals[2], n);
          image_z.push_back(zp);
          vals.clear();
        }

        // Assign pixel data
        unsigned long np;
        if(Im.isMonochrome())
        {
          const DiPixel * pixel_obj = Im.getInterData();
          if(pixel_obj != NULL)
          {
            np = pixel_obj->getCount();
            EP_Representation rep = pixel_obj->getRepresentation();
            if(rep == EPR_Uint8)
            {
              Uint8 * pixel_data = (Uint8 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint8)
            {
              Sint8 * pixel_data = (Sint8 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
            if(rep == EPR_Uint16)
            {
              Uint16 * pixel_data = (Uint16 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint16)
            {
              Sint16 * pixel_data = (Sint16 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
            if(rep == EPR_Uint32)
            {
              Uint32 * pixel_data = (Uint32 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
            if(rep == EPR_Sint32)
            {
              Sint32 * pixel_data = (Sint32 *)pixel_obj->getData();
              for(unsigned long p = 0; p < np; p++)
              {
                temp_buffer.push_back(float(pixel_data[p]));
              }
            }
          }
        }
      }
      else
      {
        std::cerr << "Image " << n << " from file " << file_list[n] <<
          " could not be read, returning blank image\n";
        return output_image;
      }
    }
    // Sort and reorder
    std::vector<float> image_buffer;

    std::sort(image_z.begin(), image_z.end(), [&image_z](
      std::pair<float,int>& l, std::pair<float,int>& r)
        { return l.first < r.first; });

    double * po = output_image.GetPixelOrigin();
    output_image.SetPixelOrigin(po[0], po[1], image_z[0].first);

    unsigned int * im_size = output_image.GetImageSize();
    for(int n = 0; n < image_z.size(); n++)
    {
      unsigned long int p_start =
        im_size[0]*im_size[1]*im_size[3]*image_z[n].second;
      unsigned long int p_end =
        im_size[0]*im_size[1]*im_size[3]*(image_z[n].second+1);
      for(unsigned long int p = p_start; p < p_end; p++)
      {
        image_buffer.push_back(temp_buffer[p]);
      }
    }
    output_image.SetImage(image_buffer);

    return output_image;
  }

  std::vector<std::string> DicomDatabase::PrintTree()
  {
    std::vector<std::string> print_text;
    for(unsigned int p = 0; p < patient_list.size(); p++)
    {
      print_text.push_back(patient_list[p]);
      for(unsigned int st = 0; st < study_list.size(); st++)
      {
        if(study_list[st].second == patient_list[p])
        {
          print_text.push_back("  "+study_list[st].first);
          for(unsigned int se = 0; se < series_list.size(); se++)
          {
            if(series_list[se].CheckStudy(study_list[st].first))
            {
              print_text.push_back("    "+series_list[se].GetModalityName()+
                " ("+std::to_string(series_list[se].GetNumFiles())+" files)");
            }
          }
        }
      }
    }

    return print_text;
  }

  std::vector< std::pair<std::string,int> > DicomDatabase::GetTree()
  {
    std::vector< std::pair<std::string,int> > tree;
    std::pair<std::string,int> temp;
    for(unsigned int p = 0; p < patient_list.size(); p++)
    {
      temp.first = patient_list[p];
      temp.second = 0;
      tree.push_back(temp);
      for(unsigned int st = 0; st < study_list.size(); st++)
      {
        if(study_list[st].second == patient_list[p])
        {
          temp.first = study_list[st].first;
          temp.second = 1;
          tree.push_back(temp);
          for(unsigned int se = 0; se < series_list.size(); se++)
          {
            if(series_list[se].CheckStudy(study_list[st].first))
            {
              temp.first = series_list[se].GetModalityName()+
                " ("+std::to_string(series_list[se].GetNumFiles())+" files)";
              temp.second = 2;
              tree.push_back(temp);
            }
          }
        }
      }
    }
    return tree;
  }
}
