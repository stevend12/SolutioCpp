/******************************************************************************/
/*                                                                            */
/* Copyright 2016-2017 Steven Dolly                                           */
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
// FileIO.hpp                                                                 //
// File Read/Write Helper Functions                                           //
// Created November 3, 2017 (Steven Dolly)                                    //
//                                                                            //
// This header file contains helper functions to read/write text files.       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Header guard
#ifndef FILEIO_HPP
#define FILEIO_HPP

// Standard C++ header files
#include <string>
#include <vector>

namespace solutio
{
  void LineRead(const std::string &s, char delim, std::vector<std::string> &elems);
  std::vector<std::string> LineRead(const std::string &s, char delim);
}

// End header guard
#endif
