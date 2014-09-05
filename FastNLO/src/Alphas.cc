#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include "Alphas.h"
#include "speaker.h"

using namespace std;


//______________________________________________________
//
// Class Alphas
//
//  Calculation of alpha_s in the MSbar scheme for given alpha_s(Mz)
//  using exact, iterative solution of 2-/3-/4-loop formulas
//  as used by GRV hep-ph/9806404
//
//  So far, the code is written for nf=5, and no flavor thresholds
//  have been implemented. Therefore the code should be used only
//  for mu_r > m_bottom (which is safe for jet observables)
//
//
// Authors    : D. Britzger, M. Wobisch
// Created    : 2011
//
//_____________________________________________________

double Alphas::fMz              = 91.1876;              // mass of Z0, PDG 2012.
double Alphas::fAlphasMz        = 0.1184;               // alpha_s at starting scale of Mz, PDG 2012; previous was 0.1185, Bethke 2011.
int Alphas::fNf                 = 5;                    // MAXIMUM number of active flavours. e.g. at low scales mu, number of flavors is calculated with respecting flavor thresholds if FlavorMatching is ON.
int Alphas::fnLoop              = 2;                    // n-loop solution of the RGE
bool Alphas::bFlavorMatching    = false;                        // switch flavor matching on or off
double Alphas::fTh[6]           = {0.0023, 0.0048, 0.095, 1.275, 4.18, 173.5};  // PDG 2012; check on pole mass, running mass when using in theory ...!


Alphas::Alphas() {
   // private constructor since this is a Singleton class
}

Alphas::~Alphas() {
}

void Alphas::SetFlavorMatchingThresholds(double th1, double th2, double th3, double th4, double th5, double th6) {
   // in GeV
   fTh[0] = th1;
   fTh[1] = th2;
   fTh[2] = th3;
   fTh[3] = th4;
   fTh[4] = th5;
   fTh[5] = th6;
}


void Alphas::GetFlavorMatchingThresholds(double& th1, double& th2, double& th3, double& th4, double& th5, double& th6) {
   th1 = fTh[0];
   th2 = fTh[1];
   th3 = fTh[2];
   th4 = fTh[3];
   th5 = fTh[4];
   th6 = fTh[5];
}


int Alphas::CalcNf(double mu) {
   //
   //  calculate number of active flavors
   //  if your scale would result in a larger number of
   //  active flavors than defined in fNf, then return
   //  'only' fNf
   //  If flavor matching is turned of, then always use
   //  fixed number of flavors.
   //
   if (!bFlavorMatching) return fNf;
   int nf = 0;
   while (mu > fTh[nf] && nf < 6) nf++;
   if (nf > fNf) return fNf ;
   else return nf;
}


double Alphas::CalcAlphasMu(double mu, double alphasMz, int nLoop, int nFlavors) {

   nLoop        = nLoop == 0 ? fnLoop : nLoop;
   double asmz  = alphasMz==0 ? fAlphasMz : alphasMz;
   int nf       = nFlavors == 0 ? CalcNf(mu) : nFlavors;
   double Q2    = pow(mu,2);

   // - initialize pi and do some initial print out
   const string csep41("#########################################");
   const string cseps = csep41 + csep41;
   static bool first = true;
   static const double twopi = 2. * 4. * atan(1.);
   if (first) {
      first = false;
      cout << endl << " " << cseps << endl;
      printf(" # ALPHAS-GRV: First call:\n");
      //     say::info["ALPHAS-GRV"] << "First call:\n";
      PrintInfo();
   }

   // - initialize beta functions
   //   static const double twopi = 6.28318530717958647692528;
   const double beta0   = 11. - 2./3. * nf;
   const double beta1   = 102. - 38./3. * nf;
   const double beta10  = beta1 / beta0 / beta0;
   const double MZ2 = pow(fMz,2);

   // - exact formula -> extract Lambda from alpha_s(Mz)
   double LAM2 = MZ2 / exp(FBeta(asmz,nLoop,nf));


   // - extract approx alpha_s(mu) value - 2 loop approx is fine
   double LL2 = MZ2 * exp(-2.*twopi/beta0/asmz + beta10 * log(2*twopi/beta0/asmz + beta10));
   double LQ2 = log(Q2 / LL2);
   double as = 2*twopi/beta0/LQ2 * (1. - beta10*log(LQ2)/LQ2);

   // - exact 4-loop value by Newton procedure
   for (int i = 1 ; i <=3 ; i++) {
      double F  = log(Q2/LAM2) - FBeta(as,nLoop,nf);
      double FP = - FBeta(as*1.01,nLoop,nf);
      double FM = - FBeta(as*0.99,nLoop,nf);
      as -= F/(FP-FM)*0.02*as ;
      //printf(" i = %d , alphas = %8.6f , mu = %7.4f\n",i,as,mu);
   }

   return as;
}


