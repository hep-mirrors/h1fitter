      Subroutine GetNCXsection(IDataSet)
      call GetDisXsection(IDataSet, 'NCDIS')
      end
      
      Subroutine GetCCXsection(IDataSet)
      call GetDisXsection(IDataSet, 'CCDIS')
      end
      
      Subroutine GetNCCharmXsection(IDataSet)
      call GetDisXsection(IDataSet, 'CHARMDIS')
      end
      
      Subroutine GetIntegratedNCXsection(IDataSet)
      call GetIntegratedDisXsection(IDataSet, 'NCDIS')
      end
      
      Subroutine GetIntegratedCCXsection(IDataSet)
C This is not fully tested
      call GetIntegratedDisXsection(IDataSet, 'CCDIS')
      end


      Subroutine  GetIntegratedDisXsection(IDataSet, XSecType)
C----------------------------------------------------------------
C
C  Double differential integrated DIS cross section calculation
C  Fills global array THEO.
C  Searches for xmin, xmax, ymin, ymax, q2min, q2max column names 
C  in a data file as well as 'sqrt(S)' and 'lumi(e-)/lumi(tot)' in
C  additional information
C  
C  Created by KN, 20/01/2012
C---------------------------------------------------------------

      implicit none
      include 'ntot.inc'
      include 'couplings.inc'
      include 'datasets.inc'
      include 'indata.inc'
      include 'theo.inc'
      include 'qcdnumhelper.inc'

      character*(*) XSecType
      integer IDataSet
      integer idxQ2min, idxQ2max, idxYmin, idxYmax, idxXmin, idxXmax
      integer i,  idx, iq2, ix, j, kkk
      
      integer nq2split
      parameter(nq2split=25)
      integer nxsplit
      parameter(nxsplit=25)

      double precision X(NPMaxDIS),Y(NPMaxDIS),Q2(NPMaxDIS),XSecP(NPMaxDIS),XSecN(NPMaxDIS)
      double precision q2_1, q2_2, alphaem_run, factor, XSec, YPlus
      double precision dq2(NPMaxDIS), dx(NPMaxDIS)
      double precision Charge, polarity, S
      double precision q2min,q2max,xmin,xmax
      double precision EmToEtotRatio, alm_mz
      logical LoopOverYBins
      integer NSubBins

C Functions:
      integer GetBinIndex
      integer GetInfoIndex
      double precision AEMRUN

C---------------------------------------------------------

      if ((nq2split+1)*(nxsplit+1).gt.NPMaxDIS) then
         print *,'ERROR IN GetIntegratedNCXsection'
         print *,'INCREASE NPMax to ',(nq2split+1)*(nxsplit+1)
         call HF_stop
      endif

