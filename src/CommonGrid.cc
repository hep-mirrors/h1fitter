/*!
 @file CommonGrid.cc
 @date Tue March 25 2014
 @author Andrey Sapronov <Andrey.Sapronov@cern.ch>

 Contains CommonGrid class member function implementations.
 */

#include <algorithm>
#include <fstream>
#include <list>
#include <sstream>
#include <float.h>

#include "CommonGrid.h"

#include "appl_grid/appl_grid.h"
#include <FastNLOxFitter.h>

using namespace std;
using namespace appl;

// external pdf functions
extern "C" void appl_fnpdf_(const double& x, const double& Q, double* f);
extern "C" void appl_fnpdf_bar_(const double& x, const double& Q, double* f);
extern "C" double appl_fnalphas_(const double& Q);
extern "C" void hf_errlog_(const int &id, const char *text, int); 

/*
void appl_fnpdf_bar(const double& x, const double& Q, double* f)
{
  appl_fnpdf_( x, Q, f);    
  double fbar[13];
  for (int fi = 0; fi < 13; fi++)
    fbar[fi] = f[12-fi];
  for (int fi = 0; fi < 13; fi++)
    f[fi] = fbar[fi];
  return; 
}
*/

CommonGrid::CommonGrid(const string & grid_type, const string &grid_source): _dynamicscale(0)
{
  if ( grid_type == "applgrid" ) { 
    _flag = 0;
    this->readAPPLgrid(grid_source);
  } else if ( grid_type == "applgrid_norm" ) { 
    _flag = 1;
    this->readAPPLgrid(grid_source);
  } else if ( grid_type == "virtgrid" ) { 
    _flag = 2; 
    this->readVirtGrid(grid_source);
  } else if ( grid_type == "virtgrid_norm" ) { 
    _flag = 3; 
    this->readVirtGrid(grid_source);
  } else if ( grid_type == "applgrid_normhyperbin" ) { 
    _flag = 4;
    this->readAPPLgrid(grid_source);
  } else if ( grid_type == "virtgrid_normhyperbin" ) { 
    _flag = 5;
    this->readVirtGrid(grid_source);
  } else if ( grid_type.find("ast") != string::npos ) { // fastNLO
     this->initfastNLO(grid_source);
  } else {
    int id = 14032542;
    char text[] = "S: Unknown grid type in theory expression.";
    int textlen = strlen(text);
    hf_errlog_(id, text, textlen);
  }
}

CommonGrid::~CommonGrid(){
  vector<tHyperBin>::iterator ihb;
  for (ihb = _hbins.begin(); ihb != _hbins.end(); ihb++){
    delete[] ihb->b;
    if ( ihb->g )  delete ihb->g;
    if ( ihb->f )  delete ihb->f;
  }
}

int
CommonGrid::readAPPLgrid(const string &grid_source)
{
  double *b = new double;
  appl::grid *g = new appl::grid(grid_source);
  if (_dynamicscale != 0)
  {
#ifdef APPLGRID_DYNSCALE
    g->setDynamicScale( _dynamicscale );
#else
    int id = 2204201401;
    char text[] = "S: Cannot use dynamic scale emulation in Applgrid, use v1.4.43 or higher";
    int textlen = strlen(text);
    hf_errlog_(id, text, textlen);
#endif
  }

  g->trim();

  tHyperBin hb;
  hb.b = b;
  hb.g = g;
  hb.f = NULL;
  hb.ngb = g->Nobs();
  _hbins.push_back(hb);
  _ndim = 2;
  return _hbins.size();
}

int
CommonGrid::initfastNLO(const string &grid_source)
{
   FastNLOxFitter* fnlo = new FastNLOxFitter(grid_source);
   
   tHyperBin hb;
   hb.b = NULL;
   hb.g = NULL;
   hb.f = fnlo;
   hb.ngb = fnlo->GetNObsBin();
   _hbins.push_back(hb);
   _ndim = fnlo->GetNumDiffBin(); // _ndim not needed for fastNLO
   for ( int i = 0 ; i<_ndim ; i++ ) 
      if ( fnlo->GetIDiffBin(i) != 1 ) _ndim++;
   return _hbins.size();
}