void Alphas::PrintInfo() {
   // - Print info
   const string csep41("#########################################");
   const string cseps = csep41 + csep41;
   static const double twopi = 2. * 4. * atan(1.);
   cout << " " << cseps << endl;
   printf(" # ALPHAS-GRV: PI              = %#18.15f\n",twopi/2.);
   printf(" # ALPHAS-GRV: M_Z/GeV         = %#9.6f\n",fMz);
   printf(" # ALPHAS-GRV: a_s(M_Z)        = %#9.6f\n",fAlphasMz);
   printf(" # APLHAS-GRV: a_s loop        = %2i\n",fnLoop);
   printf(" # APLHAS-GRV: flavor-matching = %s\n",(bFlavorMatching?"   T":"   F"));
   printf(" # APLHAS-GRV: nf (M_Z)        = %2d\n",CalcNf(fMz));
   cout << " " << cseps << endl;
}



double Alphas::FBeta(double alphasMz, int nLoop, int nf) {

   // - initialize pi and beta functions
   static const double Pi       = 3.14159265358979312;
   static const double zeta3    = 1.202056903;
   const double beta0   = (11. - 2./3. * nf) / 4.;
   const double beta1   = (102. - 38./3. * nf) / 16.;
   const double beta2   = (2857./2. - 5033./18.*nf + 325./54.*pow(nf,2)) / 64.;
   const double beta10  = beta1 / beta0;
   const double beta102 = pow(beta10,2);
   const double beta103 = pow(beta10,3);
   const double beta20  = beta2 / beta0;
   const double C10     = beta10 / beta0 *log(beta0);

   double aspi = alphasMz/Pi;
   double aspi2 = pow(aspi,2);

   //    if ( nLoop == 2 ){ // 1-loop
   //       return 1./( beta0 * aspi );
   //    }
   if (nLoop == 2) {  // 2-loop RGE
      return C10 + 1./beta0 * (1./aspi + beta10 * log(aspi) + (-beta102) * aspi + (beta103/2.)*aspi2);
   } else if (nLoop == 3) {     // 3-loop RGE
      return C10 + 1./beta0 * (1./aspi + beta10 * log(aspi) + (beta20-beta102) * aspi + (beta10*beta20 + beta103/2.)*aspi2);
   } else if (nLoop == 4) {     // 4-loop RGE
      double beta3      = ((149753./6. + 3564.*zeta3) -
                           (1078361./162. + 6508./27.*zeta3) * nf +
                           (50065./162. + 6472./81.*zeta3) * pow(nf,2) +
                           1093./729. * pow(nf,3)) /256.;
      double beta30     = beta3 / beta0;
      return C10 + 1./beta0 * (1./aspi + beta10 * log(aspi) + (beta20-beta102) * aspi  + (beta30/2. - beta10*beta20 + beta103/2.)*aspi2);
   } else {
      printf("Alphas::FBeta(). Error. Only 2-, 3- or 4-loop solution implemented.\n");
      exit(1);
   }

}
