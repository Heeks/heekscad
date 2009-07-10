/*
 * solve.cpp
 *
 *  Created on: May 4, 2009
 *      Author: Jonathan George
 *      This
 *      Copyright (c) 2009, Jonathan George
 *      This program is released under the BSD license. See the file COPYING for details.
 *
 */

#include "solve.h"
#include <cmath>
#include <stdlib.h>
#include <sstream>

using namespace std;


int solve(double  **x, int xLength, constraint * cons, int consLength, int isFine)
{
	Solver *s = new Solver(xLength);
	int ret = s->solve(x,cons,consLength,isFine);
	if(ret == succsess)
		s->Unload();
	delete s;
	return ret;
}

Solver::Solver(int xLength)
{
   this->xLength = xLength;
   origSolution = new double[xLength];
   grad = new double[xLength]; //The gradient vector (1xn)
   s = new double[xLength]; //The current search direction
   xold = new double[xLength]; //Storage for the previous design variables
   deltaX = new double[xLength];
   gradnew = new double[xLength];
   gamma = new double[xLength];
   gammatDotN = new double[xLength];
   FirstSecond = new double*[xLength];
   deltaXDotGammatDotN = new double*[xLength];
   gammatDotDeltaXt = new double*[xLength];
   NDotGammaDotDeltaXt = new double*[xLength];
   N = new double*[xLength];

   for(int i=0; i < xLength; i++)
   {
       FirstSecond[i] = new double[xLength];
       deltaXDotGammatDotN[i] = new double[xLength];
       gammatDotDeltaXt[i] = new double[xLength];
       NDotGammaDotDeltaXt[i] = new double[xLength];
       N[i] = new double[xLength]; //The estimate of the Hessian inverse
   }
   
}

Solver::~Solver()
{
        delete s;
        for(int i=0; i < xLength; i++)
        {
                delete N[i];
                delete FirstSecond[i];
                delete deltaXDotGammatDotN[i];
                delete gammatDotDeltaXt[i];
                delete NDotGammaDotDeltaXt[i];

        }
        delete N;
        delete FirstSecond;
        delete deltaXDotGammatDotN;
        delete gammatDotDeltaXt;
        delete NDotGammaDotDeltaXt;
        delete origSolution;

        delete grad;
        delete xold;
        delete gammatDotN;
		delete x;
}

