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
// (DcmtkRead.cpp)                                                            //
//                                                                            //
// Steven Dolly                                                               //
// August 17, 2020                                                            //
//                                                                            //
// This is the main file for the utility functions to read DICOM files using  //
// DCMTK.                                                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "DcmtkRead.hpp"

#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmiod/iodcommn.h>
#include <dcmtk/dcmrt/drmdose.h>
#include <dcmtk/dcmrt/drmstrct.h>
#include <dcmtk/dcmrt/drmplan.h>

namespace solutio
{
  ItkImageF3::Pointer DcmtkReadImageSeries(std::vector<std::string> file_list,
    std::function<void(float)> progress_function)
  {
    // Create output image pointer
    using ImageType = ItkImageF3;
    ImageType::Pointer output_image = ImageType::New();
    ImageType::RegionType region;
    ImageType::SizeType size;

    // Global variables
    std::vector< std::pair<float,int> > image_z;
    std::vector<float> vals;
    float val;

    // Pass 1: Get image info and slice ordering
    for(int n = 0; n < file_list.size(); n++)
    {
      // Read in DICOM image modules using DCMTK
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(file_list[n].c_str());
      if(status.good())
      {
        // Read DICOM image and data
        DcmDataset * data = fileformat.getDataset();
        DicomImage Im(data, EXS_Unknown);
        // Assign/check image properties
        // In future, check for multiframe before determining slices
        if(n == 0)
        {
          size[0] = Im.getWidth();
          size[1] = Im.getHeight();
          size[2] = file_list.size();

          ImageType::IndexType start;
          start.Fill(0);

          region.SetIndex(start);
          region.SetSize(size);

          output_image->SetRegions(region);
          output_image->Allocate();
          output_image->FillBuffer(itk::NumericTraits<float>::Zero);

          vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
          double origin[3];
          origin[0] = vals[0];
          origin[1] = vals[1];
          origin[2] = vals[2];
          output_image->SetOrigin(origin);
          std::pair<float,int> zp(vals[2], n);
          image_z.push_back(zp);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
          val = GetDicomValue<float>(data, DCM_SliceThickness);
          double spacing[3];
          spacing[0] = vals[1];
          spacing[1] = vals[0];
          spacing[2] = val;
          output_image->SetSpacing(spacing);
          vals.clear();

          vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
          ImageType::DirectionType direction;
          direction.SetIdentity();
          direction[0][0] = vals[0];
          direction[1][0] = vals[1];
          direction[2][0] = vals[2];
          direction[0][1] = vals[3];
          direction[1][1] = vals[4];
          direction[2][1] = vals[5];
          direction[0][2] = (vals[1]*vals[5] - vals[2]*vals[4]);
          direction[1][2] = (vals[2]*vals[3] - vals[0]*vals[5]);
          direction[2][2] = (vals[0]*vals[4] - vals[1]*vals[3]);
          output_image->SetDirection(direction);
          vals.clear();
        }
        else
        {
          if(Im.getHeight() != size[1] || Im.getWidth() != size[0])
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
        progress_function(float(n+1) / float(2*file_list.size()));
      }
      else
      {
        std::cerr << "Image " << n << " from file " << file_list[n] <<
          " could not be read, returning blank image\n";
        return output_image;
      }
    }

    std::sort(image_z.begin(), image_z.end(), [&image_z](
      std::pair<float,int>& l, std::pair<float,int>& r)
        { return l.first < r.first; });

    ImageType::PointType po = output_image->GetOrigin();
    po[2] = image_z[0].first;
    output_image->SetOrigin(po);

    // Pass 2: Read pixel data and place in buffer
    for(int n = 0; n < file_list.size(); n++)
    {
      // Read in DICOM image modules using DCMTK
      DcmFileFormat fileformat;
      OFCondition status = fileformat.loadFile(file_list[n].c_str());
      if(status.good())
      {
        // Read DICOM image and data
        DcmDataset * data = fileformat.getDataset();
        DicomImage Im(data, EXS_Unknown);
        // Assign pixel data
        unsigned long np;
        std::vector<float> temp_buffer;
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

        unsigned int n_slice = 0;
        while(image_z[n_slice].second != n) n_slice++;

        for(auto it = temp_buffer.begin(); it != temp_buffer.end(); ++it)
        {
          unsigned int d = std::distance(temp_buffer.begin(), it);
          const ImageType::IndexType pixel_index =
            {{(d % size[0]), d / size[0], n_slice}};
          output_image->SetPixel(pixel_index, *it);
        }
        temp_buffer.clear();

        progress_function(float(n+file_list.size()+1) / float(2*file_list.size()));
      }
    }

    return output_image;
  }

  ItkImageF3::Pointer DcmtkReadRTDose(std::string file_name,
    std::function<void(float)> progress_function)
  {
    using ImageType = ItkImageF3;
    ImageType::Pointer output_image = ImageType::New();
    ImageType::RegionType region;
    ImageType::SizeType size;

    std::vector<float> vals;
    float val;

    // Read in DICOM image modules using DCMTK
    DcmFileFormat fileformat;
    OFCondition status = fileformat.loadFile(file_name.c_str());
    if(status.good())
    {
      // Read DICOM image and data
      DcmDataset * data = fileformat.getDataset();
      DicomImage Im(data, EXS_Unknown);
      if(Im.isMonochrome())
      {
        // Read image info
        OFString temp;
        if(data->findAndGetOFString(DCM_NumberOfFrames, temp).good())
        {
          val = GetDicomValue<int>(data, DCM_NumberOfFrames);
        }
        else val = 1;
        size[0] = Im.getWidth();
        size[1] = Im.getHeight();
        size[2] = val;

        ImageType::IndexType start;
        start.Fill(0);

        region.SetIndex(start);
        region.SetSize(size);

        output_image->SetRegions(region);
        output_image->Allocate();
        output_image->FillBuffer(itk::NumericTraits<float>::Zero);

        vals = GetDicomArray<float>(data, DCM_ImagePositionPatient, 3);
        double origin[3];
        origin[0] = vals[0];
        origin[1] = vals[1];
        origin[2] = vals[2];
        output_image->SetOrigin(origin);
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_PixelSpacing, 2);
        double spacing[3];
        spacing[0] = vals[1];
        spacing[1] = vals[0];
        if(size[2] == 1) spacing[2] = 1.0;
        else
        {
          std::vector<float> gfov = GetDicomArray<float>(data, DCM_GridFrameOffsetVector, 2);
          spacing[2] = fabs(gfov[1]-gfov[0]);
        }
        output_image->SetSpacing(spacing);
        vals.clear();

        vals = GetDicomArray<float>(data, DCM_ImageOrientationPatient, 6);
        ImageType::DirectionType direction;
        direction.SetIdentity();
        direction[0][0] = vals[0];
        direction[1][0] = vals[1];
        direction[2][0] = vals[2];
        direction[0][1] = vals[3];
        direction[1][1] = vals[4];
        direction[2][1] = vals[5];
        direction[0][2] = (vals[1]*vals[5] - vals[2]*vals[4]);
        direction[1][2] = (vals[2]*vals[3] - vals[0]*vals[5]);
        direction[2][2] = (vals[0]*vals[4] - vals[1]*vals[3]);
        output_image->SetDirection(direction);
        vals.clear();

        // Read pixel data
        long unsigned int np;
        std::vector<float> temp_buffer;
        float dgs = GetDicomValue<float>(data, DCM_DoseGridScaling);
        const DiPixel * pixel_obj = Im.getInterData();
        if(pixel_obj != NULL)
        {
          np = pixel_obj->getCount();
          EP_Representation rep = pixel_obj->getRepresentation();
          if(rep == EPR_Uint32)
          {
            Uint32 * pixel_data = (Uint32 *)pixel_obj->getData();
            progress_function(0.5);
            for(unsigned long p = 0; p < np; p++)
            {
              temp_buffer.push_back(float(pixel_data[p])*dgs);
            }
            for(auto it = temp_buffer.begin(); it != temp_buffer.end(); ++it)
            {
              unsigned int d = std::distance(temp_buffer.begin(), it);
              int n_slice = d / (size[0]*size[1]);
              int c = d % size[0];
              int r = (d - (n_slice*size[0]*size[1])) / size[0];
              const ImageType::IndexType pixel_index = {{c, r, n_slice}};
              output_image->SetPixel(pixel_index, *it);
            }
            temp_buffer.clear();
            progress_function(1.0);
          }
          else
          {
            std::cerr << "Error: Pixel data not Uint32 type\n";
            return output_image;
          }
        }
        else
        {
          std::cerr << "Error: Could not read pixel data\n";
          return output_image;
        }
      }
      else
      {
        std::cerr << "Error: RTDose is not monochrome, returning blank image\n";
        return output_image;
      }
    }
    else
    {
      std::cerr << "Error: Dose from file " << file_name <<
        " could not be read, returning blank image\n";
      return output_image;
    }

    return output_image;
  }

