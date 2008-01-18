
/* Portions copyright (c) 2007 Stanford University and Jack Middleton.
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
/**@file
 *
 * Solves for singular values and singular vectors 
 */


#include <iostream> 
#include <cstdio>
#include <malloc.h>
#include <math.h>
#include <complex>
#include "SimTKcommon.h"
#include "LapackInterface.h"
#include "LinearAlgebra.h"
#include "FactorSVDRep.h"
#include "WorkSpace.h"
#include "LATraits.h"
#include "LapackConvert.h"


namespace SimTK {

   //////////////////
   // FactorSVDDefault //
   //////////////////
FactorSVDDefault::FactorSVDDefault() {
    isFactored = false;
}
FactorSVDRepBase* FactorSVDDefault::clone() const {
    return( new FactorSVDDefault(*this));
}


   ///////////
   // FactorSVD //
   ///////////
FactorSVD::~FactorSVD() {
    delete rep;
}
FactorSVD::FactorSVD() {
   rep = new FactorSVDDefault();
}
// copy constructor
FactorSVD::FactorSVD( const FactorSVD& c ) {
    rep = c.rep->clone();
}
// copy assignment operator
FactorSVD& FactorSVD::operator=(const FactorSVD& rhs) {
    rep = rhs.rep->clone();
    return *this;
}
int FactorSVD::getRank() {
     return(rep->getRank() );
}
template < typename ELT >
void FactorSVD::solve( const Vector_<ELT>& b, Vector_<ELT>& x ) {
    rep->solve( b, x );
    return;
}
template < class ELT >
void FactorSVD::solve(  const Matrix_<ELT>& b, Matrix_<ELT>& x ) {
    rep->solve(  b, x );
    return;
}

template <typename ELT>
void FactorSVD::inverse( Matrix_<ELT>& inverse ) {
    rep->inverse( inverse );
}
template < class ELT >
FactorSVD::FactorSVD( const Matrix_<ELT>& m ) {
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, (typename CNT<ELT>::TReal)DefaultRecpCondition);
}
template < class ELT >
FactorSVD::FactorSVD( const Matrix_<ELT>& m, double rcond ) {
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, rcond);
}
template < class ELT >
FactorSVD::FactorSVD( const Matrix_<ELT>& m, float rcond ) {
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, rcond);
}

template < class ELT >
void FactorSVD::factor( const Matrix_<ELT>& m ) {
    delete rep;
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, (typename CNT<ELT>::TReal)DefaultRecpCondition); 
}

template < class ELT >
void FactorSVD::factor( const Matrix_<ELT>& m, double rcond ){
    delete rep;
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, rcond );
}
template < class ELT >
void FactorSVD::factor( const Matrix_<ELT>& m, float rcond ){
    delete rep;
    rep = new FactorSVDRep<typename CNT<ELT>::StdNumber>(m, rcond );
}

template <class T> 
void FactorSVD::getSingularValuesAndVectors( Vector_<typename CNT<T>::TReal >& values, Matrix_<T>& leftVectors, Matrix_<T>& rightVectors) {

    rep->getSingularValuesAndVectors( values, leftVectors, rightVectors );

    return;
}

template <class T> 
void FactorSVD::getSingularValues( Vector_<T>& values ) {
    rep->getSingularValues( values );
    return;
}
//////////////////
   // FactorSVDRep //
   //////////////////
template <typename T >        // constructor 
    template < typename ELT >
FactorSVDRep<T>::FactorSVDRep( const Matrix_<ELT>& mat, typename CNT<T>::TReal rc):
    nCol(mat.ncol()),  
    nRow(mat.nrow()),
    mn( (mat.nrow() < mat.ncol()) ? mat.nrow() : mat.ncol() ), 
    maxmn( (mat.nrow() > mat.ncol()) ? mat.nrow() : mat.ncol() ), 
    singularValues(mn),
    inputMatrix(nCol*nCol),
    rank(0),
    structure(mat.getMatrixStructure())  {
    
    LapackInterface::getMachineUnderflow( abstol );
    abstol *= 0.5;
     rcond = rc;

    LapackConvert::convertMatrixToLapack( inputMatrix.data, mat );
    isFactored = true;
        
}
template <typename T >
int FactorSVDRep<T>::getRank() {

    if( rank == 0 ) {   // check if SVD has been done
        Vector_<RType> v;    
        getSingularValues( v );
    }
    return( rank );
}
template < class T >
void FactorSVDRep<T>::inverse(  Matrix_<T>& inverse ) {

    Matrix_<T> iden(mn,mn);
    inverse.resize(mn,mn);
    iden = 1.0;
    solve( iden, inverse );
 
}

