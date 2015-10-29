#include <cstdlib>
#include <string>
#include <iostream>
#include <cmath>
#include "fastnlotk/fastNLOTools.h"

using namespace std;
using namespace say;
using namespace fastNLO;

namespace fastNLOTools {

   //________________________________________________________________________________________________________________ //
   int ReadVector(vector<double >& v, istream& table , double nevts ){
      //! Read values according to the size() of the given vector
      //! from table (v2.0 format).
      for( unsigned int i=0 ; i<v.size() ; i++){
         // KR: Add check on inf and NaN
         //         table >> v[i];
         //         v[i] *= nevts;
         char buffer[256];
         table >> buffer;
         double value = atof(buffer);
         if ( isfinite(value) ) {
            v[i]  = value;
            v[i] *= nevts;
         } else {
            error["ReadVector"]<<"Non-finite number read from table, aborted! value = " << value << endl;
            error["ReadVector"]<<"Please check the table content." << endl;
            exit(1);
         }
      }
      return v.size();
   }


   //________________________________________________________________________________________________________________ //
   int ReadFlexibleVector(vector<double >& v, istream& table , int nProcLast , double nevts ){
      int nn = 0;
      if ( nProcLast == 0 ) {
         int size = 0;
         table >> size; nn++;
         v.resize(size);
      }
      else {
         v.resize(nProcLast);
      }
      for(unsigned int i0=0;i0<v.size();i0++){
         // KR: Add check on inf and NaN
         //         table >> v[i0];
         //         v[i0] *= nevts;
         char buffer[256];
         table >> buffer;
         double value = atof(buffer);
         if ( isfinite(value) ) {
            v[i0]  = value;
            v[i0] *= nevts;
            nn++;
         } else {
            error["ReadFlexibleVector"]<<"Non-finite number read from table, aborted! value = " << value << endl;
            error["ReadFlexibleVector"]<<"Please check the table content." << endl;
            exit(1);
         }
      }
      return nn;
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v7d& v, int dim0 , int dim1, int dim2, int dim3, int dim4, int dim5, int dim6 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1, dim2, dim3, dim4, dim5, dim6 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v6d& v, int dim0 , int dim1, int dim2, int dim3, int dim4, int dim5 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1, dim2, dim3, dim4, dim5 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }




   //________________________________________________________________________________________________________________ //
   void ResizeVector( v5d& v, int dim0 , int dim1, int dim2, int dim3, int dim4 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1, dim2, dim3, dim4 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v4d& v, int dim0 , int dim1, int dim2, int dim3 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1, dim2, dim3 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v3d& v, int dim0 , int dim1, int dim2 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1, dim2 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v2d&  v, int dim0 , int dim1 ){
      if ( dim0 > 0 ){
         v.resize(dim0);
         for ( int i= 0 ; i<dim0 ; i++)
            ResizeVector( v[i] , dim1 );
      } else {
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }


   //________________________________________________________________________________________________________________ //
   void ResizeVector( v1d& v, int dim0 ){
      if ( dim0 > 0 )
         v.resize(dim0);
      else{
         error["fastNLOTools::ResizeVector"]<<"Cannot resize table, because dimension is <= zero (dim0="<<dim0<<"). Exiting"<<endl;
         exit(1);
      }
   }



   //______________________________________________________________________________
   void ResizeFlexibleVector(vector<double >& v, const vector<double >& nom) {
      v.resize(nom.size());
   }


   //______________________________________________________________________________
   int WriteVector( const vector<string >& v, ostream& table , double nevts ) {
      if ( nevts != 0 ) {
         error["fastNLOTools::WriteVector"]<<"Cannot scale a string table by nevts (nevts="<<nevts<<")."<<endl;
         return -1000;
      }
      else return _Write1DVector(v,table);
   }
   //______________________________________________________________________________
   int WriteVector( const vector<double >& v, ostream& table , double nevts ) {
      return _Write1DVectorByN(v,table,nevts);
   }
   //______________________________________________________________________________
   int WriteVector( const vector<int >& v, ostream& table , double nevts ) {
      return _Write1DVectorByN(v,table,nevts);
   }
   //______________________________________________________________________________
   int WriteVector( const vector<unsigned long long >& v, ostream& table , double nevts ) {
      return _Write1DVectorByN(v,table,nevts);
   }


   //______________________________________________________________________________
   void AddVectors( vector<double >& vSum, const vector<double >& vAdd, double w1, double w2 ) {
      _DoAddVectors(vSum,vAdd,w1,w2);
   }
   //______________________________________________________________________________
   void AddVectors( vector<int >& vSum, const vector<int >& vAdd, double w1, double w2 ) {
      _DoAddVectors(vSum,vAdd,w1,w2);
   }
   //______________________________________________________________________________
   void AddVectors( vector<unsigned long long >& vSum, const vector<unsigned long long >& vAdd, double w1, double w2 ) {
      _DoAddVectors(vSum,vAdd,w1,w2);
   }


   //______________________________________________________________________________
   int WriteFlexibleVector(const vector<double >& v, ostream& table, int nProcLast, double nevts) {
      //! Write 1-dimensional flexible table to disk
      //! nevts: Divide all values by nevts
      //! nProcLast: Specify, if the size of the vector should be written in the first line.
      //! if nProcLast != 0, skip the first line
      if ( nevts == 0 ) {
         error["fastNLOTools::WriteFlexibleVector"]<<"Cannot divide by zero. nProcLast ="<<nProcLast<<endl;
         return -1000;
      }
      if ( nProcLast == 0 )
         table << v.size() << endl;
      if ( nProcLast != 0 && nProcLast != (int)v.size() )
         warn["fastNLOTools::WriteFlexibleVector(double)"]
            <<"Dimension of this vector is not compatible with its size (i.e. nProclast ="<<nProcLast<<", v.size()="<<v.size()<<endl;
      int n = _Write1DVectorByN(v,table,nevts);
      return ( nProcLast == 0 ) ? n+1 : n;
   }


   //______________________________________________________________________________
   int WriteFlexibleVector(const vector<string >& v, ostream& table, int nProcLast, double nevts) {
      //! Write 1-dimensional flexible table to disk
      //! nevts: ignoring nevts !!
      //! nProcLast: Specify, if the size of the vector should be written in the first line.
      //! if nProcLast != 0, skip the fist line
      if ( nevts!= 1 ) warn["fastNLOTools::WriteFlexibleVector(string)"]
         <<"String variable cannot be divided by integer number! Ignoring nevts="<<nevts<<endl;
      if ( nProcLast == 0 )
         table << v.size() << endl;
      if ( nProcLast != 0 && nProcLast != (int)v.size() )
         warn["fastNLOTools::WriteFlexibleVector(string)"]
            <<"Dimension of this vector is not compatible with its size (i.e. nProclast ="<<nProcLast<<", v.size()="<<v.size()<<endl;
      int n = _Write1DVector(v,table);
      return ( nProcLast == 0 ) ? n+1 : n;
   }

   //______________________________________________________________________________
   int WriteFlexibleVector(const vector<unsigned long long >& v, ostream& table, int nProcLast, double nevts) {
      //! Write 1-dimensional flexible table to disk
      //! nevts: ignoring nevts !!
      //! nProcLast: Specify, if the size of the vector should be written in the first line.
      //! if nProcLast != 0, skip the fist line
      if ( nevts!= 1 ) warn["fastNLOTools::WriteFlexibleVector(unsigned long long)"]
         <<"String variable cannot be divided by integer number! Ignoring nevts="<<nevts<<endl;
      if ( nProcLast == 0 )
         table << v.size() << endl;
      if ( nProcLast != 0 && nProcLast != (int)v.size() )
         warn["fastNLOTools::WriteFlexibleVector(string)"]
            <<"Dimension of this vector is not compatible with its size (i.e. nProclast ="<<nProcLast<<", v.size()="<<v.size()<<endl;
      int n = _Write1DVector(v,table);
      return ( nProcLast == 0 ) ? n+1 : n;
   }

   //______________________________________________________________________________
   int WriteFlexibleVector(const vector<int >& v, ostream& table, int nProcLast, double nevts) {
      //! Write 1-dimensional flexible table to disk
      //! nevts: ignoring nevts !!
      //! nProcLast: Specify, if the size of the vector should be written in the first line.
      //! if nProcLast != 0, skip the fist line
      if ( nevts!= 1 ) warn["fastNLOTools::WriteFlexibleVector(int)"]
         <<"Refusing dividing integer numbers by each other! Ignoring nevts="<<nevts<<endl;
      if ( nProcLast == 0 )
         table << v.size() << endl;
      if ( nProcLast != 0 && nProcLast != (int)v.size() )
         warn["fastNLOTools::WriteFlexibleVector(int)"]
            <<"Dimension of this vector is not compatible with its size (i.e. nProclast ="<<nProcLast<<", v.size()="<<v.size()<<endl;
      int n = _Write1DVector(v,table);
      return ( nProcLast == 0 ) ? n+1 : n;
   }

   //________________________________________________________________________________________________________________ //
   void StripWhitespace(string& str) {
      //! remove white spaces from string
      for(string::iterator achar = str.end(); achar>str.begin();achar--) {
         if (*achar==0x20 || *achar==0x00){
            str.erase(achar);
         }else{
            break;
         }
      }
   }

   //________________________________________________________________________________________________________________ //
   void PutBackMagicNo(istream& table){
   //! Put magic number back
      for(int i=0;i<(int)(log10((double)tablemagicno)+1);i++){
         table.unget();
      }
   }


   //______________________________________________________________________________
   bool ReadMagicNo(istream& table) {
      //! read and crosscheck magic number
      int key = 0;
      table >> key;
      if(key != tablemagicno){
         error["ReadMagicNo"]<<"Found "<<key<<" instead of "<<tablemagicno<<"."<<endl;
         return false;
      };
      return true;
   }


} // end namespace fastNLO
