/* main.cpp Sept. 5, 2006, 3.45PM  */

/**************************************************************************************/
/* Include libraries and header files												  */
/**************************************************************************************/
#include "BoundaryConditions.h"
#include "Trees.h"
#include "Cloud.h"
#include "Input.h"

#include <stdlib.h>
/**************************************************************************************/

/**************************************************************************************/
/* Declare functions																  */
/**************************************************************************************/
/* SAM functions. */
void	SetPathName(); /* Sets the subfolder to which interim results are written. */
void 	SortPathName(int numLinks); /* Sets the subfolder to which final results are written. */

void    InitMatrices(SizeGrid);
/**************************************************************************************/

/**************************************************************************************/
/* Main																				  */
/**************************************************************************************/
int main()
{
    /******************************************************************************/
    /* Initialisation & Preparation of Summary file								  */
    /******************************************************************************/
    time_t seconds;
    time(&seconds);
    srand((unsigned int) seconds);
    
    Var::ThresholdOvershoot		= 10;                                               // % by which the Einit must be exceeded
    // arbitrarily fixed to 1 in this case.
    Var::BCtype					= TIN_CAN;                                          // Boundary conditions
    Var::LoadingType            = SET_CHARGES;                                    // Charge loading type
    Var::InitiationType			= AT_REL_EMAX;                                    // Initiation type
    
    Var::step3d                 = -100;                                             // rho, E, V is calculated and store every step3d
    // = 0 3-D values never calculated
    Var::AddNew					= true;                                             // Channel is allowed to propagate: Y/N
    Var::isBndXingPossible		= false;                                            // Channel is allowed to cross boundaries: Y/N
    Var::isChannelEquipotential	= true;                                             // Channel potential ensure charges neutrality: Y/N
    Var::isFlashAccoutedInBC 	= false;                                            // Channel charge is accounted for in  BC: Y/N
    Var::isInitiationPossible	= false;                                            // Init. possible in domain after charge load: Y/N
    Var::isLinkXingPossible		= false;                                            // Channels crosses are allowed: Y/N
    Var::isRsDeveloped			= true;                                             // Return stroke development: Y/N
    Var::isInitiationPrevented	= false;                                            // Only simulate cloud electrical structure
    Var::isEsEnergyCalculated	= true;                                            // Electrostatic energy calculated at each step: Y/N
    Var::isBCerrorCalculated	= true;                                            // Error at boundary is calculated at each step: Y/N
    
    Var::ThresholdOvershoot		/= 100;                                             // Convert % into decimal
    
    ListCharge::iterator	it;													// Table with all parameters of the charge configuration
    clock_t					startTime;
    FILE *					file = nullptr;
    
    startTime = clock(); /* Time current simulation run. */
    /* Set the interim results subfolder path name.  The path name is based on the starting time
     * of the current simulation run.
     */
    
    char logName[200];
    IO::setPathName((char*)"results");
    IO::getPathName(logName);
    strcat(logName, "/summary.txt");
    
    if (access(logName, F_OK)) {
        Var::isNewRun = true;
    } else {
        Var::isNewRun = false;
    }
    
    if(Var::isNewRun) {
        file = IO::openFile((char *)"summary.txt", "w");
        IO::print(file,"ii: Starting new equipotential lightning discharge simulation\n");
        
        IO::print(file, "..: Reading grid size (N).\n");
        Var::N.init(161,161,81);
        IO::print(file, "ii:\t Grid dimensions      : [" + to_string(Var::N.x) + ", " + to_string(Var::N.y) + ", " + to_string(Var::N.z) + "]\n");

        IO::print(file, "..: Reading domain size (L)\n");
        Var::L.init(40e+3,40e+3,20e+3);
        IO::print(file, "ii:\t Total simulation size: [" + to_string(Var::L.x/1e3) + " km, " + to_string(Var::L.y/1e3) + " km, " + to_string(Var::L.z/1e3) + " km]\n");

        IO::print(file, "..: Reading grid resolution (d)\n");
        Var::d.init(Var::L,Var::N);
        IO::print(file, "ii:\t Discretized lengths  : [" + to_string(Var::d.x) + " m, " + to_string(Var::d.y) + " m, " + to_string(Var::d.z) + " m]\n");

        InitMatrices(Var::N);
        
        IO::print(file, "..: Reading ground altitude (z_gnd)\n");
        Var::z_gnd   = 0e3;
        IO::print(file, "ii: Ground altitude: " + to_string(Var::z_gnd/1e3) + "km\n");
        
        Var::z_shift = 0e3;                                                            // _m Vertical displacement of the cloud
        Var::y_shift = 0e3;                                                            // _m Horizontal influence of a windshear
        
        //    We assume that the first link is somehow established, then only the propagation threshold needs to be exceeded to develop the flash.
        IO::print(file, "..: Reading critical fields (Ec,Eth+,Eth-,Vd+,Vd-)\n");
        //Var::Ec.init(1*2.16e+5,1*2.16e+5,1*-2.16e+5, Var::z_gnd,Var::d,Var::N,1);
        //Var::Vd.init(0.21e+5,-0.21e+5, Var::z_gnd,Var::d,Var::N,1);
        Var::Ec.init(.1e+5,.1e+5,-.1e+5, Var::z_gnd,Var::d,Var::N,1);
        Var::Vd.init(0.e+5,-0.e+5, Var::z_gnd,Var::d,Var::N,1);
        IO::print(file, "..: Critical fields read (Ec,Eth+,Eth-,Vd+,Vd-)\n");

        IO::print(file, "..: Reading initiation point (InitPoint)\n");
        Var::InitX    = Var::L.x/2;
        Var::InitY    = Var::L.y/2;
        Var::InitZ    = 8e3;
        Var::InitR    = 0;
        Var::InitiationPoint.init((int)round(Var::InitX/Var::d.x), (int)round(Var::InitY/Var::d.y),(int)round(Var::InitZ/Var::d.z));
        // Initiation point with coordinates expressed as i,j,k and not x,y,z
        IO::print(file, "ii: Discharge initiated at: [" + to_string(Var::InitX/1e3) + " " + to_string(Var::InitY/1e3) + " " + to_string(Var::InitZ/1e3) + "] km; Radius of init. region: " + to_string(Var::InitR/1e3) + "km\n");

    } else {
        file = IO::openFile((char *)"summary.txt", "a");
        IO::print(file,"ii: Resuming previous discharge simulation\n");
        
        IO::print(file, "..: Reading grid size (N).\n");
        IO::read(Var::N,(char *)"Nxyz.dat");
        IO::print(file, "ii:\t Grid dimensions      : [" + to_string(Var::N.x) + ", " + to_string(Var::N.y) + ", " + to_string(Var::N.z) + "]\n");
        
        InitMatrices(Var::N);
        
        IO::print(file, "..: Reading grid resolution (d)\n");
        IO::read(Var::d,(char *)"dxyz.dat");
        IO::print(file, "ii:\t Discretized lengths  : [" + to_string(Var::d.x) + " m, " + to_string(Var::d.y) + " m, " + to_string(Var::d.z) + " m]\n");
        
        IO::print(file, "..: Calculating domain size (L)\n");
        Var::L.init((Var::N.x-1)*Var::d.x,(Var::N.y-1)*Var::d.y,(Var::N.z-1)*Var::d.z);
        IO::print(file, "ii:\t Total simulation size: [" + to_string(Var::L.x/1e3) + " km, " + to_string(Var::L.y/1e3) + " km, " + to_string(Var::L.z/1e3) + " km]\n");
        
        IO::print(file, "..: Reading ground altitude (z_gnd)\n");
        IO::read(Var::z_gnd,(char*)"z_gnd.dat");
        IO::print(file, "ii: Ground altitude: " + to_string(Var::z_gnd/1e3) + "km\n");
        
        IO::print(file, "..: Reading critical fields (Ec,Eth+,Eth-,Vd+,Vd-)\n");
        IO::read(Var::Ec.initiation,	(char*)"Einitiation.dat");
        IO::read(Var::Ec.negative,		(char*)"EthNegative.dat");
        IO::read(Var::Ec.positive,		(char*)"EthPositive.dat");
        IO::read(Var::Vd.negative,		(char*)"VdNegative.dat");
        IO::read(Var::Vd.positive,		(char*)"VdPositive.dat");
        IO::print(file, "..: Critical fields read (Ec,Eth+,Eth-,Vd+,Vd-)\n");
        
        IO::print(file, "..: Reading 3-D output increment step (step3d)\n");
        IO::read(Var::step3d,(char*)"step3d.dat");
        IO::print(file, "..: 3-D output increment step (step3d): " + to_string((int)Var::step3d) + "\n");
        
        IO::print(file,  "..: Finding last recorded iteration.\n");
        int LastStep = 0;
        char filename[200];
        FILE * fp;
        sprintf(filename,"phi3d%d.dat",LastStep);
        while((fp=IO::openFile(filename,"r")))
        {
            LastStep += Var::step3d;
            sprintf(filename,"phi3d%d.dat",LastStep);
            fclose(fp);
        }
        LastStep -= Var::step3d; // This removes the last iteration count from the while loop
        IO::print(file, "ii: Resuming at step " + to_string(LastStep) + ".\n");
        Var::NumLinks = LastStep;
        Var::isInitiationPossible	= true;
        
        IO::print(file, "..: Reading electric potential before the flash (phiNum)\n");
        IO::read(Var::phiNum, (char*)"phiNumBF.dat");
        IO::print(file, "..: Electric potential before the flash read\n");
        
        IO::print(file, "..: Reading electric field before the flash (EzNum)\n");
        IO::read(Var::EzNum, (char*)"EnumBF.dat");
        IO::print(file, "..: Electric field before the flash read\n");
        
        /* Reading previous discharge. */
        IO::print(file, "..: Reading initiation point (InitPoint)\n");
        CMatrix1D M;
        IO::read(M,(char*)"InitPoint.dat");
        Var::InitX = M[0];
        Var::InitY = M[1];
        Var::InitZ = M[2];
        Var::InitR = M[3];
        Var::InitiationPoint.init((int)round(Var::InitX/Var::d.x), (int)round(Var::InitY/Var::d.y),(int)round(Var::InitZ/Var::d.z));
        IO::print(file, "ii: Discharge initiated at: [" + to_string(Var::InitX/1e3) + " " + to_string(Var::InitY/1e3) + " " + to_string(Var::InitZ/1e3) + "] km; Radius of init. region: " + to_string(Var::InitR/1e3) + "km\n");
        
        sprintf(filename,"phi3d%d.dat",0);
        IO::print(file, "..: Initializing ambient electric potential matrix (phi<<"+(string)filename+").\n");
        IO::read(Var::phi_amb,filename);
        
        sprintf(filename,"phi3d%d.dat",LastStep);
        IO::print(file, "..: Initializing  electric potential matrix (phi<<"+(string)filename+").\n");
        IO::read(Var::phi,filename);
        
        Var::phi_cha = Var::phi - Var::phi_amb;
        
        sprintf(filename,"Un3d%d.dat",LastStep);
        IO::print(file, "..: Initializing map of fixed potential points (Un<<"+(string)filename+")\n");
        IO::read(Var::Un,filename);
        
        sprintf(filename,"rho3d%d.dat",LastStep);
        IO::print(file, "..: Initializing electric charge density matrix (rho<<"+(string)filename+")\n");
        Var::C.reset(Var::d,Var::N);
        IO::read(Var::rho,filename);
        for(int ii=0; ii<Var::N.x; ii++) for(int jj=0; jj<Var::N.y; jj++) for(int kk=0; kk<Var::N.z; kk++)
            Var::rho[ii][jj][kk] *=1e-9;
        
        
        IO::print(file, "..: Reading fractal structure (links)\n");
        static	ListLink    PreviousLinks;
        static	ListDouble	PreviousChannelPotential;
        static	ListDouble	PreviousCarriedCharge;
        CMatrix2D           PreviousDischargeDipoleMoment;
        Vector              P;
        
        IO::read(PreviousLinks                  , (char*)"EstablishedLinks.dat"     );
        IO::read(PreviousChannelPotential       , (char*)"ChannelPotentials.dat"    );
        IO::read(PreviousCarriedCharge          , (char*)"CarriedCharge.dat"        );
        IO::read(PreviousDischargeDipoleMoment  , (char*)"DischargeDipoleMoment.dat");
        
        ListLink::iterator   itLink     = PreviousLinks.begin();
        ListDouble::iterator itVo       = PreviousChannelPotential.begin();
        ListDouble::iterator itQ        = PreviousCarriedCharge.begin();
        
        int n=0;
        while (n<LastStep) {
            Var::EstablishedLinks.push_back(*itLink);
            itLink++;
            Var::ChannelPotential.push_back(*itVo);
            itVo++;
            Var::CarriedCharge.push_back(*itQ);
            itQ++;
            
            P.x = PreviousDischargeDipoleMoment[n][0];
            P.y = PreviousDischargeDipoleMoment[n][1];
            P.z = PreviousDischargeDipoleMoment[n][2];
            Var::DischargeDipoleMoment.push_back(P);
            n++;
        }
        
        if(Var::isEsEnergyCalculated) {
            n=0;
            static  ListDouble      PreviousEsEnergy;
            ListDouble::iterator    itEs = PreviousEsEnergy.begin();
            IO::read(PreviousEsEnergy, (char*)"EsEnergy.dat");
            while (n<LastStep) {
                Var::EsEnergy.push_back(*itEs);
                itEs++;
                n++;
            }
        }
        if(Var::isBCerrorCalculated) {
            Vector      BndError;
            CMatrix2D   PreviousBndUpdateErrors;
            IO::read(PreviousBndUpdateErrors, (char*)"BndError.dat");
            n=0;
            while(n<LastStep) {
                BndError.x = PreviousBndUpdateErrors[n][0];
                BndError.y = PreviousBndUpdateErrors[n][1];
                BndError.z = PreviousBndUpdateErrors[n][2];
                Var::BndUpdateErrors.push_back(BndError);
                n++;
            }
        }
    }
    
    if(Var::isNewRun){
        if (Var::LoadingType == SET_CHARGES) {
            IO::print(file, "..: Setting charge layers\n");
            // Q (C) Charge content; Xq,Yq,Zq (m) Charge center coordinate; R,H (m) Size of the charge center //
            Var::Q =     15;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2-Var::y_shift/2;	Var::Zq = 3e+3+Var::z_shift; Var::Rq1 = 5.00e+3;	Var::Rq3 = 6.0e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            Var::Q =    -15;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2-Var::y_shift/2;	Var::Zq = 8e+3+Var::z_shift; Var::Rq1 = 10.00e+3;	Var::Rq3 = 4.0e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            
            for (it=Var::ChargeCfg.begin(); it!=Var::ChargeCfg.end(); it++)
                Var::C += *it;
            IO::print(file, "++: Finished Setting charge layers!\n");
        } else if (Var::LoadingType == CURRENTS) {
            IO::print(file, "..: Setting charge layers geometry\n");
            // Q (C) Charge content; Xq,Yq,Zq (m) Charge center coordinate; R,H (m) Size of the charge center //
            Var::Q =    0.;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2-Var::y_shift/2;	Var::Zq = 2.00e+3+Var::z_shift; Var::Rq1 = 1.50e+3;	Var::Rq3 = 1.50e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            Var::Q =	0.;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2-Var::y_shift/2;	Var::Zq = 3.75e+3+Var::z_shift; Var::Rq1 = 3.00e+3;	Var::Rq3 = 1.50e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            Var::Q =    0.;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2+Var::y_shift/2;	Var::Zq = 6.75e+3+Var::z_shift; Var::Rq1 = 4.00e+3;	Var::Rq3 = 1.50e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            Var::Q =    0;	Var::Xq = Var::L.x/2;	Var::Yq = Var::L.y/2+Var::y_shift/2;	Var::Zq = 8.00e+3+Var::z_shift; Var::Rq1 = 4.00e+3;	Var::Rq3 = 0.50e+3;
            Var::C.disk(Var::Q, Var::Xq,Var::Yq,Var::Zq, Var::Rq1,Var::Rq3, Var::d,Var::N);
            Var::ChargeCfg.push_back(Var::C);
            IO::print(file, "++: Finished setting charge layers geometry!\n");
            
            IO::print(file, "..: Loading charge layers\n");
            Var::I1			= 1.5;														// _A Loading current I1
            Var::I2			= -90e-3;													// _A Loading current I2
            Var::Iscreen	= 0*664e-3;													// _A Screening current Iscreen
            Cloud::LoadTripole(file, Var::I1,Var::I2,Var::Iscreen);
            IO::print(file, "++: Finished loading charge layers!\n");
        } else if (Var::LoadingType == SET_POTENTIAL) {
            IO::print(file, "..: Setting potential\n");
            //        C.reset(d,N);
            //        for(it=ChargeCfg.begin() ; it!=ChargeCfg.end() ; it++)
            //            C += *it;
            //       
            //        ApplyBC(BCtype,phi,C.rho,d,N);
            
            Var::Eo  = 1e5; // _V/_m
            Var::Vo  = -Var::Eo*(Var::N.z-1)*Var::d.z;
            Var::Xp  = Var::L.x/2;
            Var::Yp  = Var::L.y/2;
            Var::Zp  = Var::L.z;
            Var::Rp1 = Var::L.x;
            Var::Rp2 = Var::L.y;
            Var::Rp3 = Var::d.z;
            
            
            Var::V.init(Var::Vo, Var::Xp,Var::Yp, Var::Zp,Var::Rp1,Var::Rp2,Var::Rp3, Var::d,Var::N);
            
            //        for(int ii=0 ; ii<N.x ; ii++) for(int jj=0 ; jj<N.y ; jj++)    for(int kk=0 ; kk<N.z ; kk++)
            //        {phi[ii][jj][kk]=-Eo*kk*d.z;};
            
            
            IO::print(file, "++: Finished setting potential!\n");
        } else if(Var::LoadingType == FROM_FILE) {
            double Lr = 9e3;
            Var::L.z  = 21e3;
            Var::C.init((char*)"results/rho2d280000.dat",(char*)"results/rhos2d280000.dat",(char*)"results/d.dat",(char*)"results/N.dat",(char*)"results/z_gnd.dat", Var::d, Var::N, Var::z_gnd, Lr, Var::L.z);
            Var::ChargeCfg.push_back(Var::C);
            
            //		for(int ii=0; ii<Var::N.x ; ii++) for(int jj=0; jj<Var::N.y ; jj++) for(int kk=0; kk<Var::N.z ; kk++)
            //			Var::C.rho[ii][jj][kk] *= 1e9;
            
            Var::L.init(Var::N.x*Var::d.x,	Var::N.y*Var::d.y,	Var::L.z);				// _m	Size of the simulation domain
            
            InitMatrices(Var::N);
            
            //	We assume that the first link is somehow established, then only the propagation threshold needs to be exceeded to develop the flash.
            Var::Ec.init(2.10e+5,2.16e+5,-2.16e+5, Var::z_gnd,Var::d,Var::N,1);			// _V/m Initiation-, Positive channel propagation-, Negative channel propagation-threshold
            Var::Vd.init(0e+5,-0e+5, Var::z_gnd,Var::d,Var::N,1);							// _V/m Voltage drop in positive, negative channels
            Var::InitX	= Var::L.x/2;													// _m X-coordinate of initiation point
            Var::InitY	= Var::L.y/2;													// _m Y-coordinate of initiation point
            Var::InitZ	= 9.25e3;														// _m Z-coordinate of initiation point
            
            Var::InitiationPoint.init((int)round(Var::InitX/Var::d.x), (int)round(Var::InitY/Var::d.y),(int)round(Var::InitZ/Var::d.z));
            // Initiation point with coordinates expressed as i,j,k and not x,y,z
            /*
             IO::write(Var::C.rho, "rho.dat");
             IO::write(Var::z_gnd, "z_gnd.dat");
             IO::write(Var::d.x, Var::d.y, Var::d.z, "dxyz.dat");
             IO::write(Var::N.x, Var::N.y, Var::N.z, "Nxyz.dat");
             
             CMatrix1D rrho3D(Var::N.x*Var::N.y*Var::N.z);
             int nn = 0;
             for(int kk=0 ; kk<Var::N.z ; kk++) for(int jj=0 ; jj<Var::N.y ; jj++) for(int ii=0 ; ii<Var::N.x ; ii++)
             {
             rrho3D[nn]		= Var::C.rho[ii][jj][kk]*1e9; //_nC
             nn++;
             };
             IO::write(rrho3D,"rho3d0.dat");
             */
            // Endof Charge center parameters //
            
            
            IO::print(file, "..: Loading charge layers\n");
            
            Var::C.reset(Var::d,Var::N);
            
            printf("ii:\t Charge layers are as follows:\n");
            for(it=Var::ChargeCfg.begin() ; it!=Var::ChargeCfg.end() ; it++)
            {
                cout<<*it;
                Var::C += *it;
            }
            IO::print(file, "++: Finished loading charge layers!\n");
        }
    }
    IO::print(file, "..: Initializing domain\n");
    if(Var::LoadingType == SET_POTENTIAL) {
        bool UniformE = true;
        BC::AddUniformE(UniformE, Var::Eo, Var::phi, Var::Un, Var::L, Var::d, Var::N);
        Var::SOR.init(Var::phi,Var::epsilon, Var::MaxStep, Var::d, Var::N, Var::V, Var::Un);
    } else {
        Var::SOR.init(Var::phi, Var::epsilon, Var::MaxStep, Var::d, Var::N, Var::C, Var::Un);
        BC::Apply(Var::BCtype,Var::phi,Var::C.rho,Var::d,Var::N);
    }
    
    Var::SOR.Solve(Var::d,Var::N,Var::Un,Var::phi);
    IO::print(file, "++: Domain initialized!\n");
    
    IO::print(file, "..: Fractal structure read\n");
    
    
    IO::print(file, "ii: Calculating electrostatic energy before the discharge\n");
    Var::Eps_bf=0;
    for(int ii=0 ; ii<Var::N.x ; ii++) for(int jj=0 ; jj<Var::N.y ; jj++) for(int kk=0 ; kk<Var::N.z ; kk++)
        Var::Eps_bf += eps0*pow(foo::Eijk(ii,jj,kk,Var::phi,Var::d,Var::N)[0],2)/2*Var::d.x*Var::d.y*Var::d.z;
    IO::print(file, "ii: Electrostatic energy after the discharge: " + to_string(Var::Eps_af) + " J\n");
    
    IO::print(file, "ii: Finding potential extrema\n");
    Var::rho		= foo::Globalrho(Var::phi,Var::d,Var::N);
    Var::Qtot_bf	= foo::TotalCharge(Var::rho,Var::d,Var::N);
    Var::phi.MinMax(Var::Vmin,Var::Vmax);
    Var::rho.MinMax(Var::rhoAmbMin,Var::rhoAmbMax);
    Var::phi_amb	= Var::phi;
    IO::print(file, "ii:\t Extrema: [" + to_string(Var::Vmin) + " ," + to_string(Var::Vmax) + "]\n");
    
    if(Var::isNewRun) {
        IO::print(file,	"..: Initiating the tree\n");
        Var::isInitiationPossible = Tree::Initiate(file, Var::InitiationType, Var::InitiationPoint);
    }
    if(Var::isInitiationPrevented == true)
    {
        Var::isInitiationPossible = false;
        IO::print(file,	"Initiation not allowed\n");
    }
    IO::print(file, "++: Finished initiating the tree!\n");
    
    IO::print(file, "..: Growing the tree\n");
    // Var::NumLinks = 0;
    if(Var::isInitiationPossible)
    {
        Tree::Grow(file, Var::AddNew);
    }
    IO::print(file, "ii: Finished growing the tree\n");
    
    IO::print(file, "ii: Calculating electrostatic energy after the discharge\n");
    Var::Eps_af=0;
    for(int ii=0 ; ii<Var::N.x ; ii++) for(int jj=0 ; jj<Var::N.y ; jj++) for(int kk=0 ; kk<Var::N.z ; kk++)
        Var::Eps_af += eps0*pow(foo::Eijk(ii,jj,kk,Var::phi,Var::d,Var::N)[0],2)/2*Var::d.x*Var::d.y*Var::d.z;
    Var::eFlux = foo::eFieldFlux(Var::phi,Var::d,Var::N);
    IO::print(file, "ii: Electrostatic energy after the discharge: " + to_string(Var::Eps_af) + " J\n");
    
    IO::print(file, "..: Estimating the field and storing results\n");
    
    Tree::StoreData(file); /* Store the final data (still in the interim results folder). */
    //SortPathName(Var::NumLinks); /* Create the final results folder. */
    /* Move the final results to the final results folder.  This function call also deletes the interim
     * results folder.
     */
    //IO::moveFromOldDirectory();
    IO::print(file, "++: Finished estimating the field and storing results!\n");
    IO::print(file, "..: Summarizing results\n");
    
    if(Var::BCtype == TIN_CAN)
        IO::print(file, "ii:\t Closed Boundary Conditions\n");
    else if(Var::BCtype == OPEN_BC)
        IO::print(file,	"ii:\t Open Boundary Conditions\n");
    else if(Var::BCtype == G_G)
        IO::print(file,	"ii:\t Moving Capacitor Plate Boundary Conditions\n");
    else if(Var::BCtype == FREE_SPACE)
        IO::print(file,	"ii:\t Free Space Boundary Conditions\n");
    
    if(Var::isFlashAccoutedInBC==true)
        IO::print(file,	"ii:\t Updated Boundary Conditions\n");
    else
        IO::print(file,	"ii:\t Fast Boundary Conditions (no update)\n");
    
    
    /************************************************
     *	Changed "%f" to "%lf"						*
     *************************************************/
    
    IO::print(file,	"ii:\t Threshold Overshoot  : " + to_string(Var::ThresholdOvershoot*100) + " %\n");
    IO::print(file, "ii:\t Grid dimensions      : [" + to_string(Var::N.x) + ", " + to_string(Var::N.y) + ", " + to_string(Var::N.z) + "]\n");
    IO::print(file, "ii:\t Discretized lengths  : [" + to_string(Var::d.x) + " m, " + to_string(Var::d.y) + " m, " + to_string(Var::d.z) + " m]\n");
    IO::print(file, "ii:\t Total simulation size: [" + to_string(Var::L.x/1e3) + " m, " + to_string(Var::L.y/1e3) + " km, " + to_string(Var::L.z/1e3) + " km]\n");
    
    IO::print(file,	"ii:\t El.Stat. Energy before channel propagation    : " + to_string(Var::Eps_bf)        + " J\n");
    IO::print(file,	"ii:\t El.Stat. Energy after  channel propagation    : " + to_string(Var::Eps_af)        + " J\n");
    IO::print(file,	"ii:\t Total    Charge before channel propagation    : " + to_string(Var::Qtot_bf)       + " C\n");
    IO::print(file,	"ii:\t Total    Charge after  channel propagation    : " + to_string(Var::Qtot_af)       + " C\n");
    IO::print(file,	"ii:\t Positive Charge in the channel                : " + to_string(Var::QchannelPlus)  + " C\n");
    IO::print(file,	"ii:\t Negative Charge in the channel                : " + to_string(Var::QchannelMinus) + " C\n");
    
    IO::print(file,	"ii:\t Flux of electric field through the boundaries : " + to_string(Var::eFlux(0)) + " " + to_string(Var::eFlux(1)) + " " + " " + to_string(Var::eFlux(2)) + " " + to_string(Var::eFlux(3)) + " " + to_string(Var::eFlux(4)) + " " + to_string(Var::eFlux(5)) + " " + to_string(Var::eFlux(6)) + " C\n");
    IO::print(file,	"ii:\t Equivalent charge                             : " + to_string(Var::Qtot_af)       + " C\n");
    
    printf(			"ii:\t Charge layers:\n");
    for(it=Var::ChargeCfg.begin() ; it!=Var::ChargeCfg.end() ; it++)
    {
        cout<<*it;
        // write(*it, fname);
    }
    
    clock_t endTime = clock();
    clock_t runTime = endTime - startTime;
    
    IO::print(file,	"ii: Total run time: " + to_string((double)runTime/CLOCKS_PER_SEC) + " s\n");
    fclose(file);
    
    /* Clear the charge structure so that it can be set during the next simulation run.
     * Hence, it is possible to modify the input file (from the 'buck' program') so that
     * subsequent simulation runs can work with different source data.
     */
    Var::ChargeCfg.clear();
    return 1;
}
/**********************************************************************************/

void InitMatrices(SizeGrid N)
{
    Var::E.init(N.x,N.y,N.z);									// _V/m	Total electric field
    Var::phi.init(N.x,N.y,N.z);									// _V	Total electric potential
    Var::phi_cha.init(N.x,N.y,N.z);								// _V	Channel electric potential
    Var::phi_amb.init(N.x,N.y,N.z);								// _V	Cloud electric potential
    Var::rho.init(N.x,N.y,N.z);									// _C/m3	total charge density
    Var::Un.init(N.x,N.y,N.z);									// Map of occupied grid points
    Var::phiNum.init(N.z);													// _V	Total electric potential on a vertical axis in the center of simulation domain
    Var::EzNum.init(N.z);													// _V/m	Total electric field on a vertical axis in the center of simulation domain
    
}