template <typename T >
FactorSVDRep<T>::~FactorSVDRep() {}

template <typename T >
FactorSVDRepBase* FactorSVDRep<T>::clone() const {
   return( new FactorSVDRep<T>(*this) );
}

template < class T >
void FactorSVDRep<T>::solve( const Vector_<T>& b, Vector_<T> &x ) {

    SimTK_APIARGCHECK_ALWAYS(isFactored ,"FactorSVD","solve",
       "No matrix was passed to FactorSVD. \n"  );

    SimTK_APIARGCHECK2_ALWAYS(b.size()==nRow,"FactorSVD","solve",
       "number of rows in right hand side=%d does not match number of rows in original matrix=%d \n",
        b.size(), nRow );

    Matrix_<T> m(maxmn,1);

    for(int i=0;i<b.size();i++) {
        m(i,0) = b(i);
    }
    Matrix_<T> r(nCol, 1 );
    doSolve( m, r );
    x.copyAssign(r);
    return;

}

template < class T >
void FactorSVDRep<T>::solve( const Matrix_<T>& b, Matrix_<T> &x ) {

    SimTK_APIARGCHECK_ALWAYS(isFactored ,"FactorSVD","solve",
       "No matrix was passed to FactorSVD. \n"  );

    SimTK_APIARGCHECK2_ALWAYS(b.nrow()==nRow,"FactorSVD","solve",
       "number of rows in right hand side=%d does not match number of rows in original matrix=%d \n",
        b.size(), nRow );

    x.resize( nCol, b.ncol() );
    Matrix_<T> tb;
    tb.resize(maxmn, b.ncol() );
    for(int j=0;j<b.ncol();j++) for(int i=0;i<b.nrow();i++) tb(i,j) = b(i,j);
    doSolve(tb, x);


}

template <typename T >
void FactorSVDRep<T>::doSolve(  Matrix_<T>& b, Matrix_<T>& x) {
    int i,j;
    int info;
    typedef typename CNT<T>::TReal RealType;

    TypedWorkSpace<T> tempMatrix = inputMatrix;

    x.resize(nCol, b.ncol() );
    Matrix_<T> tb;
    tb.resize(maxmn, b.ncol() );
    for(j=0;j<b.ncol();j++) for(i=0;i<b.nrow();i++) tb(i,j) = b(i,j);

    LapackInterface::gelss<T>( nRow, nCol, mn, b.ncol(), tempMatrix.data, nRow, &tb(0,0), 
                      tb.nrow(), singularValues.data, rcond, rank, info  );

    if( info > 0 ) {
        SimTK_THROW2( SimTK::Exception::ConvergedFailed,
        "FactorSVD::solve", 
        "divide and conquer singular value decomposition" );
    }
    
    for(j=0;j<b.ncol();j++) for(i=0;i<nCol;i++) x(i,j) = tb(i,j);

}

template < class T >
void FactorSVDRep<T>::getSingularValuesAndVectors( Vector_<RType>& values,  Matrix_<T>& leftVectors, Matrix_<T>& rightVectors ) {
   

    leftVectors.resize(nRow,nRow);
    rightVectors.resize(nCol,nCol);
    values.resize(mn);

    computeSVD( true, &values(0), &leftVectors(0,0), &rightVectors(0,0) );

    return;
}
template < class T >
void FactorSVDRep<T>::getSingularValues( Vector_<RType>& values ) {

    values.resize(mn);

    computeSVD( false, &values(0), NULL, NULL );

    return;
}
template < class T >
void FactorSVDRep<T>::computeSVD( bool computeVectors, RType* values, T* leftVectors, T* rightVectors ) {

    int info;
    char jobz;

    if( computeVectors ) {
        jobz = 'A';
    } else {
        jobz = 'N';
    }

    TypedWorkSpace<T> tempMatrix = inputMatrix;
    LapackInterface::gesdd<T>(jobz, nRow,nCol,tempMatrix.data, nRow, values,
           leftVectors, nRow, rightVectors, nCol, info);

    for(int i=0, rank=0;i<mn;i++) {
        if( values[i] > rcond*values[0] ) rank++;
    } 

    if( info > 0 ) {
        SimTK_THROW2( SimTK::Exception::ConvergedFailed,
        "FactorSVD::getSingularValuesAndVectors", 
        "divide and conquer singular value decomposition" );
    }

    return;
}

// instantiate
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<double>& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<float>& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<std::complex<float> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<std::complex<double> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<conjugate<float> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<conjugate<double> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< double> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< float> >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< std::complex<float> > >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< std::complex<double> > >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< conjugate<float> > >& m );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< conjugate<double> > >& m );

