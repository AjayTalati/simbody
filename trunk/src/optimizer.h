
#ifndef _SimTK_OPTIMIZER_H
#define _SimTK_OPTIMIZER_H

/* Portions copyright (c) 2006 Stanford University and Jack Middleton.
 * Contributors:
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


// #include "objectiveFunction.h"

#include "optimizerInterface.h"

namespace SimTK {


class smOptimizer :  public smOptimizerInterface {

   public:
    smOptimizer( int );

smStatus setOptimizerParameters(unsigned int param, double *values); 
smStatus getOptimizerParameters(unsigned int param, double *values);
smStatus setObjectiveFunction(void (*func)(double*,double*,double*)); 
template < typename T > smStatus optimize(T);
template < int N, typename T, int S > smStatus optimize(SimTK::Vec< N, T, S> ) ;

    private:
     void *data;

}; // Class smOptimizer
} // namespace SimTK

#endif //_SimTK_OPTIMIZER_H