C
C Get indexes for Q2, x and y bins:
C
      idxQ2min = GetBinIndex(IDataSet,'q2min')
      idxXmin  = GetBinIndex(IDataSet,'xmin')
      idxYmin = GetBinIndex(IDataSet,'ymin')
      idxQ2max = GetBinIndex(IDataSet,'q2max')
      idxXmax  = GetBinIndex(IDataSet,'xmax')
      idxYmax = GetBinIndex(IDataSet,'ymax')
      LoopOverYBins = .false.
      S = (DATASETInfo( GetInfoIndex(IDataSet,'sqrt(S)'), IDataSet))**2
      EmToEtotRatio=(DATASETInfo(GetInfoIndex(IDataSet,
     $     'lumi(e-)/lumi(tot)'),IDataSet))

      if (idxQ2min.eq.0 .or. idxQ2max.eq.0) then
         print *, 'Q2 bins not well defined in a data file'
         Return
      endif
      if (idxXmin.eq.0 .or. idxXmax.eq.0) then
         if (idxYmin.eq.0 .or. idxYmax.eq.0) then
            print *, 'X or Y bins need to be defined in a data file'
            Return
         endif
         LoopOverYBins = .true.
      endif

      do i=1,NDATAPOINTS(IDataSet)
         idx =  DATASETIDX(IDataSet,i)

         if(idx.gt.1) then      ! maybe I can just copy previous entry
            if((AbstractBins(idxQ2min,idx).eq.AbstractBins(idxQ2min,idx-1)).and.
     +       (AbstractBins(idxQ2max,idx).eq.AbstractBins(idxQ2max,idx-1)).and.
     +       (AbstractBins(idxXmin,idx).eq.AbstractBins(idxXmin,idx-1)).and.
     +       (AbstractBins(idxXmax,idx).eq.AbstractBins(idxXmax,idx-1)).and.
     +       (AbstractBins(idxYmin,idx).eq.AbstractBins(idxYmin,idx-1)).and.
     +       (AbstractBins(idxYmax,idx).eq.AbstractBins(idxYmax,idx-1))) then
               THEO(idx) = THEO(idx-1)
               cycle
            endif
         endif

         q2min = AbstractBins(idxQ2min,idx)
         q2max = AbstractBins(idxQ2max,idx)

         j=0
         do iq2=0,nq2split
            q2_1 = q2_2
            q2_2 = exp( log(q2min) + (log(q2max) - log(q2min)) / nq2split*dble(iq2))            
            if(iq2.gt.0) then
               do ix=0, nxsplit-1
                  j=j+1
                  dq2(j) = q2_2 - q2_1
                  q2(j) = exp( log(q2_1) + 0.5*(log(q2_2) - log(q2_1)) ) 

                  
                  if(LoopOverYBins) then
                     xmax = q2(j) / (S * AbstractBins(idxYmin,idx))
                     xmin = q2(j) / (S * AbstractBins(idxYmax,idx))
                  else
                     xmax = AbstractBins(idxXmax,idx)
                     xmin = AbstractBins(idxXmin,idx)
                  endif
                  
                  x(j) = xmin + (xmax-xmin)/dble(nxsplit)*(dble(ix)+0.5)
                  dx(j) = (xmax-xmin) / dble(nxsplit)
                  y(j) = q2(j) / (S * x(j))
               enddo
            endif
         enddo                  ! loop over q2 subgrid
         NSubBins = j

         polarity = 0.D0
         call CalcReducedXsectionForXYQ2(X,Y,Q2,NSubBins, 1.D0,
     $        polarity,IDataSet,XSecType, XSecP)
         call CalcReducedXsectionForXYQ2(X,Y,Q2,NSubBins,-1.D0,
     $        polarity,IDataSet,XSecType, XSecN)

         XSec = 0.D0
         do j=1, NSubBins
            XSecP(j) = EmToEtotRatio*XSecN(j) + (1.D0-EmToEtotRatio)*XSecP(j)

            Yplus  = 1. + (1.-y(j))**2
            XSecP(j) = XSecP(j) * YPlus

            factor=1.D0
            if (XSecType.eq.'CCDIS') then
               factor=(Mw**4/(Mw**2+q2(j))**2)*Gf**2/(2*pi*x(j))*convfac
            else if (XSecType.eq.'NCDIS') then
c               alm_mz = 1.d0 / 128.9d0
c               alphaem_run = alm_mz/(1. - alm_mz * 2/(3.*pi)*log(q2(j)/mz**2))
               alphaem_run = aemrun(q2(j))
               factor=2*pi*alphaem_run**2/(x(j)*q2(j)**2)*convfac
            else
               print *, 'GetIntegratedDisXsection, XSecType',XSecType,
     $              'not supported' 
               stop
            endif

            XSecP(j) = XSecP(j) * factor
            XSecP(j) = XSecP(j) * dq2(j)
            XSecP(j) = XSecP(j) * dx(j)

            XSec = XSec+XSecP(j)
         enddo
         
         THEO(idx) =  XSec
c temporary divide over dq2
c         THEO(idx) =  XSec / (q2max - q2min)
c         print *, idx, ':', THEO(idx)
c         call HF_stop
      enddo   ! loop over data points

      end



      Subroutine GetDisXsection(IDataSet, XSecType)
C----------------------------------------------------------------
C
C  NC and CC double differential reduced cross section calculation 
C  for dataset IDataSet. Fills global array THEO.
C  Needs 'Q2', 'x', 'y' columns in a data file and following CInfo fields:
C                'sqrt(S)','e charge', 'reduced', 'e polarity' 
C
C  Created by SG, 25/05/2011
C  Start with zero mass implementation
C                 14/06/2011 : re-introduce RT code
C---------------------------------------------------------------
      implicit none
      include 'ntot.inc'
      include 'steering.inc'
      include 'datasets.inc'
      include 'indata.inc'
      include 'theo.inc'
      include 'fcn.inc'
      include 'couplings.inc'
      include 'qcdnumhelper.inc'

      character*(*) XSecType
      integer IDataSet
      integer idxQ2, idxX, idxY, i,  idx
      
      double precision X(NPMaxDIS),Y(NPMaxDIS),Q2(NPMaxDIS),XSec(NPMaxDIS)
      double precision Charge, polarity, alphaem_run, factor
      logical IsReduced

