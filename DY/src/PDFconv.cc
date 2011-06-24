#include <algorithm>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>

#include "PhysPar.h"
#include "PDFconv.h"

using namespace std;

extern "C"{
void fpdfxq_(int *iset, const double *x, const double *q2, double *pdfs, 
            int *ichk);
void fastini_(const double *xint, const double *q2int, int *n, int *ichk);
void fastsns_(const int *iset, const double *pdfin, const int *isel,
              const int *ibuf);
void fastfxq_(const int *ibuf, const double *xq, const int *nint);
}

const int PDFconv::_nfl = 13;
PDFconv::PDFconv(const int chg_prod, const double * beam_en,
         const IntSteps *ist):IntSteps(*ist){

  _chg_prod = chg_prod;
  _beam_en = *beam_en;
  // initialuze fdef matrix
  // numbering:
//\bar{t   b   c   s   u   d}  g   d   u   s   c   b   t
//     0   1   2   3   4   5   6   7   8   9  10  11  12
  const double fdef[_nfl*_nfl] = {
       1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., // tb
       0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., // bb
       0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., // cb
       0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., // sb
       0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., // ub
       0., 0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., // db
       0., 0., 0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., // g
       0., 0., 0., 0., 0., 0., 0., 1., 0., 0., 0., 0., 0., // d
       0., 0., 0., 0., 0., 0., 0., 0., 1., 0., 0., 0., 0., // u
       0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0., 0., 0., // s
       0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0., 0., // c
       0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0., // b
       0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1.  // t
  };

  _fdef = new double[_nfl*_nfl];
  for (int ifl2=0; ifl2<_nfl*_nfl; ifl2++){
    _fdef[ifl2] = fdef[ifl2];
  }

  getPDFconv=NULL;
  if ( string("W") == _boz ){
    getPDFconv=&PDFconv::getPDFconvW;
  } else if ( string("Z") == _boz ){
    getPDFconv=&PDFconv::getPDFconvZ;
  }

  this->init();
}

PDFconv::~PDFconv()
{
  delete[] _fdef;

  for (int imp=0; imp<2*_nms+1; imp++){
    delete[] _IDX[imp];
  }
  delete[] _IDX;

  delete[] _XINT;
  delete[] _Q2INT;

  for (int ifl = 0; ifl < 13; ifl++){
    delete[] _XFINT[ifl];
  }
  delete [] _XFINT;
}

// Initialise grids and arrays
void PDFconv::init(){
  // create interpolation point index array
  int nmp=2*_nms+1, nyp=2*_nys+1;
  _IDX = new int*[nmp];
  for (int imp=0; imp<nmp; imp++){
    _IDX[imp] = new int[2*nyp];
    for (int iyp=0; iyp<2*nyp; iyp++) _IDX[imp][iyp] = 0;
  }

  // fill it with indices
  vector<double> vintx;
  for (int imp=0; imp<nmp; imp++){
    double m = (_msteps[imp/2]+_msteps[(imp+1)/2])/2.;
    for(int iyp=0;iyp<nyp;iyp++){
      double y = (_ysteps[iyp/2]+_ysteps[(iyp+1)/2])/2.;

      double x1(0.), x2(0.);
      x1 = m/_beam_en*exp(y);
      x2 = m/_beam_en*exp(-y);
      if ( x1<=0. || x1 >= 1. || x2<=0. || x2 >= 1.) continue;

      vector<double>::iterator xpos = find(vintx.begin(),vintx.end(),x1);
      if ( xpos!=vintx.end() ){
        _IDX[imp][iyp] = int(xpos-vintx.begin());
      } else {
        vintx.push_back(x1);
        _IDX[imp][iyp] = vintx.size()-1;
      }
      xpos = find(vintx.begin(),vintx.end(),x2);
      if ( xpos!=vintx.end() ){
        _IDX[imp][nyp+iyp] = int(xpos-vintx.begin());
      } else {
        vintx.push_back(x2);
        _IDX[imp][nyp+iyp] = vintx.size()-1;
      }
    }
  }

  // create interpolation grid
  _NINT = vintx.size();
  _XINT = new double[_NINT];
  _Q2INT = new double[_NINT];
  int ix(0);
  double scale2(0);
  if ( string("Z") == _boz ) {
    scale2 = PhysPar::m2z;
  } else if ( string("W") == _boz ) {
    scale2 = PhysPar::m2w;
  } else {
    cout << "Unknown bozon type: " << _boz << endl;
    exit(1);
  }

  // assing values to it
  vector<double>::iterator it = vintx.begin();
  for(;it!=vintx.end();it++){
    _XINT[ix] = *it;
    _Q2INT[ix] = scale2;
    ix++;
  }

  // create interpolated functions array
  _XFINT = new double*[13];
  for (int ifl = 0; ifl < 13; ifl++){
    _XFINT[ifl] = new double[_NINT];
    for ( int iint=0; iint<_NINT; iint++) _XFINT[ifl][iint] = 0.;
  }

}