int Solver::solve(double  **xin,constraint * cons, int consLength, int isFine)
{
		xsave = xin;

		for(int i=0; i < xLength; i++)
		{
			parms[xin[i]] = 1;
		}

		for(int i=0; i < consLength; i++)
		{
			Load(cons[i]);
		}

		xLength = GetVectorSize();
		x = new double[xLength];
		for(int i=0; i < xLength; i++)
			x[i] = GetInitialValue(i);


        std::stringstream cstr;
        double convergence,pert ;
        //Save the original parameters for later.
        for(int i=0;i<xLength;i++)
        {
                origSolution[i]=x[i];
        }

        if(isFine>0) convergence = XconvergenceFine;
        else convergence = XconvergenceRough;
        //integer to keep track of how many times calc is called
        int ftimes=0;
        //Calculate Function at the starting point:
        double f0;
		f0 = GetError();
        if(f0<smallF) return succsess;
        ftimes++;
        //Calculate the gradient at the starting point:

        //Calculate the gradient
        //gradF=x;
        double norm,first,second,temper; //The norm of the gradient vector
        double f1,f2,f3,alpha1,alpha2,alpha3,alphaStar;
        norm = 0;
        pert = f0*pertMag;
        for(int j=0;j<xLength;j++)
        {
                temper= x[j];
                x[j]= temper-pert;
                first = GetError();
                x[j]= temper+pert;
                second = GetError();
                grad[j]=.5*(second-first)/pert;
                ftimes++;
#ifdef DEBUG
                cstr << "gradient: " << grad[j];
                debugprint(cstr.str());
                cstr.clear();
#endif
                x[j]=temper;
                norm = norm+(grad[j]*grad[j]);
        }
        norm = sqrt(norm);
        //Estimate the norm of N

        //Initialize N and calculate s
        for(int i=0;i<xLength;i++)
        {
                for(int j=0;j<xLength;j++)
                {
                        if(i==j)
                        {
                                //N[i][j]=norm; //Calculate a scaled identity matrix as a Hessian inverse estimate
                                //N[i][j]=grad[i]/(norm+.001);
                                N[i][j]=1;
                                s[i]=-grad[i]; //Calculate the initial search vector

                        }
                        else N[i][j]=0;
                }
        }
        double fnew;
        fnew=f0+1;      //make fnew greater than fold
        double alpha=1; //Initial search vector multiplier

        double fold;
        for(int i=0;i<xLength;i++)
        {
                xold[i]=x[i];//Copy last values to xold
        }

        ///////////////////////////////////////////////////////
        /// Start of line search
        ///////////////////////////////////////////////////////

        //Make the initial position alpha1
        alpha1=0;
        f1 = f0;

        //Take a step of alpha=1 as alpha2
        alpha2=1;
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alpha2*s[i];//calculate the new x
        }
        f2 = GetError();
        ftimes++;

        //Take a step of alpha 3 that is 2*alpha2
        alpha3 = alpha*2;
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alpha3*s[i];//calculate the new x
        }
        f3=GetError();
        ftimes++;

        //Now reduce or lengthen alpha2 and alpha3 until the minimum is
        //Bracketed by the triplet f1>f2<f3
        while(f2>f1 || f2>f3)
        {
                if(f2>f1)
                {
                        //If f2 is greater than f1 then we shorten alpha2 and alpha3 closer to f1
                        //Effectively both are shortened by a factor of two.
                        alpha3=alpha2;
                        f3=f2;
                        alpha2=alpha2/2;
                        for(int i=0;i<xLength;i++)
                        {
                                x[i]=xold[i]+alpha2*s[i];//calculate the new x
                        }
                        f2=GetError();
                        ftimes++;
                }

                else if(f2>f3)
                {
                        //If f2 is greater than f3 then we length alpah2 and alpha3 closer to f1
                        //Effectively both are lengthened by a factor of two.
                        alpha2=alpha3;
                        f2=f3;
                        alpha3=alpha3*2;
                        for(int i=0;i<xLength;i++)
                        {
                                x[i]=xold[i]+alpha3*s[i];//calculate the new x
                        }
                        f3=GetError();
                        ftimes++;

                }
        }
        // get the alpha for the minimum f of the quadratic approximation
        alphaStar= alpha2+((alpha2-alpha1)*(f1-f3))/(3*(f1-2*f2+f3));

        //Guarantee that the new alphaStar is within the bracket
        if(alphaStar>alpha3 || alphaStar<alpha1) alphaStar=alpha2;

        if(alphaStar!=alphaStar)
        {
                alphaStar=.001;//Fix nan problem
        }
        /// Set the values to alphaStar
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alphaStar*s[i];//calculate the new x
        }
        fnew=GetError();
        ftimes++;
        fold=fnew;
        /*
        cout<<"F at alphaStar: "<<fnew<<endl;
        cout<<"alphaStar: "<<alphaStar<<endl;
        cout<<"F0: "<<f0<<endl;
        cout<<"F1: "<<f1<<endl;
        cout<<"F2: "<<f2<<endl;
        cout<<"F3: "<<f3<<endl;
        cout<<"Alpha1: "<<alpha1<<endl;
        cout<<"Alpha2: "<<alpha2<<endl;
        cout<<"Alpha3: "<<alpha3<<endl;
        */

        /////////////////////////////////////
        ///end of line search
        /////////////////////////////////////


        double bottom=0;
        double deltaXtDotGamma;
        double gammatDotNDotGamma=0;
        double firstTerm=0;
        double deltaXnorm=1;


        int iterations=1;
        int steps;

        ///Calculate deltaX
        for(int i=0;i<xLength;i++)
        {
                deltaX[i]=x[i]-xold[i];//Calculate the difference in x for the Hessian update
        }
        double maxIterNumber = MaxIterations * xLength;
        while(deltaXnorm>convergence && fnew>smallF && iterations<maxIterNumber)
        {
        //////////////////////////////////////////////////////////////////////
        ///Start of main loop!!!!
        //////////////////////////////////////////////////////////////////////
        bottom=0;
        deltaXtDotGamma = 0;
        pert = fnew*pertMag;
        if(pert<pertMin) pert = pertMin;
        for(int i=0;i<xLength;i++)
        {
                //Calculate the new gradient vector
                temper=x[i];
                x[i]=temper-pert;
                first = GetError();
                x[i]=temper+pert;
                second= GetError();
                gradnew[i]=.5*(second-first)/pert;
                ftimes++;
                x[i]=temper;
                //Calculate the change in the gradient
                gamma[i]=gradnew[i]-grad[i];
                bottom+=deltaX[i]*gamma[i];

                deltaXtDotGamma += deltaX[i]*gamma[i];

        }

        //make sure that bottom is never 0
        if (bottom==0) bottom=.0000000001;

        //calculate all (1xn).(nxn)

        for(int i=0;i<xLength;i++)
        {
                gammatDotN[i]=0;
                for(int j=0;j<xLength;j++)
                {
                        gammatDotN[i]+=gamma[j]*N[i][j];//This is gammatDotN transpose
                }

        }
        //calculate all (1xn).(nx1)

        gammatDotNDotGamma=0;
        for(int i=0;i<xLength;i++)
        {
                gammatDotNDotGamma+=gammatDotN[i]*gamma[i];
        }

        //Calculate the first term

        firstTerm=0;
        firstTerm=1+gammatDotNDotGamma/bottom;

        //Calculate all (nx1).(1xn) matrices
        for(int i=0;i<xLength;i++)
        {
                for(int j=0;j<xLength;j++)
                {
                        FirstSecond[i][j]=((deltaX[j]*deltaX[i])/bottom)*firstTerm;
                        deltaXDotGammatDotN[i][j]=deltaX[i]*gammatDotN[j];
                        gammatDotDeltaXt[i][j]=gamma[i]*deltaX[j];
                }
        }

        //Calculate all (nxn).(nxn) matrices

        for(int i=0;i<xLength;i++)
        {
                for(int j=0;j<xLength;j++)
                {
                        NDotGammaDotDeltaXt[i][j]=0;
                        for(int k=0;k<xLength;k++)
                        {
                                NDotGammaDotDeltaXt[i][j]+=N[i][k]*gammatDotDeltaXt[k][j];
                        }
                }
        }
        //Now calculate the BFGS update on N
        //cout<<"N:"<<endl;
        for(int i=0;i<xLength;i++)
        {

                for(int j=0;j<xLength;j++)
                {
                        N[i][j]=N[i][j]+FirstSecond[i][j]-(deltaXDotGammatDotN[i][j]+NDotGammaDotDeltaXt[i][j])/bottom;
                        //cout<<" "<<N[i][j]<<" ";
                }
                //cout<<endl;
        }

        //Calculate s
        for(int i=0;i<xLength;i++)
        {
                s[i]=0;
                for(int j=0;j<xLength;j++)
                {
                        s[i]+=-N[i][j]*gradnew[j];
                }
        }

        alpha=1; //Initial search vector multiplier


        //copy newest values to the xold
        for(int i=0;i<xLength;i++)
        {
                xold[i]=x[i];//Copy last values to xold
        }
        steps=0;

        ///////////////////////////////////////////////////////
        /// Start of line search
        ///////////////////////////////////////////////////////

        //Make the initial position alpha1
        alpha1=0;
        f1 = fnew;

        //Take a step of alpha=1 as alpha2
        alpha2=1;
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alpha2*s[i];//calculate the new x
        }
        f2 = GetError();
        ftimes++;

        //Take a step of alpha 3 that is 2*alpha2
        alpha3 = alpha2*2;
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alpha3*s[i];//calculate the new x
        }
        f3=GetError();
        ftimes++;

        //Now reduce or lengthen alpha2 and alpha3 until the minimum is
        //Bracketed by the triplet f1>f2<f3
        steps=0;
        while(f2>f1 || f2>f3)
        {
                if(f2>f1)
                {
                        //If f2 is greater than f1 then we shorten alpha2 and alpha3 closer to f1
                        //Effectively both are shortened by a factor of two.
                        alpha3=alpha2;
                        f3=f2;
                        alpha2=alpha2/2;
                        for(int i=0;i<xLength;i++)
                        {
                                x[i]=xold[i]+alpha2*s[i];//calculate the new x
                        }
                        f2=GetError();
                        ftimes++;
                }

                else if(f2>f3)
                {
                        //If f2 is greater than f3 then we length alpah2 and alpha3 closer to f1
                        //Effectively both are lengthened by a factor of two.
                        alpha2=alpha3;
                        f2=f3;
                        alpha3=alpha3*2;
                        for(int i=0;i<xLength;i++)
                        {
                                x[i]=xold[i]+alpha3*s[i];//calculate the new x
                        }
                        f3=GetError();
                        ftimes++;
                }
                /* this should be deleted soon!!!!
                if(steps==-4)
                        {
                                alpha2=1;
                                alpha3=2;

                                for(int i=0;i<xLength;i++)
                                {
                                        for(int j=0;j<xLength;j++)
                                        {
                                                if(i==j)
                                                {
                                                        N[i][j]=1;
                                                        s[i]=-gradnew[i]; //Calculate the initial search vector
                                                }
                                                else N[i][j]=0;
                                        }
                                }
                        }
                */
                /*
                if(steps>100)
                        {
                        continue;
                        }
                */
                steps=steps+1;
        }


        // get the alpha for the minimum f of the quadratic approximation
        alphaStar= alpha2+((alpha2-alpha1)*(f1-f3))/(3*(f1-2*f2+f3));


        //Guarantee that the new alphaStar is within the bracket
        if(alphaStar>=alpha3 || alphaStar<=alpha1)
        {
                alphaStar=alpha2;
        }
        if(alphaStar!=alphaStar) alphaStar=0;

        /// Set the values to alphaStar
        for(int i=0;i<xLength;i++)
        {
                x[i]=xold[i]+alphaStar*s[i];//calculate the new x
        }
        fnew=GetError();
        ftimes++;

        /*
        cout<<"F at alphaStar: "<<fnew<<endl;
        cout<<"alphaStar: "<<alphaStar<<endl;
        cout<<"F1: "<<f1<<endl;
        cout<<"F2: "<<f2<<endl;
        cout<<"F3: "<<f3<<endl;
        cout<<"Alpha1: "<<alpha1<<endl;
        cout<<"Alpha2: "<<alpha2<<endl;
        cout<<"Alpha3: "<<alpha3<<endl;
        */

        /////////////////////////////////////
        ///end of line search
        ////////////////////////////////////

        deltaXnorm=0;
        for(int i=0;i<xLength;i++)
        {
                deltaX[i]=x[i]-xold[i];//Calculate the difference in x for the hessian update
                deltaXnorm+=deltaX[i]*deltaX[i];
                grad[i]=gradnew[i];
        }
        deltaXnorm=sqrt(deltaXnorm);
        iterations++;
        /////////////////////////////////////////////////////////////
        ///End of Main loop
        /////////////////////////////////////////////////////////////
        }
        ////Debug


