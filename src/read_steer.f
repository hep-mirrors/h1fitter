
      subroutine read_steer
C---------------------------------------------------
C 
C> Read steering file steer.txt
C
C---------------------------------------------------
      implicit none


      include 'ntot.inc'
      include 'steering.inc'
      include 'couplings.inc'
      include 'pdflength.inc'
      include 'pdfparam.inc'
      include 'datasets.inc'
      include 'systematics.inc'
C=================================================


C Define namelists:

      character*32 PDFStyle, Chi2Style

      real*8 Q02       ! Starting scale
      integer IOrder   ! Evolution order
C Main steering parameters namelist
      namelist/H1Fitter/ITheory, IOrder, Q02, HFSCHEME, PDFStyle, 
     $     Chi2Style, LDebug, ifsttype, ASatur, LSatur


C Output style namelist
      namelist/Output/DoBands, Q2VAL, OutNX, OutXRange

C (Optional) MC method namelist
      namelist/MCErrors/LRand, ISeeDMC, StaType, SysType
 
C (Optional) Chebyshev namelist
      namelist/Cheb/ILENPDF,pdfLenWeight,NCHEBGLU,NCHEBSEA
     $     ,IOFFSETCHEBSEA,ichebtypeGlu,ichebtypeSea
     $     ,WMNlen,WMXlen, ChebXMin

C (Optional) Polynomial parameterisation for valence
      namelist/Poly/NPOLYVAL,IZPOPOLY,IPOLYSQR

      integer i, ilastq2
      integer StdCin,StdCout


C Namelist for datafiles to read
      namelist/InFiles/NInputFiles,InputFileNames


C Namelist for EW parameters:
      namelist/EWpars/alphaem, gf, sin2thw, convfac,
     $ Mz, Mw, Mh, wz, ww, wh, wtp,
     $ Vud, Vus, Vub, Vcd, Vcs, Vcb,
     $ men, mel, mmn, mmo, mtn, mta, mup, mdn,
     $ mch, mst, mtp, mbt
C-----------------------------------------------------


C---------

*     ------------------------------------------------
*     Initialise basic parameters
*     ------------------------------------------------




      Itheory = 0



C=================================================


C  PDF length on/off:
      ILENPDF = 0

C PDF length weight factor:
      do i=1,5
         pdfLenWeight(i) = 0.
      enddo
C Chebyshev param. of the gluon:
      NCHEBGLU = 0

C Chebyshev param. of the Sea:
      NCHEBSEA = 0

C Offset for the Sea chebyshev parameters (default:20)
      IOFFSETCHEBSEA = 20

C Type of Chebyshev parameterization:
      ichebtypeGlu = 0
      ichebtypeSea = 0


C 25 Jan 2011
C     Pure polinomial param for the valence quarks:
C     by default starting from N=61 for Uv and N=71 for Dv
      NPOLYVAL = 0

C Add option to change Z of valence PDFs at x=1 from  (1-x) to (1-x)^2
      IZPOPOLY = 1

C Square polynom before calculating dv,uv. This forces positivity
      IPOLYSQR = 0

C  Key for W range 
      WMNlen =  20.
      WMXlen = 320.


      chebxmin = 1.E-5

C  Hermes-like strange:
      ifsttype = 0

* 
      PDFStyle  = '13p HERAPDF'
      Chi2Style = 'HERAPDF'


C MC Errors defaults:
      lRAND = .false.
      iSEEDmc = 0
      STATYPE = 1
      SYSTYPE = 1

C PDF output options:
      outnx = 101
      do i=1,NBANDS
       Q2VAL(i) = -1.
      enddo
      outxrange(1) = 1e-4
      outxrange(2) = 1.0

C XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
      strange_frac = 0.31
      charm_frac = 0.00


      mch        = 1.4D0
      mbt        = 4.75D0
      mtp        = 174.D0

      HFSCHEME = 0


C==== 24/08/2010: Add Saturation inspired cut ====
      ASatur = 0.0
      LSatur = 0.0


C=================================================



C
C  Read the main H1Fitter namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=H1Fitter,END=41,ERR=42)
      close (51)


      open (51,file='ewparam.txt',status='old')
      read (51,NML=EWpars,END=43,ERR=44)
      close (51)

C
C  Read the output namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=Output,END=51,ERR=52)
      close (51)

C
C Decode PDF style:
C      
      call SetPDFStyle(PDFStyle)

C
C Decode Chi2 style:
C
      call SetChi2Style(Chi2Style)

      HF_MASS(1) = mch
      HF_MASS(2) = mbt
      HF_MASS(3) = mtp

C
C  Read the MC method namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=MCErrors,ERR=62,end=61)
 61   continue
      close (51)