// Interpolate PDFs at current iteration
int PDFconv::interpPDF(){
  // initialize QCDNUM with our grid
  int ichk(1);
  fastini_(_XINT, _Q2INT, &_NINT, &ichk);
  
  int iset(1), isel(7), ibuf(-1), inbuf(-ibuf);

  for (int ifl=0; ifl<_nfl; ifl++){
   // if ( 6 == ifl ) continue;
    fastsns_(&iset,&_fdef[ifl*_nfl],&isel,&ibuf);
    fastfxq_(&inbuf,_XFINT[ifl],&_NINT);
  }

  return 1;
}

// Calculate PDF convolution for Z production
int PDFconv::getPDFconvZ(const int imp, const int iyp, double dir, 
    const double &scale, double &xfxcD, double &xfxcU)
{
  using namespace PhysPar;

  double m = (_msteps[imp/2]+_msteps[(imp+1)/2])/2.;
  double y = (_ysteps[iyp/2]+_ysteps[(iyp+1)/2])/2.;
  
  double x1 = m/_beam_en*exp(dir*y);
  double x2 = m/_beam_en*exp(-dir*y);

  double xq1[_nfl]={0.};
  double xq2[_nfl]={0.};
  //double sxq1[_nfl]={0.};
  //double sxq2[_nfl]={0.};
  //int   ichk = 0;
  //int   iset = 1;

  //double q2 = (scale) * (scale);

  if ( (x1< 1.e-6)||(x2<1.e-6)||(x1>1.-1.e-6)||(x2>1.-1.e-6)) {
    xfxcD=0.;
    xfxcU=0.;
    return 0;
  }
  else{
  // Access QCDNUM pdfs:
    //fpdfxq_(&iset,&x1,&q2,sxq1,&ichk);
    //fpdfxq_(&iset,&x2,&q2,sxq2,&ichk);
  // Access fast QCDNUM pdfs:
    if ( 0 < dir ){
      for (int ifl=0; ifl<_nfl; ifl++){
        xq1[ifl] = _XFINT[ifl][_IDX[imp][iyp]];
        xq2[ifl] = _XFINT[ifl][_IDX[imp][2*_nys+1+iyp]];
      }
    } else if ( 0 > dir){
      for (int ifl=0; ifl<_nfl; ifl++){
        xq2[ifl] = _XFINT[ifl][_IDX[imp][iyp]];
        xq1[ifl] = _XFINT[ifl][_IDX[imp][2*_nys+1+iyp]];
      }
    }
  }
/*
  for (int ifl=0; ifl<13; ifl++){
    cout << ifl << "\t" << setw(14) << sxq1[ifl] <<  setw(14) <<xq1[ifl] << setw(14) <<sxq1[ifl]-xq1[ifl] << endl;
    cout << ifl << "\t" << setw(14) << sxq2[ifl] <<  setw(14) <<xq2[ifl] << setw(14) <<sxq2[ifl]-xq2[ifl] << endl;
  }*/

  // apply beam composition
  double xqt[_nfl] = {0.};
  if ( 0 > _chg_prod ) {
    if ( 0 < dir ){
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq1[iq];
      for (int iq=0; iq<_nfl; iq++)  xq1[iq]=xqt[_nfl-1-iq];
    } else {
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq2[iq];
      for (int iq=0; iq<_nfl; iq++)  xq2[iq]=xqt[_nfl-1-iq];
    }
  }
  /*
  if ( -1 == ih2 ) {
    if ( 0 < dir ){
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq2[iq];
      for (int iq=0; iq<_nfl; iq++)  xq2[iq]=xqt[_nfl-1-iq];
    } else {
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq1[iq];
      for (int iq=0; iq<_nfl; iq++)  xq1[iq]=xqt[_nfl-1-iq];
    }
  }
  */
  // numbering:
  // \bar{t  b  c  s  u  d} g  d  u  s  c  b  t
  //      0  1  2  3  4  5  6  7  8  9 10 11 12
  xfxcD = 0.; xfxcU = 0.;
  for ( int iflav = 1; iflav<6; iflav++ ) {
    int ifl       = 6+iflav  ;
    int iflbar    = 6-iflav ;
    if ( 1 == iflav%2 ) 
      xfxcD += xq1[ifl]*xq2[iflbar]/(x1*x2);
    else if ( 0 == iflav%2 )
      xfxcU += xq1[ifl]*xq2[iflbar]/(x1*x2);
  }

  return 1;
}