C Functions:
      integer GetBinIndex
      integer GetInfoIndex
      double precision AEMRUN

c H1qcdfunc
      integer ifirst
      data ifirst /1/
C---------------------------------------------------------


      if (NDATAPOINTS(IDataSet).gt.NPMaxDIS) then
         print *,'ERROR IN GetDisXsection'
         print *,'INCREASE NPMaxDIS to ',NDATAPOINTS(IDataSet)
         call HF_stop
      endif

C
C Get indexes for Q2, x and y bins:
C
      idxQ2 = GetBinIndex(IDataSet,'Q2')
      idxX  = GetBinIndex(IDataSet,'x')
      idxY = GetBinIndex(IDataSet,'y')
      IsReduced = DATASETInfo( GetInfoIndex(IDataSet,'reduced'), IDataSet).gt.0

      if (idxQ2.eq.0 .or. idxX.eq.0 .or. idxY.eq.0) then
         Return
      endif

C prepare bins:
      do i=1,NDATAPOINTS(IDataSet)
C
C Reference from the dataset to a global data index:
C
         idx =  DATASETIDX(IDataSet,i)
C
C Local X,Y,Q2 arrays, used for QCDNUM SF caclulations:
C
         X(i)   = AbstractBins(idxX,idx)
         Y(i)   = AbstractBins(idxY,idx)
         Q2(i)  = AbstractBins(idxQ2,idx)
      enddo

      call ReadPolarityAndCharge(idataset,charge,polarity)
      call CalcReducedXsectionForXYQ2(X,Y,Q2,NDATAPOINTS(IDataSet),
     $     charge,polarity,IDataSet,XSecType,XSec)


      do i=1,NDATAPOINTS(IDataSet)
         idx =  DATASETIDX(IDataSet,i)

         factor=1.D0
         if(.not. IsReduced) then
            if (XSecType.eq.'CCDIS') then
               factor=(Mw**4/(Mw**2+q2(i))**2)*Gf**2/(2*pi*x(i))*convfac
            else if (XSecType.eq.'NCDIS'.or.XSecType.eq.'CHARMDIS') then
               alphaem_run = aemrun(q2(i))
               factor=2*pi*alphaem_run**2/(x(i)*q2(i)**2)*convfac
            else
               print *, 'GetDisXsection, XSecType',XSecType,
     $              'not supported'
               stop
            endif
         endif

         THEO(idx) =  XSec(i)*factor

      enddo

      if ((iflagFCN.eq.3).and.(h1QCDFUNC).and.(XSecType.eq.'NCDIS')) then
         if (ifirst.eq.1) then
            print*,'getting output for the H1QCDFUNC'
        
            call GetH1qcdfuncOutput(charge, polarity)
            ifirst=0
            
         endif
      endif
      end



      Subroutine ReadPolarityAndCharge(idataset,charge,polarity)
C----------------------------------------------------------------
C
C  Get polarity and charge from the data file
C
C  Created by Krzysztof Nowak, 18/01/2012
C---------------------------------------------------------------

      implicit none
      include 'ntot.inc'
      include 'datasets.inc'
      include 'polarity.inc'

C Input:
      integer IDataSet

C Output:
      double precision charge, polarity

      double precision err_pol_unc, shift_pol
      double precision err_pol_corL
      double precision err_pol_corT

C Functions:
      integer GetInfoIndex

      polarity=0.d0
      err_pol_unc=0.d0
      err_pol_corL=0.d0
      err_pol_corT=0.d0
      polarity = DATASETInfo( GetInfoIndex(IDataSet,'e polarity'),
     $     IDataSet)
      if (polarity.ne.0) then
         err_pol_unc = 
     $        DATASETInfo( GetInfoIndex(IDataSet,'pol err unc'), IDataSet)
         err_pol_corL = 
     $        DATASETInfo( GetInfoIndex(IDataSet,'pol err corLpol'), IDataSet)
         err_pol_corT = 
     $        DATASETInfo( GetInfoIndex(IDataSet,'pol err corTpol'), IDataSet)
      endif
      charge = DATASETInfo( GetInfoIndex(IDataSet,'e charge'), IDataSet)

      if (charge.lt.0.) then
         if (polarity.gt.0) then
            shift_pol=shift_polRHm
         else
            shift_pol=shift_polLHm
         endif
      else
         if (polarity.gt.0) then
            shift_pol=shift_polRHp
         else
            shift_pol=shift_polLHp
         endif
      endif

      polarity=polarity*(1+err_pol_unc/100*shift_pol+
     $     err_pol_corL/100*shift_polL+
     $     err_pol_corT/100*shift_polT)

