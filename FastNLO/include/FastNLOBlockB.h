// Author: Daniel Britzger
// DESY, 23/07/2011

//  Version 0.3, 
//
//  History:
//    Version 0, initial version


#ifndef FASTNLOBLOCKB
#define FASTNLOBLOCKB

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//  Data storage class for 'BlockB'-variables                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include <string>
#include <iostream>
#include <cstdio>
#include <vector>

using namespace std;

class FastNLOBlockB { 

private:
   void ResizeTable( vector<double >* v, int dim0 );
   void ResizeTable( vector<vector<double > >*  v, int dim0 , int dim1 );
   void ResizeTable( vector<vector<double > >*  v, int dim0 , int* dim1GetNxmaxFromDimI );
   void ResizeTable( vector<vector<vector<double > > >* v, int dim0 , int* dim1GetNxmaxFromDimI, int dim2 );
   void ResizeTable( vector<vector<vector<double > > >* v, int dim0 , int dim1, int dim2 );
   void ResizeTable( vector<vector<vector<vector<double > > > >* v, int dim0 , int dim1, int dim2, int dim3 );
   void ResizeTable( vector<vector<vector<vector<double > > > >* v, int dim0 , int dim1, int* dim2GetNxmaxFromDimI, int dim3 );
   void ResizeTable( vector<vector<vector<vector<vector<double > > > > >* v, int dim0 , int dim1, int dim2, int dim3, int dim4 );
   void ResizeTable( vector<vector<vector<vector<vector<double > > > > >* v, int dim0 , int dim1, int dim2, int* dim3GetNxmaxFromDimI, int dim4 );
   void ResizeTable( vector<vector<vector<vector<vector<double > > > > >* v, int dim0 , int* dim1GetNxmaxFromDimI, int dim2, int dim3, int dim4 );
   void ResizeTable( vector<vector<vector<vector<vector<vector<double > > > > > >* v, int dim0 , int dim1, int dim2, int dim3, int dim4, int dim5 );
   void ResizeTable( vector<vector<vector<vector<vector<vector<vector<double > > > > > > >* v, int dim0 , int dim1, int dim2, int dim3, int dim4, int dim5, int dim6 );
   void ResizeTable( vector<vector<vector<vector<vector<vector<vector<double > > > > > > >* v, int dim0 , int dim1, int dim2, int dim3, int dim4, int* dim5GetNxmaxFromDimI , int dim6 );

   int ReadFlexibleVector( vector<double >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<double > >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<vector<double > > >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<vector<vector<double > > > >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<vector<vector<vector<double > > > > >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<vector<vector<vector<vector<double > > > > > >* v, istream *table , bool nProcLast = false );
   int ReadFlexibleVector( vector<vector<vector<vector<vector<vector<vector<double > > > > > > >* v, istream *table , bool nProcLast = false );


   void ResizeFlexibleVector(vector<vector<vector<vector<vector<vector<vector<double > > > > > > >* v, vector<vector<vector<vector<vector<vector<vector<double > > > > > > >*nom );
   void ResizeFlexibleVector(vector<vector<vector<vector<vector<vector<double > > > > > >* v, vector<vector<vector<vector<vector<vector<double > > > > > >*nom );
   void ResizeFlexibleVector(vector<vector<vector<vector<vector<double > > > > >* v, vector<vector<vector<vector<vector<double > > > > >*nom );
   void ResizeFlexibleVector(vector<vector<vector<vector<double > > > >* v, vector<vector<vector<vector<double > > > >*nom );
   void ResizeFlexibleVector(vector<vector<vector<double > > >* v, vector<vector<vector<double > > >*nom );
   void ResizeFlexibleVector(vector<vector<double > >* v, vector<vector<double > >*nom );
   void ResizeFlexibleVector(vector<double >* v, vector<double >*nom );


   int ReadTable( vector<vector<vector<vector<vector<vector<vector<double > > > > > > >* v, istream *table );
   int ReadTable( vector<vector<vector<vector<vector<vector<double > > > > > >* v, istream *table );
   int ReadTable( vector<vector<vector<vector<vector<double > > > > >* v, istream *table );
   int ReadTable( vector<vector<vector<vector<double > > > >* v, istream *table );
   int ReadTable( vector<vector<vector<double > > >* v, istream *table );
   int ReadTable( vector<vector<double > >* v, istream *table );
   int ReadTable( vector<double >* v, istream *table );

 
public:
   int GetNxmax(int i);
   int GetTotalScalevars();
   int GetTotalScalenodes();
   void StripWhitespace(string* s);
  
public:

