/******************************************************************************/
/*                                                                            */
/* Copyright 2016-2018 Steven Dolly                                           */
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
// Statistics Utilities                                                       //
// (Statistics.cpp)                                                           //
//                                                                            //
// Steven Dolly                                                               //
// February 27, 2018                                                          //
//                                                                            //
// This file contains the main code for the statistics-based utilities.       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Class header
#include "Statistics.hpp"

// C headers
#include <cstdlib>
#include <cmath>

namespace solutio {
  // Generate a pseudo-random number according to a normal distribution
  double RandNormal(double mean, double stddev)
  {
    static double num2 = 0.0;
    static int num2_cached = 0;
    if(!num2_cached)
    {
      double x, y, r;
      do
      {
        x = 2.0*rand()/RAND_MAX - 1;
        y = 2.0*rand()/RAND_MAX - 1;
        r = x*x + y*y;
      }
      while (r == 0.0 || r > 1.0);
      {
        double d = sqrt(-2.0*log(r)/r);
        double num1 = x*d;
        num2 = y*d;
        double result = num1*stddev + mean;
        num2_cached = 1;
        return result;
      }
    }
    else
    {
      num2_cached = 0;
      return num2*stddev + mean;
    }
  }
}