  RTStructureSet DcmtkReadRTS(std::string file_name,
    std::function<void(float)> progress_function)
  {
    RTStructureSet rts;
    Sint32 roi_num, ref_num, con_num, col_num[3], con_id, num_points;
    OFString name, geo_type, att_con;
    OFVector<Float64> point_data;

    // Read in DICOM image modules using DCMTK
    DRTStructureSet rts_dcm;
    OFCondition status = rts_dcm.loadFile(file_name.c_str());
    if(status.good())
    {
      DRTStructureSetROISequence roi_seq = rts_dcm.getStructureSetROISequence();
      if(!roi_seq.isValid())
      {
        std::cout << "Error: StructureSetROISequence not valid\n";
        return rts;
      }
      DRTROIContourSequence roi_con_seq = rts_dcm.getROIContourSequence();
      if(!roi_con_seq.isValid())
      {
        std::cout << "Error: ROIContourSequence not valid\n";
        return rts;
      }
      for(int n = 0; n < roi_seq.getNumberOfItems(); n++)
      {
        solutio::Vec3<double> point;
        RTStructure s;
        DRTStructureSetROISequence::Item roi_seq_it = roi_seq.getItem(n);
        DRTROIContourSequence::Item roi_con_seq_it;
        // Set name
        roi_seq_it.getROIName(name);
        s.SetName(std::string(name.c_str()));
        // Find contour data based on ROI number
        roi_seq_it.getROINumber(roi_num);
        for(int c = 0; c < roi_con_seq.getNumberOfItems(); c++)
        {
          roi_con_seq[c].getReferencedROINumber(con_num);
          if(con_num == roi_num)
          {
            ref_num = c;
            break;
          }
        }
        // Get contour data
        roi_con_seq_it = roi_con_seq.getItem(ref_num);
        if(roi_con_seq_it.isValid())
        {
          // Contour color
          for(int c = 0; c < 3; c++)
          {
            roi_con_seq_it.getROIDisplayColor(col_num[c], c);
          }
          s.SetColor(float(col_num[0]) / 255.0, float(col_num[1]) / 255.0,
            float(col_num[2]) / 255.0);
          // Contour data
          DRTContourSequence con_seq = roi_con_seq_it.getContourSequence();
          for(int c = 0; c < con_seq.getNumberOfItems(); c++)
          {
            StructureContour contour;
            con_seq[c].getContourGeometricType(geo_type);
            contour.SetGeometricType(std::string(geo_type.c_str()));
            con_seq[c].getContourData(point_data);
            con_seq[c].getNumberOfContourPoints(num_points);
            for(int p = 0; p < 3*num_points; p+=3)
            {
              point.x = point_data[p];
              point.y = point_data[p+1];
              point.z = point_data[p+2];
              contour.AddPoint(point);
            }
            s.AddContour(contour);
          }
        }
        else
        {
          std::cout << name << ": " << status.text() << '\n';
          continue;
        }
        std::cout << roi_num << ": " << s.GetName() << " (" << s.GetColor(0) <<
          ", " << s.GetColor(1) << ", " << s.GetColor(2) << "), " <<
          s.GetNumContours() << " contours\n";
        rts.AddStructure(s);
      }
    }
    else std::cout << status.text() << '\n';

    return rts;
  }