c
c      if(polarity.ne.0.d0) then
c         print '( ''charge:  '', F8.4, 
c     $        '' pol: '', F16.4, 
c     $        ''shift pol: '', F16.4 , 
c     $        ''shift Lpol: '', F16.4 , 
c     $        ''shift Tpol: '', F16.4 )', 
c     $        charge, polarity,shift_pol,shift_polL,shift_polT
c      endif
c
      end


      Subroutine CalcReducedXsectionForXYQ2(X,Y,Q2,npts,charge,polarity,
     $     idataset,XSecType,XSec)
C----------------------------------------------------------------
C   Double differential reduced cross section calculation 
C   for a table given by X, Y, Q2. Fills array XSec
C
C  Created by Krzysztof Nowak, 18/01/2012
C---------------------------------------------------------------
      implicit none
      include 'ntot.inc'
      include 'steering.inc'
      include 'datasets.inc'
      include 'qcdnumhelper.inc'

C Input:
      integer npts, IDataSet
      character*(*) XSecType
      double precision X(NPMaxDIS),Y(NPMaxDIS),Q2(NPMaxDIS)
      double precision Charge, polarity
C Output: 
      double precision XSec(NPMaxDIS)


      integer i, idx
      double precision yplus(NPMaxDIS), yminus(NPMaxDIS)
      double precision F2(NPMaxDIS),xF3(NPMaxDIS),FL(NPMaxDIS)
      double precision F2gamma(NPMaxDIS),FLgamma(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS),F2b(NPMaxDIS),FLb(NPMaxDIS)

C---------------------------------------------------------


C Protect against overflow of internal arrays:
C
      if (npts.gt.NPMaxDIS) then
         print *,'ERROR IN CalculateReducedXsection'
         print *,'INCREASE NPMaxDIS (qcdnumhelper.inc) to ',npts
         call HF_stop
      endif


      call UseZmvnsScheme(F2, FL, xF3, F2gamma, FLgamma,
     $     q2, x, npts, polarity, charge, XSecType)


      if     (mod(HFSCHEME,10).eq.1) then

         call UseAcotScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $        x, q2, npts, polarity, XSecType, charge, IDataSet)
         
      elseif (mod(HFSCHEME,10).eq.2) then
         
         call UseRtScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $        x, q2, npts, XSecType, F2gamma, FLgamma, IDataSet)
         
      elseif (mod(HFSCHEME,10).eq.3) then 

         call UseHqstfScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $        x, q2, npts, XSecType)

      elseif (mod(HFSCHEME,10).eq.4) then 

         call UseABKMFFScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $        x, q2, npts, XSecType, F2gamma, FLgamma, IDataSet)
      endif

C all the transformations below are array operations!
      yplus = 1+(1-y)**2
      yminus = 1-(1-y)**2

      if(XSecType.eq.'CCDIS') then
         if (charge.gt.0) then
            XSec = 0.5*(yplus*F2 - yminus*xF3 - y*y*FL)
            Xsec = Xsec*(1+polarity)
         else
            XSec = 0.5*(yplus*F2 + yminus*xF3 - y*y*FL)
            Xsec = Xsec*(1-polarity)
         endif
      else if(XSecType.eq.'NCDIS') then
         XSec = F2 + yminus/yplus*xF3 - y*y/yplus*FL
      else if(XSecType.eq.'CHARMDIS') then
         XSec = F2c - y*y/yplus*FLc
      else
         print *, 'CalcReducedXsectionForXYQ2, XSecType',
     $        XSecType,'not supported'
         stop
      endif
      
      end


      subroutine UseZmvnsScheme(f2, fl, xf3, f2gamma, flgamma,
     $     q2, x, npts, polarity, charge, XSecType)
