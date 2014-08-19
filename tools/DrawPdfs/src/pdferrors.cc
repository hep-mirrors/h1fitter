#include "pdferrors.h"

#include <math.h>
#include <algorithm>
#include <iostream>

//Functions for MC errors
double mean(vector <double> xi)
{
  if (xi.size() == 0)
    {
      cout << "Error in pdferrors.cc, passed empty vector to double mean(vector <double> xi) "<< endl;
      return 0;      
    }
  double avg = 0;
  for (vector<double>::iterator it = xi.begin(); it != xi.end(); it++)
    avg += *it;
  avg /= xi.size();

  return avg;
}
double rms(vector <double> xi)
{
  if (xi.size() == 0)
    {
      cout << "Error in pdferrors.cc, passed empty vector to double rms(vector <double> xi) "<< endl;
      return 0;      
    }

  double sum2 = 0;
  for (vector<double>::iterator it = xi.begin(); it != xi.end(); it++)
    sum2 += pow(*it,2);
  sum2 /= xi.size();
  return sqrt(fabs(sum2 - pow(mean(xi),2)));
}
double median(vector <double> xi)
{
  if (xi.size() == 0)
    {
      cout << "Error in pdferrors.cc, passed empty vector to double median(vector <double> xi) "<< endl;
      return 0;      
    }
  double med = 0;
  sort(xi.begin(), xi.end());

  if (xi.size() % 2) //odd
    med = *(xi.begin() + ((xi.size() + 1) / 2) - 1);
  else //even
    {
      med += *(xi.begin() + ((xi.size() / 2) - 1));
      med += *(xi.begin() + ((xi.size() / 2)));
      med /=2;
    }

  return med;
}
double cl(int sigma)
{
  switch (sigma)
    {
    case 1:
      return 0.682689492137086;
      break;
    case 2:
      return 0.954499736103642;
      break;
    case 3:
      return 0.997300203936740;
      break;
    default:
      cout << "Confidence Level interval available only for sigma = 1, 2, 3, requested: " << sigma << " sigma" << endl;
      exit(1);
    }
}

double delta(vector <double> xi, double central, double ConfLevel)
{
  double delta = 0;

  vector <double> deltaxi;
  vector<double>::iterator i = xi.begin();
  while (i != xi.end())
    {
      deltaxi.push_back(fabs(*i - central));
      ++i;
    }

  sort(deltaxi.begin(), deltaxi.end());

  vector<double>::iterator di = deltaxi.begin();
  while (di != deltaxi.end())
    {
      delta = *di;
      int index = di - deltaxi.begin() + 1;
      double prob = (double)index / (double)deltaxi.size();
      //      cout << index << "  " << *di << "  " << prob << endl;
      if (prob > ConfLevel)
	break;
      ++di;
    }
  return delta;
}
void deltaasym(vector <double> xi, double central, double& delta_p, double& delta_m, double ConfLevel)
{
  delta_m = delta_p = 0;

  vector <double> deltaxi;
  vector<double>::iterator i = xi.begin();
  while (i != xi.end())
    {
      deltaxi.push_back(*i - central);
      ++i;
    }

  sort(deltaxi.begin(), deltaxi.end());

  vector<double>::iterator di = deltaxi.begin();
  while (di != deltaxi.end())
    {
      delta_m = fabs(*di);
      int index = di - deltaxi.begin() + 1;
      double prob = (double)index / (double)deltaxi.size();
      //      cout << index << "  " << *di << "  " << prob << endl;
      if (prob >= ((1-ConfLevel)/2.))
	break;
      ++di;
    }

  di = deltaxi.end() - 1;
  while (di != deltaxi.begin())
    {
      delta_p = *di;
      int index = di - deltaxi.begin() + 1;
      double prob = (double)index / (double)deltaxi.size();
      //      cout << index << "  " << *di << "  " << prob << endl;
      if (prob <= ((1.+ConfLevel)/2.))
	break;
      --di;
    }
}
//Functions for Hessian errors
double ahessdelta(vector <double> xi, vector < vector <double> > corr)
{
  double ep, em;
  ahessdeltaasym(xi, ep, em, corr);
  return (ep+em)/2.;
}
void ahessdeltaasym(vector <double> xi, double& delta_p, double& delta_m, vector < vector<double> > corr )
{
  if (xi.size() % 2 == 0)
    {
      cout << "Error, expected even number of PDF variations for asymmetric hessian" << endl;
      cout << xi.size() << endl;
      cout << (xi.size() % 2) << endl;
      exit(1);
    }
  double val = *(xi.begin());
  double ep = 0;
  double em = 0;

  if ( corr.empty() ) {
    for (vector<double>::iterator i = xi.begin() + 1; i != xi.end(); i++,i++)
      {
	double vm = *i;
	double vp = *(i+1);
	ep += pow(max(max(0., vp-val), vm-val),2);
	em += pow(max(max(0., val-vp), val-vm),2);
      }
    delta_p = sqrt(ep);
    delta_m = sqrt(em);
  }
  else {
    for (int i=0; i < xi.size()-1; i++, i++)
      for (int j = 0; j < xi.size()-1; j++,j++) 
	{
	  double vm = xi[i+1];
	  double vp = xi[i+2];
	  double vm2 = xi[j+1];
	  double vp2 = xi[j+2];
	  ep += max(max(0., vp-val), vm-val)*max(max(0., vp2-val), vm2-val)*corr[i/2][j/2];
	  em += max(max(0., val-vp), val-vm)*max(max(0., val-vp2), val-vm2)*corr[i/2][j/2];
	}
    delta_p = sqrt(ep);
    delta_m = sqrt(em);
  }
}


double shessdelta(vector <double> xi, vector < vector <double> > cor)
{
  double val = *(xi.begin());
  double err = 0;

  if ( cor.empty() ) {
    for (vector<double>::iterator i = xi.begin() + 1; i != xi.end(); i++)
      err += pow(*i-val, 2);
  }
  else {
    for (int i=1; i<xi.size(); i++) 
      for (int j=1; j<xi.size(); j++) 
	{
	  err += (xi[i]-val)*(xi[j]-val)*cor[i-1][j-1];
	}
  }
  return sqrt(err);
}
//Functions for VAR uncertainties
void vardeltaasym(vector <double> xi, int npar, double& delta_p, double& delta_m)
{
  vector <double> yi;
  yi.push_back(*(xi.begin()));
  for (int i = 0; i < npar; i++)
    {
      yi.push_back(*(xi.end()-1));
      xi.pop_back();
    }
  double e1_p, e1_m;
  ahessdeltaasym(xi, e1_p, e1_m);
  double e2_p, e2_m;
  deltaenvelope(yi, e2_p, e2_m);
  delta_p = sqrt(pow(e1_p,2)+pow(e2_p,2));
  delta_m = sqrt(pow(e1_m,2)+pow(e2_m,2));
}
void deltaenvelope(vector <double> xi, double& delta_p, double& delta_m)
{
  double val = *(xi.begin());
  delta_p = 0;
  delta_m = 0;
  for (vector<double>::iterator i = xi.begin() + 1; i != xi.end(); i++)
    {
      delta_p = max(delta_p, *i - val);
      delta_m = max(delta_m, val - *i);
    }
}