int
CommonGrid::readVirtGrid(const string &grid_source)
{
  cout << "reading virtual grid from " << grid_source << endl;
  ifstream vg(grid_source.c_str());
  string line;
  if (vg.is_open()){
    while (1) {
      getline(vg,line);
      if (true == vg.eof()) break;
      if (line.at(0) == '#' ) continue; //ignore comments
      line.erase(line.find_last_not_of(" \n\r\t")+1); // trim trailing whitespaces
      stringstream sl(line);
      // first count words
      int nw(0);
      while (sl.good()) {
        string ts;
	sl >> ts;
	nw++;
      }
      if (0!=(nw-2)%2) {
        int id = 14040142;
        char text[] = "S: Bad number of bins in virtual grid. Each bin must have low and high value.";
        int textlen = strlen(text);
        hf_errlog_(id, text, textlen);
      }
      // set the grid dimension
      _ndim = nw;

      // now read bins
      sl.clear();
      sl.seekg(0);
      sl.str(line);
      double *b = new double[nw-2];
      for (int iw=0; iw<nw-2; iw++) sl >> b[iw];
      
      // and grid source
      string ag_source;
      sl>>ag_source;
      int n_grid_bins(0);
      sl>>n_grid_bins;
      appl::grid *g = new  appl::grid(ag_source);
      g->trim();

      // add the hyperbin
      tHyperBin hb;
      hb.b = b;
      hb.g = g;
      hb.ngb = n_grid_bins;
      _hbins.push_back(hb);
    }
    vg.close();
  }
  return _hbins.size();
}

std::vector< std::vector<double> >
CommonGrid::vconvolute(const int iorder, const double mur, const double muf)
{
   // calculate cross sections with fastNLO or applgrid
   vector<tHyperBin>::iterator ihb;
   std::vector< std::vector<double> > result;
   for (ihb = _hbins.begin(); ihb != _hbins.end(); ihb++){
      if ( ihb->g ) 
	 // have to decrement iorder, since for appl_grid 0 -> LO, 1 -> NLO, etc.
	      result.push_back(vconvolute_appl(iorder-1,mur,muf,&(*ihb)));
      else if ( ihb->f )
	      result.push_back(vconvolute_fastnlo(iorder,mur,muf,ihb->f));
      else {
	 int id = 15010262;
	 char text[] = "S: Either applgrid or fastNLO must be present.";
	 int textlen = strlen(text);
	 hf_errlog_(id, text, textlen);	 
      }
   }
   return result;
}

std::vector<double> 
CommonGrid::vconvolute_fastnlo(const int iorder, const double mur, const double muf, FastNLOxFitter* fnlo)
{
   //! calculate fastNLO cross sections
   if ( mur != fnlo->GetScaleFactorMuR() || mur != fnlo->GetScaleFactorMuF()) 
      fnlo->SetScaleFactorsMuRMuF(mur, muf);
   // iorder is already set earlier
   fnlo->FillAlphasCache();
   fnlo->FillPDFCache();
   fnlo->CalcCrossSection();
   return fnlo->GetCrossSection();
}