C----------------------------------------------------------------
C    Calculates F2, FL, xF3, F2gamma and FLgamma using ZMVFNS from QCDNUM
C  
C  Created by Krzysztof Nowak, 31/01/2012
C---------------------------------------------------------------
      implicit none
      include 'steering.inc'
      include 'couplings.inc'
      include 'qcdnumhelper.inc'

C Input:
      double precision X(NPMaxDIS),Q2(NPMaxDIS)
      double precision charge, polarity
      integer npts
      character*(*) XSecType

C Output: 
      double precision F2(NPMaxDIS), FL(NPMaxDIS), xF3(NPMaxDIS)
      double precision F2gamma(NPMaxDIS), FLgamma(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS)
      double precision F2b(NPMaxDIS),FLb(NPMaxDIS)

C--------------------------------------------------------
C Temporary variables:
      double precision F2m(NPMaxDIS),xF3m(NPMaxDIS),FLm(NPMaxDIS)
      integer i
      double precision ve,ae,au,ad,vu,vd,A_u,A_d,B_u,B_d,pz


C EW param

      double precision sin2th_eff, xkappa, epsilon
      double precision deltar,sweff, sin2thw2
      double precision cau, cad, cvu, cvd




C QCDNUM ZMVFNS, caclulate FL, F2 and xF3 for d- and u- type quarks all bins:

      if(XSecType.eq.'CCDIS') then
         if (charge.gt.0) then
            CALL ZMSTFUN(1,CCEP2F,X,Q2,FL,npts,0)
            CALL ZMSTFUN(2,CCEP2F,X,Q2,F2,npts,0)
            CALL ZMSTFUN(3,CCEP3F,X,Q2,XF3,npts,0)    
         else
            CALL ZMSTFUN(1,CCEM2F,X,Q2,FL,npts,0)
            CALL ZMSTFUN(2,CCEM2F,X,Q2,F2,npts,0)      
            CALL ZMSTFUN(3,CCEM3F,X,Q2,XF3,npts,0) 
         endif
      elseif (XSecType.eq.'NCDIS'.or.XSecType.eq.'CHARMDIS') then
C     u-type ( u+c ) contributions 
         CALL ZMSTFUN(1,CNEP2F,X,Q2,FL,npts,0)
         CALL ZMSTFUN(2,CNEP2F,X,Q2,F2,npts,0)
         CALL ZMSTFUN(3,CNEP3F,X,Q2,XF3,npts,0)    
         
C     d-type (d + s + b) contributions
         CALL ZMSTFUN(1,CNEM2F,X,Q2,FLm,npts,0)
         CALL ZMSTFUN(2,CNEM2F,X,Q2,F2m,npts,0)
         CALL ZMSTFUN(3,CNEM3F,X,Q2,XF3m,npts,0) 
      else
         print *, 'UseZmvnsScheme, XSecType',XSecType,
     $        'not supported'
         stop
      endif

c     for NC needs to combine F2p with F2m etc.        

      if(XSecType.eq.'NCDIS'.or.XSecType.eq.'CHARMDIS') then
         if(EWFIT.eq.0) then
C
C EW couplings of the electron
C
            ve = -0.5d0 + 2.*sin2thw
            ae = -0.5d0         

C
C and quarks
C         
         
            au = 0.5d0
            ad = -0.5d0
                  
            vu = au - (4.d0/3.d0)*sin2thw
            vd = ad + (2.d0/3.d0)*sin2thw
         else
            call wrap_ew(q2,sweff,deltar,cau,cad,cvu,cvd,polarity,charge)
            sin2thw2 = 1.d0 - MW**2/MZ**2
            sin2th_eff = 0.23134d0
            xkappa = sin2th_eff/sin2thw
            epsilon = xkappa -1.0
            ve = -0.5d0 + 2.*sin2th_eff
            ae = -0.5d0
               
            vu = cvu - (4.d0/3.d0)*epsilon*sin2thw2
            vd = cvd + (2.d0/3.d0)*epsilon*sin2thw2
            au = cau
            ad = cad
         endif
         
         do i=1,npts
            
C     Propagator factor PZ
            PZ = 4.d0 * sin2thw * cos2thw * (1.+Mz**2/Q2(i))
               
C     modify propagator for EW corrections -- only for EW fit
            if (EWfit.ne.0) PZ = PZ * (1.d0 - Deltar)
               
            PZ = 1./Pz
