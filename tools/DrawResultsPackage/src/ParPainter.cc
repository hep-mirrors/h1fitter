#include "ParPainter.h"
#include "CommandParser.h"
#include "DrawLogo.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

#include <stdlib.h>
#include <TGraphErrors.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TFile.h>
#include <TLine.h>

struct partype
{
  double par;
  double unc;
  int dirid;
};
typedef map<string,vector<partype> > parlisttype;

map<int, string> fitstatus;

void ParPainter(vector<Output*> info_output)
{
  //make par list
  parlisttype parlist;

  //read fit status
  for (vector<string>::iterator dit = opts.dirs.begin(); dit != opts.dirs.end(); dit++)
    {
      string fname = *dit + "/minuit.out.txt";

      ifstream f(fname.c_str());
      if (!f.good())
	continue;

      fitstatus[dit - opts.dirs.begin()] = "Converged";

      string line;
      while (getline(f, line))
	if (line.find("MATRIX FORCED POS-DEF") != string::npos)
	  fitstatus[dit - opts.dirs.begin()] = "pos-def-forced";
    }

  //read paramaters
  for (vector<Output*>::iterator io = info_output.begin(); io != info_output.end(); io++)
    {
      int NFittedParameters = (*io)->GetNFittedParameters();
      for(int i=0; i<NFittedParameters; i++)
	{
	  partype pr;
	  pr.par = (*io)->GetFittedParameter(i, false);
	  pr.unc = (*io)->GetFittedParameter(i, true);
	  pr.dirid = io - info_output.begin();
	  parlist[(*io)->GetFittedParametersName(i)->Data()].push_back(pr);
	}
    }
  int ndata = parlist.size();

  float par[info_output.size()][ndata];
  float unc[info_output.size()][ndata];
  string parname[ndata];
  string dirname[info_output.size()];
  for (int i = 0; i < info_output.size(); i++)
    for (int d = 0; d < ndata; d++)
      {
	par[i][d] = 0;
	unc[i][d] = 0;
      }

  for (int d = 0; d < info_output.size(); d++)
    {
      dirname[d] = opts.labels[d];
      while (dirname[d].find("/") != string::npos)
	dirname[d].replace(dirname[d].find("/"), 1, " ");
      while (dirname[d].find("_") != string::npos)
	dirname[d].replace(dirname[d].find("_"), 1, " ");
    }

  //loop on parameters
  int p = 0;
  for (parlisttype::iterator pit = parlist.begin(); pit != parlist.end(); pit++)
    {
      parname[p] = (*pit).first;
      //loop on directories
      for (vector<partype>::iterator dit = (*pit).second.begin(); dit != (*pit).second.end(); dit++)
	{
	  par[(*dit).dirid][p] = (*dit).par;
	  unc[(*dit).dirid][p] = (*dit).unc;
	}
      p++;
    }

  //write table
  FILE *ftab = fopen((opts.outdir + "par.tex").c_str(), "w");
  if (!ftab)
    {
      cout << "Cannot open par tex file" << endl;
      return;
    }

  int points;
  if (info_output.size() >= 1)
    points = 14;
  if (info_output.size() >= 3)
    points = 12;
  if (info_output.size() >= 4)
    points = 11;
  if (info_output.size() >= 5)
    points = 9;

  float cm = 0.035277778 * points * 9;

  fprintf(ftab,"\\documentclass[%dpt]{report}\n", points);
  fprintf(ftab,"\\usepackage{extsizes}\n");
  fprintf(ftab,"\\usepackage[paperwidth=21cm,paperheight=21cm,left=0.2cm,right=0.2cm]{geometry}\n");
  fprintf(ftab,"\\usepackage[scaled]{helvet}\n");
  fprintf(ftab,"\\renewcommand\\familydefault{\\sfdefault}\n");
  fprintf(ftab,"\\pagestyle{empty}\n");

  fprintf(ftab,"\\usepackage{booktabs}\n");
  fprintf(ftab,"\\usepackage{color}\n");
  fprintf(ftab,"\\begin{document}\n");

  fprintf(ftab,"\\begin{table}\n");
  fprintf(ftab,"  \\begin{center}\n");
  fprintf(ftab,"    \\begin{tabular}{l");
  for (int i = 0; i < info_output.size(); i++)
    fprintf(ftab,"p{%.2fcm}", cm);
  fprintf(ftab,"}\n");
  fprintf(ftab,"      \\toprule\n");

  fprintf(ftab," Parameter  ");
  for (int i = 0; i < info_output.size(); i++)
    fprintf(ftab," & %s", dirname[i].c_str());
  fprintf(ftab,"  \\\\ \n");
  fprintf(ftab,"      \\midrule\n");

  for (int d = 0; d < ndata; d++)
    {
      fprintf(ftab,"  %s ", parname[d].c_str());
      for (int i = 0; i < info_output.size(); i++)
	if (unc[i][d] != 0 || par[i][d] != 0 )
	  {
	    if (fitstatus[i] != "pos-def-forced")
	      fprintf(ftab,"& %s $\\pm$ %s", Round(par[i][d], unc[i][d])[0].c_str(), Round(unc[i][d], unc[i][d])[1].c_str());
	    else
	      fprintf(ftab,"& %s $\\pm$ {\\color{red}{ %s }}", Round(par[i][d], unc[i][d])[0].c_str(), Round(unc[i][d], unc[i][d])[1].c_str());
	  }
	else
	  fprintf(ftab,"& - ");
      fprintf(ftab,"  \\\\ \n");
    }

  fprintf(ftab,"      \\midrule\n");
  fprintf(ftab," Fit status  ");
  for (int i = 0; i < info_output.size(); i++)
    fprintf(ftab," & %s", fitstatus[i].c_str());
  fprintf(ftab,"  \\\\ \n");

  fprintf(ftab,"      \\bottomrule\n");
  fprintf(ftab,"    \\end{tabular}\n");
  fprintf(ftab,"  \\end{center}\n");
  fprintf(ftab,"\\end{table}\n");

  fprintf(ftab,"\n");
  fprintf(ftab,"\\end{document}\n");
  fclose(ftab);

  string command = "pdflatex -interaction=batchmode -output-directory=" + opts.outdir + " " + opts.outdir + "par.tex >/dev/null";
  //debug latex
  //string command = "pdflatex  -output-directory=" + opts.outdir + " " + opts.outdir + "par.tex";
  system(command.c_str());

  //cleanup
  string clean = "rm -f " 
    + opts.outdir + "par.aux " 
    + opts.outdir + "par.log " 
    + opts.outdir + "par.nav " 
    + opts.outdir + "par.out " 
    + opts.outdir + "par.snm " 
    + opts.outdir + "par.toc ";
  system(clean.c_str());

  return;
}