std::vector<double> 
CommonGrid::vconvolute_appl(const int iorder, const double mur, const double muf, tHyperBin* ihb)
{
  vector<double> xs;
  vector<double> gxs;
  appl::grid *g = ihb->g;
  // extract convoluted cross sections
  if (_ppbar)
     gxs = g->vconvolute(appl_fnpdf_, appl_fnpdf_bar_, appl_fnalphas_, iorder, mur, muf);
  else
     gxs = g->vconvolute(appl_fnpdf_, appl_fnalphas_, iorder, mur, muf);
  // compute virtual bin width if available,
  double bw(1.);
  for (int ibb = 0; ibb<(_ndim-2)/2; ibb++){
     bw *= ihb->b[2*ibb+1] - ihb->b[2*ibb];
  }
  if ( 0!= bw){
     for(vector<double>::iterator ixs = gxs.begin(); ixs!=gxs.end(); ixs++) (*ixs)/=bw;
  } else {
     int id = 14040842;
     char text[] = "S: Bin width for virtual grid is 0, cannot scale.";
     int textlen = strlen(text);
     hf_errlog_(id, text, textlen);
  }

  // scale by hyperbin width if requested
  if(0 != (_flag & 1) && _flag <= 3) 
    for(vector<double>::iterator ixs = gxs.begin(); ixs!=gxs.end(); ixs++)
      (*ixs) *= bw * g->deltaobs(int(ixs - gxs.begin()));
  // scale by bin width if normalization is requested
  else if(_flag > 3) 
    for(vector<double>::iterator ixs = gxs.begin(); ixs!=gxs.end(); ixs++)
      (*ixs) *= bw;

  xs.insert(xs.end(), gxs.begin(), gxs.begin()+ihb->ngb);
  return xs;
}

int
CommonGrid::checkBins(vector<int> &bin_flags, vector<vector<double> > &data_bins)
{
  // do not check bins for normalization grids
  if ( 0 != (_flag & 1) || _flag > 3 ) return 0;

  // compute number of bins in the virtual grid. Compare to the number of data bins
  int n_gbins(0);
  vector<tHyperBin>::iterator ihb;
  for (ihb = _hbins.begin(); ihb!=_hbins.end(); ihb++){
    n_gbins += ihb->ngb;
  }

  // Not checking number of bins, because grids can have wider range
  /*
  if ( n_gbins != data_bins[0].size() ) {
    int id = 14040242;
    char text[] = "S: Number of bins in grid doesn't match for data one.";
    int textlen = strlen(text);
    hf_errlog_(id, text, textlen);
  }
  */
    
  // compare the bins if flag is set
  vector<double> tv;
  vector<vector<double> > gb(data_bins.size(),tv);
  for (ihb = _hbins.begin(); ihb!=_hbins.end(); ihb++){
    appl::grid *g = ihb->g;
    
    for (int igb = 0; igb <ihb->ngb; igb++){  // cycle over applgrid bins
      for (int id = 0; id <_ndim-2; id++ ){ // cycle over hyperbins
        gb.at(id).push_back(ihb->b[id]);
      }
      gb.at(_ndim-2).push_back(g->obslow(igb));  // fill last dimension bins
      gb.at(_ndim-1).push_back(g->obslow(igb+1));  // with applgrid values
    }
  }

  for (int iv = 0; iv<_ndim; iv++){
    for (int ib = 0; ib<data_bins.at(iv).size(); ib++){
      if ( bin_flags.at(ib) == 0 ) continue;
      if ( 0 == (bin_flags.at(ib) & 2) ) {
        bool found_bin = false;
        for (int ibg = 0; ibg < gb[iv].size(); ibg++){
	  //          if (fabs(gb[iv][ibg] - data_bins[data_bins.size()-_ndim+iv][ib]) < 100*DBL_MIN) 
          if (fabs(*(short*)& gb[iv][ibg] - *(short*)& data_bins[data_bins.size()-_ndim+iv][ib]) < 2 ) 
	    found_bin = true;
	}
	if ( !found_bin ){ 
            int id = 14040342;
            char text[] = "S: Data and grid bins don't match.";
            int textlen = strlen(text);
            hf_errlog_(id, text, textlen);
            return -1;
	}
      }
    }
  }
  
  return 0;
}


int 
CommonGrid::setCKM(const vector<double> &v_ckm)
{
  _hbins.at(0).g->setckm(v_ckm);

  return 0;
}