  BrachyPlan DcmtkReadBrachyPlan(std::string file_name)
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
        // Load fraction schemes
        DRTFractionGroupSequence frac_seq = rtp_dcm.getFractionGroupSequence();
        if(frac_seq.isValid())
        {
          for(int n = 0; n < frac_seq.getNumberOfItems(); n++)
          {
            DRTFractionGroupSequence::Item frac_seq_it = frac_seq.getItem(n);
            FractionGroup fg;
            frac_seq_it.getFractionGroupNumber(id_num);
            fg.Index = id_num;
            frac_seq_it.getNumberOfFractionsPlanned(id_num);
            fg.Fractions = id_num;
            fg.Type = "Brachy";
            DRTReferencedBrachyApplicationSetupSequenceInRTFractionSchemeModule
              bass_seq = frac_seq_it.getReferencedBrachyApplicationSetupSequence();
            for(int b = 0; b < bass_seq.getNumberOfItems(); b++)
            {
              DRTReferencedBrachyApplicationSetupSequenceInRTFractionSchemeModule::Item
                bass_seq_it = bass_seq.getItem(b);
              bass_seq_it.getBrachyApplicationSetupDose(strength);
              fg.Dose.push_back(double(strength));
              bass_seq_it.getReferencedBrachyApplicationSetupNumber(id_num);
              fg.DoseID.push_back(id_num);
            }
            bp.FractionGroups.push_back(fg);
          }
        }
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
              bp.DosePoints.push_back(dp);
            }
          }
        }
        // Load treatment machine name
        DRTTreatmentMachineSequenceInRTBrachyApplicationSetupsModule tx_seq =
          rtp_dcm.getTreatmentMachineSequence();
        DRTTreatmentMachineSequenceInRTBrachyApplicationSetupsModule::Item
          tx_seq_it = tx_seq.getItem(0);
        tx_seq_it.getTreatmentMachineName(name);
        bp.TreatmentMachineName = std::string(name.c_str());
        // Load treatment technique and type
        rtp_dcm.getBrachyTreatmentTechnique(name);
        bp.TreatmentTechnique = std::string(name.c_str());
        rtp_dcm.getBrachyTreatmentType(name);
        bp.TreatmentType = std::string(name.c_str());
        // Load source sequence
        DRTSourceSequence source_seq = rtp_dcm.getSourceSequence();
        if(source_seq.isValid())
        {
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
            bp.Sources.push_back(bs);
          }
        }
        // Load application sequence
        DRTApplicationSetupSequence app_seq =
          rtp_dcm.getApplicationSetupSequence();
        if(app_seq.isValid())
        {
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
              bp.Applicators.push_back(ba);
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
            bp.Applicators.push_back(ba);
          }
        }
      }
      else
      {
        std::string err_text = "Error obtaining DICOM data set. DCMTK Error: ";
        err_text += status.text();
        throw std::runtime_error(err_text);
      }
    }
    else
    {
      std::string err_text = "Error: file is not DICOM or could not be read. DCMTK Error: ";
      err_text += status.text();
      throw std::runtime_error(err_text);
    }
    return bp;
  }
}