C     EW couplings of u-type and d-type quarks at the scale Q2
               
            if (charge.gt.0) then
               A_u = e2u        ! gamma
     $              + (-ve-polarity*ae)*PZ*2.*euq*vu !gamma-Z
     $              + (ve**2 + ae**2+2*polarity*ve*ae)*PZ**2*(vu**2+au**2) !Z
               
               A_d = e2d 
     $              + (-ve-polarity*ae)*PZ*2.*edq*vd 
     $              + (ve**2 + ae**2+2*polarity*ve*ae)*PZ**2*(vd**2+ad**2)
               
               B_u = (ae+polarity*ve)*PZ*2.*euq*au !gamma-Z
     $              + (-2.*ve*ae-polarity*(ve**2+ae**2))*(PZ**2)*2.*vu*au !Z
               B_d = (ae+polarity*ve)*PZ*2.*edq*ad 
     $              + (-2.*ve*ae-polarity*(ve**2+ae**2))*(PZ**2)*2.*vd*ad
            else
               A_u = e2u        ! gamma
     $              + (-ve+polarity*ae)*PZ*2.*euq*vu !gamma-Z
     $              + (ve**2 + ae**2-2*polarity*ve*ae)*PZ**2*(vu**2+au**2) !Z
               
               A_d = e2d 
     $              + (-ve+polarity*ae)*PZ*2.*edq*vd 
     $              + (ve**2 + ae**2-2*polarity*ve*ae)*PZ**2*(vd**2+ad**2)
               
               B_u = (-ae+polarity*ve)*PZ*2.*euq*au !gamma-Z
     $              + (2.*ve*ae-polarity*(ve**2+ae**2))*(PZ**2)*2.*vu*au !Z
               B_d = (-ae+polarity*ve)*PZ*2.*edq*ad 
     $              + (2.*ve*ae-polarity*(ve**2+ae**2))*(PZ**2)*2.*vd*ad
               
            endif
               
cv for polarised case should reduce to:
cv         A_u = e2u - ve*PZ*2.*euq*vu +(ve**2 + ae**2)*PZ**2*(vu**2+au**2)
cv         A_d = e2d - ve*PZ*2.*edq*vd +(ve**2 + ae**2)*PZ**2*(vd**2+ad**2)
cv         B_u = -ae*PZ*2.*euq*au + 2.*ve*ae*(PZ**2)*2.*vu*au
cv         B_d = -ae*PZ*2.*edq*ad + 2.*ve*ae*(PZ**2)*2.*vd*ad

            
            F2Gamma(i) = 4.D0/9.D0 * F2(i)  + 1.D0/9.D0 * F2m(i)
            FLGamma(i) = 4.D0/9.D0 * FL(i)  + 1.D0/9.D0 * FLm(i)
            XF3(i)  = B_U*XF3(i)  + B_D*XF3m(i)
            F2(i)   = A_U*F2(i)   + A_D*F2m(i)
            FL(i)   = A_U*FL(i)   + A_D*FLm(i)
            
         enddo
      elseif(XSecType.eq.'CCDIS') then
         do i=1,npts
            F2Gamma(i) = 4.D0/9.D0 * F2(i)  + 1.D0/9.D0 * F2m(i)
            FLGamma(i) = 4.D0/9.D0 * FL(i)  + 1.D0/9.D0 * FLm(i)
         enddo
      else 
         print *, 'UseZmvnsScheme, XSecType',XSecType,
     $        'not supported'
         stop
      endif
      
      end

      subroutine UseAcotScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $     x, q2, npts, polarity, XSecType, charge, IDataSet)
C----------------------------------------------------------------
C Calculates F2, FL, XF3, F2c, FLc, F2b, FLb 
C according to ACOT scheme
C
C  Created by Krzysztof Nowak, 23/01/2012
C---------------------------------------------------------------
      implicit none
      include 'ntot.inc'
      include 'datasets.inc'
      include 'steering.inc'
      include 'fcn.inc'
      include 'qcdnumhelper.inc'

C Input:
      double precision x(NPMaxDIS), q2(NPMaxDIS)
      double precision charge, polarity
      integer npts,IDataSet
      character*(*) XSecType
C Output:
      double precision F2(NPMaxDIS), FL(NPMaxDIS), xF3(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS)
      double precision F2b(NPMaxDIS),FLb(NPMaxDIS)

C Additional variables:
      integer icharge, i, idx
      logical UseKFactors
      double precision f123l(4),f123lc(4),f123lb(4)
      
      if (mod(HFSCHEME,10).eq.1) then
         UseKFactors = .true.   !ACOT Full , ACOT Chi, ACOT ZM