#ifdef DEBUG

        for(int i=0;i<xLength;i++)
        {
                cstr<<"Parameter("<<i<<"): "<<*(x[i])<<endl;
                //cout<<xold[i]<<endl;
        }
        cstr<<"Fnew: "<<fnew<<endl;
        cstr<<"Number of Iterations: "<<iterations<<endl;
        cstr<<"Number of function calls: "<<ftimes<<endl;
        debugprint(cstr.str());
        cstr.clear();

#endif

        ///End of function
        double validSolution;
        if(isFine==1) validSolution=validSolutionFine;
        else validSolution=validSoltuionRough;
        if(fnew<validSolution)
                {
                return succsess;
                }
        else
                {

                //Replace the bad numbers with the last result
                for(int i=0;i<xLength;i++)
                {
                        x[i]=origSolution[i];
                }
                return noSolution;
                }

}

double Solver::GetElement(int i)
{
	return x[i];
}

double calc(constraint * cons, int consLength)
{
        double error=0;
        double temp,dx,dy,m,n,Ex,Ey,rad1,rad2,t,Xint,Yint,dx2,dy2,hyp1,hyp2,temp2;
        for(int i=0;i<consLength;i++)
        {
			switch(cons[i].type)
			{
				
				case internalAngle:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;
                        dx2 = L2_P2_x - L2_P1_x;
                        dy2 = L2_P2_y - L2_P1_y;

                        hyp1=_hypot(dx,dy);
                        hyp2=_hypot(dx2,dy2);

                        dx=dx/hyp1;
                        dy=dy/hyp1;
                        dx2=dx2/hyp2;
                        dy2=dy2/hyp2;

                        temp = dx*dx2+dy*dy2;
                        temp2 = cos(angleP);
                        error += (temp+temp2)*(temp+temp2);
                }
				break;

				case externalAngle:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;
                        dx2 = L2_P2_x - L2_P1_x;
                        dy2 = L2_P2_y - L2_P1_y;

                        hyp1=_hypot(dx,dy);
                        hyp2=_hypot(dx2,dy2);

                        dx=dx/hyp1;
                        dy=dy/hyp1;
                        dx2=dx2/hyp2;
                        dy2=dy2/hyp2;

                        temp = dx*dx2-dy*dy2;
                        temp2 = cos(M_PI-angleP);
                        error += (temp+temp2)*(temp+temp2);
                }
				break;

				case perpendicular:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;
                        dx2 = L2_P2_x - L2_P1_x;
                        dy2 = L2_P2_y - L2_P1_y;

                        hyp1=_hypot(dx,dy);
                        hyp2=_hypot(dx2,dy2);

                        dx=dx/hyp1;
                        dy=dy/hyp1;
                        dx2=dx2/hyp2;
                        dy2=dy2/hyp2;

                        temp = dx*dx2+dy*dy2;
                        error += (temp)*(temp);
                }
				break;

                // Colinear constraint
				case colinear:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;

                        m=dy/dx;
                        n=dx/dy;
                        // Calculate the error between the expected intersection point
                        // and the true point of the second lines two end points on the
                        // first line
                        if(m<=1 && m>-1)
                        {
                                //Calculate the expected y point given the x coordinate of the point
                                Ey=L1_P1_y+m*(L2_P1_x-L1_P1_x);
                                error+=(Ey-L2_P1_y)*(Ey-L2_P1_y);

                                Ey=L1_P1_y+m*(L2_P2_x-L1_P1_x);
                                error+=(Ey-L2_P2_y)*(Ey-L2_P2_y);
                        }
                        else
                        {
                                //Calculate the expected x point given the y coordinate of the point
                                Ex=L1_P1_x+n*(L2_P1_y-L1_P1_y);
                                error+=(Ex-L2_P1_x)*(Ex-L2_P1_x);

                                Ex=L1_P1_x+n*(L2_P2_y-L1_P1_y);
                                error+=(Ex-L2_P2_x)*(Ex-L2_P2_x);
                        }
                }
				break;

                // Point on a circle
				case pointOnCircle:
                {
                        //see what the current radius to the point is
                        rad1=_hypot(C1_Center_x-P1_x,C1_Center_y-P1_y);
                        //Compare this radius to the radius of the circle, return the error squared
                        temp = rad1-C1_rad;
                        error += temp*temp;
                        //cout<<"Point On circle error"<<temp*temp<<endl;
                }
				break;

				case pointOnArc:
                {
                        //see what the current radius to the point is
                        rad1=_hypot(A1_Center_x-P1_x,A1_Center_y-P1_y);
                        rad2=_hypot(A1_Center_x-A1_Start_x,A1_Center_y-A1_Start_y);
                        //Compare this radius to the radius of the circle, return the error squared
                        temp = rad1-rad2;
                        error += temp*temp;
                        //cout<<"Point On circle error"<<temp*temp<<endl;
                }
				break;

				case pointOnLineMidpoint:
                {
                        Ex=(L1_P1_x+L1_P2_x)/2;
                        Ey=(L1_P1_y+L1_P2_y)/2;
                        temp = Ex-P1_x;
                        temp2 = Ey-P1_y;
                        error += temp*temp+temp2*temp2;

                }
				break;

				case pointOnArcMidpoint:
                {
                        rad1=_hypot(A1_Center_x-A1_Start_x,A1_Center_y-A1_Start_y);
                        temp = atan2(A1_Start_y-A1_Center_y,A1_Start_x-A1_Center_x);
                        temp2= atan2(A1_End_y-A1_Center_y,A1_End_x-A1_Center_x);
                        Ex=A1_Center_x+rad1*cos((temp2+temp)/2);
                        Ey=A1_Center_y+rad1*sin((temp2+temp)/2);
                        temp = (Ex-P1_x);
                        temp2 = (Ey-P1_y);
                        error += temp*temp+temp2*temp2;
                }
				break;

				case pointOnCircleQuad:
                {
                        Ex=C1_Center_x;
                        Ey=C1_Center_y;
                        switch((int)quadIndex)
                        {
                                case 0:
                                        Ex+=C1_rad;
                                        break;
                                case 1:
                                        Ey+=C1_rad;
                                        break;
                                case 2:
                                        Ex-=C1_rad;
                                        break;
                                case 3:
                                        Ey-=C1_rad;
                        }
                        temp = (Ex-P1_x);
                        temp2 = (Ey-P1_y);
                        error += temp*temp+temp2*temp2;
                }
				break;

				case symmetricPoints:
                {
                        dx=Sym_P2_x-Sym_P1_x;
                        dy=Sym_P2_y-Sym_P1_y;
                        t=-(dy*P1_x-dx*P1_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = P1_x+dy*t*2;
                        Ey = P1_y-dx*t*2;
                        temp = (Ex-P2_x);
                        temp2 = (Ey-P2_y);
                        error += temp*temp+temp2*temp2;
                }
				break;

				case symmetricLines:
                {
                        dx=Sym_P2_x-Sym_P1_x;
                        dy=Sym_P2_y-Sym_P1_y;
                        t=-(dy*L1_P1_x-dx*L1_P1_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = L1_P1_x+dy*t*2;
                        Ey = L1_P1_y-dx*t*2;
                        temp = (Ex-L2_P1_x);
                        temp2 = (Ey-L2_P1_y);
                        error += temp*temp+temp2*temp2;
                        t=-(dy*L1_P2_x-dx*L1_P2_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = L1_P2_x+dy*t*2;
                        Ey = L1_P2_y-dx*t*2;
                        temp = (Ex-L2_P2_x);
                        temp2 = (Ey-L2_P2_y);
                        error += temp*temp+temp2*temp2;
                }
				break;

				case symmetricCircles:
                {
                        dx=Sym_P2_x-Sym_P1_x;
                        dy=Sym_P2_y-Sym_P1_y;
                        t=-(dy*C1_Center_x-dx*C1_Center_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = C1_Center_x+dy*t*2;
                        Ey = C1_Center_y-dx*t*2;
                        temp = (Ex-C2_Center_x);
                        temp2 = (Ey-C2_Center_y);
                        error += temp*temp+temp2*temp2;
                        temp = (C1_rad-C2_rad);
                        error += temp*temp;
                }
				break;

				case symmetricArcs:
                {
                        dx=Sym_P2_x-Sym_P1_x;
                        dy=Sym_P2_y-Sym_P1_y;
                        t=-(dy*A1_Start_x-dx*A1_Start_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = A1_Start_x+dy*t*2;
                        Ey = A1_Start_y-dx*t*2;
                        temp = (Ex-A2_Start_x);
                        temp2 = (Ey-A2_Start_y);
                        error += temp*temp+temp2*temp2;
                        t=-(dy*A1_End_x-dx*A1_End_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = A1_End_x+dy*t*2;
                        Ey = A1_End_y-dx*t*2;
                        temp = (Ex-A2_End_x);
                        temp2 = (Ey-A2_End_y);
                        error += temp*temp+temp2*temp2;
                        t=-(dy*A1_Center_x-dx*A1_Center_y-dy*Sym_P1_x+dx*Sym_P1_y)/(dx*dx+dy*dy);
                        Ex = A1_Center_x+dy*t*2;
                        Ey = A1_Center_y-dx*t*2;
                        temp = (Ex-A2_Center_x);
                        temp2 = (Ey-A2_Center_y);
                        error += temp*temp+temp2*temp2;
                }
				break;

				case arcStartToArcEnd:
                {
                        error += (A1_Start_x-A2_End_x)*(A1_Start_x-A2_End_x)+(A1_Start_y-A2_End_y)*(A1_Start_y-A2_End_y);
                }
				break;

				case arcStartToArcStart:
                {
                        error += (A1_Start_x-A2_Start_x)*(A1_Start_x-A2_Start_x)+(A1_Start_y-A2_Start_y)*(A1_Start_y-A2_Start_y);
                }
				break;

				case arcEndtoArcEnd:
                {
                        error += (A1_End_x-A2_End_x)*(A1_End_x-A2_End_x)+(A1_End_y-A2_End_y)*(A1_End_y-A2_End_y);
                }
				break;

				case arcTangentToArc:
                {
//#ifndef NEWARC
                        // temp = center point distance
                        temp = sqrt(A1_Center_x-A2_Center_x)*(A1_Center_x-A2_Center_x)+(A1_Center_y-A2_Center_y)*(A1_Center_y-A2_Center_y);
                        // center point to center point distance= r1+r2
                        double radDiff=A1_radius-A2_radius;
                        double extError,intError;
                        extError = ((A1_radius+A2_radius)-temp)*((A1_radius+A2_radius)-temp);
                        if(radDiff>=1)//A1 is bigger
                        {
                                intError = (radDiff-temp)*(radDiff-temp);
                        }
                        else
                        {
                                intError = (-radDiff-temp)*(-radDiff-temp);
                        }
                        if(extError<intError) error += extError;
                        else error =+ intError;
//#else
//#endif
                }
				break;

				case circleTangentToCircle:
                {
                        // temp = center point distance
                        temp = sqrt(C1_Center_x-C2_Center_x)*(C1_Center_x-C2_Center_x)+(C1_Center_y-C2_Center_y)*(C1_Center_y-C2_Center_y);
                        // center point to center point distance= r1+r2
                        double radDiff=C1_rad-C2_rad;
                        double extError,intError;
                        extError = ((C1_rad+C2_rad)-temp)*((C1_rad+C2_rad)-temp);
                        if(radDiff>=1)//A1 is bigger
                        {
                                intError = (radDiff-temp)*(radDiff-temp);
                        }
                        else
                        {
                                intError = (-radDiff-temp)*(-radDiff-temp);
                        }
                        if(extError<intError) error += extError;
                        else error =+ intError;

                }
				break;

				case circleTangentToArc:
                {
//#ifndef NEWARC
                        // temp = center point distance
                        temp = sqrt(C1_Center_x-A1_Center_x)*(C1_Center_x-A1_Center_x)+(C1_Center_y-A1_Center_y)*(C1_Center_y-A1_Center_y);
                        // center point to center point distance= r1+r2
                        double radDiff=C1_rad-A1_radius;
                        double extError,intError;
                        extError = ((C1_rad+A1_radius)-temp)*((C1_rad+A1_radius)-temp);
                        if(radDiff>=1)//A1 is bigger
                        {
                                intError = (radDiff-temp)*(radDiff-temp);
                        }
                        else
                        {
                                intError = (-radDiff-temp)*(-radDiff-temp);
                        }
                        if(extError<intError) error += extError;
                        else error =+ intError;
                }
				break;
				}
        }
        return error;

}
