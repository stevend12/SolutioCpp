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
// ITK Utilities                                                              //
// (SolutioItk.hpp)                                                           //
//                                                                            //
// Steven Dolly                                                               //
// August 18, 2020                                                            //
//                                                                            //
// This file contains utilities to extend ITK for the SolutioCpp library.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef SOLUTIOITK_HPP
#define SOLUTIOITK_HPP

#include <itkImage.h>

namespace solutio
{
  typedef itk::Image<float, 3> ItkImageF3;
  typedef itk::Image<double, 3> ItkImageD3;
}

#endif