C
C  Read the Chebyshev namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=Cheb,ERR=64,end=63)
 63   continue
      close (51)

C
C  Read the Poly namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=Poly,ERR=66,end=65)
 65   continue
      close (51)


C
C  Read the data namelist:
C
      open (51,file='steering.txt',status='old')
      read (51,NML=InFiles,END=71,ERR=72)
      print '(''Read '',I4,'' data files'')',NInputFiles
      close (51)


      if (lDebug) then
C Print the namelists:
         print *,'Input Namelists:'
         print H1Fitter
         print Output
         print MCErrors
         print InFiles
         print EWpars
      endif


      I_FIT_ORDER = IOrder
      starting_scale = Q02



C
C Names of syst. error sources:
C
      do i=1,NSYS
         System(i) = ' '
      enddo

      goto 73
C 
 41   continue
      print '(''Namelist &H1Fitter NOT found'')'
      goto 73
 42   continue
      print '(''Error reading namelist &H1Fitter, STOP'')'
      stop
 43   continue
      print '(''Namelist @EWPars NOT found, STOP'')'
      stop
 44   continue
      print '(''Error reading namelist @EWPars, STOP'')'
      stop
 51   continue
      print '(''Namelist &Output NOT found'')'
      goto 73
 52   continue
      print '(''Error reading namelist &Output, STOP'')'
      stop
 62   continue
      print '(''Error reading namelist &MCErrors, STOP'')'
      stop
 64   continue
      print '(''Error reading namelist &Cheb, STOP'')'
      stop
 66   continue
      print '(''Error reading namelist &Poly, STOP'')'
      stop
 71   continue
      print '(''Namelist &InFiles NOT found'')'
      goto 73
 72   continue
      print '(''Error reading namelist &InFiles, STOP'')'
      stop


 73   continue

      chebxminlog = log(chebxmin)


      ilastq2 = NBANDS/2
      do i=NBANDS/2,1,-1
         if (q2val(i).lt.0) ilastq2 = i-1
      enddo
      do i=1,NBANDS/2
       Q2VAL(i+ilastq2) = Q2VAL(i+NBANDS/2)
      enddo
c      print *,'q2val ', (q2val(i),i=1,NBANDS)

* --- Check the consistency of the steering file

      if (HFSCHEME.eq.1.and.HF_MASS(2)**2.lt.starting_scale) then
       write(6,*)
       write(6,*) 'Bottom thres. has to be larger than starting scale'
       write(6,*)
       stop
      endif

      if (HFSCHEME.eq.1.and.HF_MASS(2).lt.HF_MASS(1)) then
       write(6,*)
       write(6,*) 'Bottom thres. has to be larger than charm thres.'
       write(6,*)
       stop
      endif


      if (NCHEBGLU.ne.0) then
         print *,'Use Chebyshev polynoms for gluon with N=',NCHEBGLU
      endif

      if (NCHEBSEA.ne.0) then
         print *,'Use Chebyshev polynoms for sea with N=',NCHEBSEA
         print *,'Offset for minuit parameters is',IOFFSETCHEBSEA
      endif

      return
      end


      Subroutine SetPDFStyle(PDFStyle)
C---------------------------------------
C
C>  Set PDF parameterisation type
C
C---------------------------------------
      implicit none
      character*(*) PDFStyle
      include 'steering.inc'
C---------------------------------
      
      if (PDFStyle.eq.'10p HERAPDF') then
         iparam = 22
      elseif (PDFStyle.eq.'13p HERAPDF') then
         iparam = 229
      elseif (PDFStyle.eq.'CTEQ') then
         iparam = 171717
      elseif (PDFStyle.eq.'CHEB') then
         iparam = 4
      else
         print *,'Unsupported PDFStyle =',PDFStyle
         print *,'Check value in steering.txt'
         print *,'STOP'
         stop
      endif

      end


      Subroutine SetChi2Style(Chi2Style)
C---------------------------------------
C
C>  Set Chi2 style
C
C---------------------------------------
      implicit none
      character*(*) Chi2Style
      include 'steering.inc'
C---------------------------------

      if (Chi2Style.eq.'HERAPDF') then
         ICHI2 = 11
      elseif (Chi2Style.eq.'HERAPDF Sqrt') then
         ICHI2 = 31
      elseif (Chi2Style.eq.'HERAPDF Linear') then
         ICHI2 = 21
      elseif (Chi2Style.eq.'CTEQ') then
         ICHI2 = 2
      elseif (Chi2Style.eq.'H12000') then
         ICHI2 = 1        
      else
         print *,'Unsupported Chi2Style =',Chi2Style
         print *,'Check value in steering.txt'
         print *,'STOP'
         stop
      endif
      
      end