// Calculate PDF convolution for W production
int PDFconv::getPDFconvW(const int imp, const int iyp, double dir, 
    const double &scale, double &xfxcWm, double &xfxcWp)
{
  using namespace PhysPar;

  double m = (_msteps[imp/2]+_msteps[(imp+1)/2])/2.;
  double y = (_ysteps[iyp/2]+_ysteps[(iyp+1)/2])/2.;
  
  double x1 = m/_beam_en*exp(dir*y);
  double x2 = m/_beam_en*exp(-dir*y);

  double xq1[_nfl]={0.};
  double xq2[_nfl]={0.};
  //double sxq1[_nfl]={0.};
  //double sxq2[_nfl]={0.};
  //int   ichk = 0;
  //int   iset = 1;

  //double q2 = (scale) * (scale);

  if ( (x1< 1.e-6)||(x2<1.e-6)||(x1>1.-1.e-6)||(x2>1.-1.e-6)) {
    xfxcWm=0.;
    xfxcWp=0.;
    return 0;
  }
  else{
  // Access QCDNUM pdfs:
    //fpdfxq_(&iset,&x1,&q2,sxq1,&ichk);
    //fpdfxq_(&iset,&x2,&q2,sxq2,&ichk);
  // Access fast QCDNUM pdfs:
    if ( 0 < dir ){
      for (int ifl=0; ifl<_nfl; ifl++){
        xq1[ifl] = _XFINT[ifl][_IDX[imp][iyp]];
        xq2[ifl] = _XFINT[ifl][_IDX[imp][2*_nys+1+iyp]];
      }
    } else if ( 0 > dir){
      for (int ifl=0; ifl<_nfl; ifl++){
        xq2[ifl] = _XFINT[ifl][_IDX[imp][iyp]];
        xq1[ifl] = _XFINT[ifl][_IDX[imp][2*_nys+1+iyp]];
      }
    }
  }
/*
  for (int ifl=0; ifl<13; ifl++){
    cout << ifl << "\t" << setw(14) << sxq1[ifl] <<  setw(14) <<xq1[ifl] << setw(14) <<sxq1[ifl]-xq1[ifl] << endl;
    cout << ifl << "\t" << setw(14) << sxq2[ifl] <<  setw(14) <<xq2[ifl] << setw(14) <<sxq2[ifl]-xq2[ifl] << endl;
  }*/

  // apply beam composition
  double xqt[_nfl] = {0.};
  if ( 0 > _chg_prod ) {
    if ( 0 < dir ){
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq1[iq];
      for (int iq=0; iq<_nfl; iq++)  xq1[iq]=xqt[_nfl-1-iq];
    } else {
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq2[iq];
      for (int iq=0; iq<_nfl; iq++)  xq2[iq]=xqt[_nfl-1-iq];
    }
  }
  /*
  if ( -1 == ih2 ) {
    if ( 0 < dir ){
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq2[iq];
      for (int iq=0; iq<_nfl; iq++)  xq2[iq]=xqt[_nfl-1-iq];
    } else {
      for (int iq=0; iq<_nfl; iq++)  xqt[iq]=xq1[iq];
      for (int iq=0; iq<_nfl; iq++)  xq1[iq]=xqt[_nfl-1-iq];
    }
  }
  */
  // numbering:
  // \bar{t  b  c  s  u  d} g  d  u  s  c  b  t
  //      0  1  2  3  4  5  6  7  8  9 10 11 12
  xfxcWm = 0.; xfxcWp = 0.;
  xfxcWm = (V2[0]*xq1[4]*xq2[7]
          + V2[1]*xq1[2]*xq2[9]
          + V2[2]*xq1[4]*xq2[9]
          + V2[3]*xq1[2]*xq2[7])/(x1*x2)*2.;

  xfxcWp = (V2[0]*xq1[8] *xq2[5]
          + V2[1]*xq1[10]*xq2[3]
          + V2[2]*xq1[8] *xq2[3]
          + V2[3]*xq1[10]*xq2[5])/(x1*x2)*2.;

  return 1;
}