!      else
!         UseKFactors = .false.  !ACOT ZM
      endif

c     icharge_in: 0 NC: photon exchange only
c     icharge_in: 4 NC: e+ gamma+gammaZ+Z 
c     icharge_in: 5 NC: e- gamma+gammaZ+Z 
c     icharge_in:-1 CC e-
c     icharge_in:+1 CC e+
      if(XSecType.eq.'CCDIS') then
         icharge = int(charge)
      else if (XSecType.eq.'NCDIS') then
         if (charge.gt.0) icharge = 4
         if (charge.lt.0) icharge = 5

      else if (XSecType.eq.'CHARMDIS') then
         icharge = 0
      else
         print *, 'UseAcotScheme, XSecType', XSecType,
     $        'not supported'
         stop
      endif

      do i=1,npts
         idx =  DATASETIDX(IDataSet,i)
         
         call sf_acot_wrap(x(i),q2(i),
     $        f123l,f123lc,f123lb,
     $        hfscheme, icharge, 
     $        iFlagFCN, idx,
     $        UseKFactors, polarity)
      
         FL(i)  = F123L(4)
         F2(i)  = F123L(2)
         XF3(i) = x(i) * F123L(3)
c         if ((charge.gt.0).and.(XSecType.eq.'NCDIS')) then
c            XF3(i) = - XF3(i)
c         endif

         FLc(i)  = F123Lc(4)
         F2c(i)  = F123Lc(2)

         FLb(i)  = F123Lb(4)
         F2b(i)  = F123Lb(2)
         
      enddo

      end



      subroutine UseRtScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $        x, q2, npts, XSecType, F2gamma, FLgamma, IDataSet)
C----------------------------------------------------------------
C Calculates F2, FL, XF3, F2c, FLc, F2b, FLb 
C according to Robert Thorne scheme
C
C  Created by Krzysztof Nowak, 23/01/2012
C---------------------------------------------------------------
      implicit none
      include 'ntot.inc'
      include 'datasets.inc'
      include 'steering.inc'
      include 'fcn.inc'
      include 'qcdnumhelper.inc'

C Input:
      double precision x(NPMaxDIS), q2(NPMaxDIS)
      double precision F2gamma(NPMaxDIS), FLgamma(NPMaxDIS)
      integer npts,IDataSet
      character*(*) XSecType

C Output:
      double precision F2(NPMaxDIS), FL(NPMaxDIS), xF3(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS)
      double precision F2b(NPMaxDIS),FLb(NPMaxDIS)

C Additional variables:
      integer i, idx
      logical UseKFactors
      Double precision f2pRT,flpRT,f1pRT,rpRT,f2nRT,flnRT,f1nRT,rnRT
      Double precision f2cRT,flcRT,f1cRT,f2bRT,flbRT,f1bRT

C RT code good only for NC case      
      if (XSecType.eq.'CCDIS') return
      
      
      if (HFSCHEME.eq.22) then 
         UseKFactors = .true.    ! RT Fast
      else
         UseKFactors = .false.   ! RT
      endif


C RT does not provide terms beyond gamma exchange. Since they occur at high Q2,
C use QCDNUM to take them into account as a "k"-factor 
C
C  F2_total^{RT} =  F2_{\gamma}^{RT}  *  (  F2_{total}^{QCDNUM}/F2_{\gamma}^{QCDNUM}   
C
      do i=1,npts
         idx =  DATASETIDX(IDataSet,i)
         
           call sfun_wrap(x(i),q2(i)
     $          ,f2pRT,flpRT,f1pRT,
     +          rpRT,f2nRT,flnRT,
     +          f1nRT,rnRT,f2cRT,
     +          flcRT,f1cRT,f2bRT,
     +          flbRT,f1bRT
           ! Input:
     $          ,iFlagFCN,idx    ! fcn flag, data point index
     $          ,F2Gamma,FLGamma
     $          ,UseKFactors
     $          )
           
      
C     Replace F2,FL from QCDNUM by RT values
C     Keep xF3 from QCDNUM
         F2(i) = F2pRT * (F2(i)/F2Gamma(i))
         FL(i) = FLpRT * (FL(i)/FLGamma(i))
         F2c(i) = f2cRT
         FLc(i) = flcRT
         F2b(i) = f2bRT
         FLb(i) = flbRT
         
      enddo
      end




      subroutine UseHqstfScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $     x, q2, npts, XSecType)