   char* fname;
   int   fNObsBins;
  
   // ---- Block B ---- //
   static const int DividebyNevt = 1;

   int IXsectUnits;
   int IDataFlag;
   int IAddMultFlag;
   int IContrFlag1;
   int IContrFlag2;
   int IContrFlag3;
   int NScaleDep;  

   vector < string > CtrbDescript;
   vector < string > CodeDescript;

   // ---- IDataFlag==1 and IAddMultFlag==1 ---- //
   int Nuncorrel;
   vector < string > UncDescr;
   int Ncorrel;
   vector < string > CorDescr;
   vector < double > Xcenter;
   vector < double > Value;
   vector < vector < double > > UncorLo;
   vector < vector < double > > UncorHi;
   vector < vector < double > > CorrLo;
   vector < vector < double > > CorrHi;
   int NErrMatrix;
   vector < vector < double > > matrixelement;
   vector < double > fact;

   // 'normal' FastNLO vars
   int IRef;
   int IScaleDep;
   unsigned long long int Nevt;
   int Npow;
   int NPDF;
   vector < int > NPDFPDG;
   int NPDFDim;
   int NFragFunc;
   vector < int > NFFPDG;
   int NFFDim;
   int NSubproc;
   int IPDFdef1;
   int IPDFdef2;
   int IPDFdef3;
   // Missing: linear PDF combinations for IPDFdef1=0
   vector < int > Nxtot1;
   vector < double > Hxlim1;
   vector < vector < double > > XNode1;
   vector < int >  Nxtot2;
   vector < double > Hxlim2;
   vector < vector < double > > XNode2;
   vector < int > Nztot;
   vector < double > Hzlim;
   vector < vector < double > > ZNode;
   int NScales;
   int NScaleDim;
   vector < int > Iscale;
   vector < vector < string > > ScaleDescript;
   vector < int > Nscalevar;
   vector < int > Nscalenode;
   vector < vector < double > > ScaleFac;
   vector < double > ScaleFactorsScale1;
   vector < double > ScaleFactorsScale2;
   vector < vector < vector < vector < double > > > > ScaleNode;
   vector < vector < vector < double > > > Scale1Node;
   vector < vector < vector < double > > > Scale2Node;

   // --------------------------- mu_f, mu_r variaton scheme --------------------------- //
   vector < vector < vector < vector < vector < double > > > > > SigmaTildeMuIndep;
   vector < vector < vector < vector < vector < double > > > > > SigmaTildeMuFDep;
   vector < vector < vector < vector < vector < double > > > > > SigmaTildeMuRDep;
   vector < vector < double > > SigmaRefMixed;
   vector < vector < double > > SigmaRef_s2;
   vector < vector < double > > SigmaRef_s1;

   vector < vector < double > > ScaleNodeScale1;
   vector < vector < double > > ScaleNodeScale2;

   // ---- stuff for reading the table ---- //
   vector < double > XsectionRef_s1;
   vector < double > XsectionRefMixed;
   vector < double > XsectionRef_s2;
   vector < vector < vector < vector < vector <double > > > > > PdfLcMuVar;
   vector < vector < vector < double > > > AlphasTwoPi;
   vector < vector < vector < vector < vector < double > > > > > SigmaTilde;
   vector < vector < vector < vector < double > > > > PdfLc;
   vector < vector < double > > AlphasTwoPi_v20;


protected:

   static const int tablemagicno	= 1234567890;
  
  
public:

   void ReadBlockB(istream *table);
   void Print(const int ic = 0, const int iprint = 0);
   void SetName(const char* name) { fname = (char*) name;};
   void FillPDFCache();
  
public:
  
   FastNLOBlockB(const char* name, const int NObsBins );
   FastNLOBlockB(const char* name, const int NObsBins , istream* table );
   ~FastNLOBlockB();

};

#endif