template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<double>& m, double rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<float>& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<std::complex<float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<std::complex<double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<conjugate<float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<conjugate<double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< std::complex<float> > >& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< std::complex<double> > >& m, double rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< conjugate<float> > >& m, float rcond );
template SimTK_SIMMATH_EXPORT FactorSVD::FactorSVD( const Matrix_<negator< conjugate<double> > >& m, double rcond );

template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValues<float >(Vector_<float>&  );
template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValues<double >(Vector_<double>&  );

template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValuesAndVectors<float >(Vector_<float>&, Matrix_<float>&, Matrix_<float>&  );
template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValuesAndVectors<double >(Vector_<double>&, Matrix_<double>&, Matrix_<double>&  );
template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValuesAndVectors<std::complex<float> >(Vector_<float>&, Matrix_<std::complex<float> >&, Matrix_<std::complex<float> >&  );
template SimTK_SIMMATH_EXPORT void FactorSVD::getSingularValuesAndVectors<std::complex<double> >(Vector_<double>&, Matrix_<std::complex<double> >&, Matrix_<std::complex<double> >&  );

template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<double>& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<float>& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<std::complex<float> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<std::complex<double> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<conjugate<float> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<conjugate<double> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< double> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< float> >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< std::complex<float> > >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< std::complex<double> > >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< conjugate<float> > >& m );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< conjugate<double> > >& m );

template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<double>& m, double rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<float>& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<std::complex<float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<std::complex<double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<conjugate<float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<conjugate<double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< double> >& m, double rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< float> >& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< std::complex<float> > >& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< std::complex<double> > >& m, double rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< conjugate<float> > >& m, float rcond );
template SimTK_SIMMATH_EXPORT void FactorSVD::factor( const Matrix_<negator< conjugate<double> > >& m, double rcond );

template SimTK_SIMMATH_EXPORT void FactorSVD::inverse<float>(Matrix_<float>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::inverse<double>(Matrix_<double>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::inverse<std::complex<float> >(Matrix_<std::complex<float> >&);
template SimTK_SIMMATH_EXPORT void FactorSVD::inverse<std::complex<double> >(Matrix_<std::complex<double> >&);

template SimTK_SIMMATH_EXPORT void FactorSVD::solve<float>(const Vector_<float>&, Vector_<float>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<double>(const Vector_<double>&, Vector_<double>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<std::complex<float> >(const Vector_<std::complex<float> >&, Vector_<std::complex<float> >&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<std::complex<double> >(const Vector_<std::complex<double> >&, Vector_<std::complex<double> >&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<float>(const Matrix_<float>&, Matrix_<float>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<double>(const Matrix_<double>&, Matrix_<double>&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<std::complex<float> >(const Matrix_<std::complex<float> >&, Matrix_<std::complex<float> >&);
template SimTK_SIMMATH_EXPORT void FactorSVD::solve<std::complex<double> >(const Matrix_<std::complex<double> >&, Matrix_<std::complex<double> >&);

template class FactorSVDRep<double>;
template FactorSVDRep<double>::FactorSVDRep( const Matrix_<double>& m, double rcond);
template FactorSVDRep<double>::FactorSVDRep( const Matrix_<negator<double> >& m, double rcond);

template class FactorSVDRep<float>;
template FactorSVDRep<float>::FactorSVDRep( const Matrix_<float>& m, float rcond );
template FactorSVDRep<float>::FactorSVDRep( const Matrix_<negator<float> >& m, float rcond );

template class FactorSVDRep<std::complex<double> >;
template FactorSVDRep<std::complex<double> >::FactorSVDRep( const Matrix_<std::complex<double> >& m, double rcond );
template FactorSVDRep<std::complex<double> >::FactorSVDRep( const Matrix_<negator<std::complex<double> > >& m, double rcond );
template FactorSVDRep<std::complex<double> >::FactorSVDRep( const Matrix_<conjugate<double> >& m, double rcond );
template FactorSVDRep<std::complex<double> >::FactorSVDRep( const Matrix_<negator<conjugate<double> > >& m, double rcond );

template class FactorSVDRep<std::complex<float> >;
template FactorSVDRep<std::complex<float> >::FactorSVDRep( const Matrix_<std::complex<float> >& m, float rcond );
template FactorSVDRep<std::complex<float> >::FactorSVDRep( const Matrix_<negator<std::complex<float> > >& m, float rcond );
template FactorSVDRep<std::complex<float> >::FactorSVDRep( const Matrix_<conjugate<float> >& m, float rcond );
template FactorSVDRep<std::complex<float> >::FactorSVDRep( const Matrix_<negator<conjugate<float> > >& m, float rcond );

} // namespace SimTK