C----------------------------------------------------------------
C     Calculates F2, FL, XF3, F2c, FLc, F2b and FLb using HQST scheme
C
C     Created by Krzysztof Nowak, 31/01/2012
C---------------------------------------------------------------
      implicit none
      include 'qcdnumhelper.inc'
      
C Input:
      double precision X(NPMaxDIS),Q2(NPMaxDIS)
      integer npts
      character*(*) XSecType
      
C Output: 
      double precision F2(NPMaxDIS), FL(NPMaxDIS), xF3(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS),F2b(NPMaxDIS),FLb(NPMaxDIS)

C Additional Variables:
      double precision NC2FHF(-6:6)
      integer i
  
C     HQSTF code good only for NC case      
      if (XSecType.eq.'CCDIS') return

      NC2FHF = 4.D0/9.D0 * CNEP2F  + 1.D0/9.D0 * CNEM2F
      CALL HQSTFUN(2,1,NC2FHF,X,Q2,F2c,npts,0)
      CALL HQSTFUN(1,1,NC2FHF,X,Q2,FLc,npts,0)
      CALL HQSTFUN(2,-2,NC2FHF,X,Q2,F2b,npts,0)
      CALL HQSTFUN(1,-2,NC2FHF,X,Q2,FLb,npts,0)
      
      do i=1,npts
         F2(i) = F2(i) + F2c(i) + F2b(i) 
         FL(i) = FL(i) + FLc(i) + FLb(i)
      enddo

      end




      subroutine UseABKMFFScheme(F2, FL, XF3, F2c, FLc, F2b, FLb, 
     $     x, q2, npts, XSecType, F2gamma, FLgamma, IDataSet)
C----------------------------------------------------------------
C     Calculates F2, FL, XF3, F2c, FLc, F2b and FLb using ABKM FF scheme
C---------------------------------------------------------------
      implicit none
      include 'ntot.inc'
      include 'datasets.inc'
      include 'steering.inc'
      include 'fcn.inc'
      include 'qcdnumhelper.inc'
      
C Input:
      double precision X(NPMaxDIS),Q2(NPMaxDIS)
      double precision F2gamma(NPMaxDIS), FLgamma(NPMaxDIS)
      integer npts,IDataSet
      character*(*) XSecType
      
C Output: 
      double precision F2(NPMaxDIS), FL(NPMaxDIS), xF3(NPMaxDIS)
      double precision F2c(NPMaxDIS),FLc(NPMaxDIS),F2b(NPMaxDIS),FLb(NPMaxDIS)

C ABKM 
      Double precision f2abkm,flabkm,f3abkm
      Double precision f2cabkm,flcabkm,f3cabkm
      Double precision f2babkm,flbabkm,f3babkm

C Additional Variables:
      double precision NC2FHF(-6:6)
      integer i, idx
  
C     HQSTF code good only for NC case      
      if (XSecType.eq.'CCDIS') return

      NC2FHF = 4.D0/9.D0 * CNEP2F  + 1.D0/9.D0 * CNEM2F
      CALL HQSTFUN(2,1,NC2FHF,X,Q2,F2c,npts,0)
      CALL HQSTFUN(1,1,NC2FHF,X,Q2,FLc,npts,0)
      CALL HQSTFUN(2,-2,NC2FHF,X,Q2,F2b,npts,0)
      CALL HQSTFUN(1,-2,NC2FHF,X,Q2,FLb,npts,0)
      

      do i=1,npts
         idx =  DATASETIDX(IDataSet,i)
         
         call sf_abkm_wrap(x(i),q2(i)
     $        ,f2abkm,flabkm,f3abkm,f2cabkm,flcabkm,f3cabkm
     $        ,f2babkm,flbabkm,f3babkm,3,1,22)

c correct for Z contribution (as in RT case)           
           F2(i) = f2abkm * (F2(i)/F2Gamma(i))  + f2cabkm + f2babkm
           FL(i) = flabkm * (FL(i)/FLGamma(i)) + flcabkm + flbabkm
C     Keep xF3 from QCDNUM FF (wraper will return zero)

c           write(6,*) 'ABKM: F2sum,F2tot,F2g,F2c,F2b', i,F2,F2ab,f2abkm,
c     $ f2cabkm,f2babkm
      enddo
      end