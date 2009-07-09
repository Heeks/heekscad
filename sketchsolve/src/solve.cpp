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
}

int Solver::solve(double  **x,constraint * cons, int consLength, int isFine)
{
        std::stringstream cstr;
        double convergence,pert ;
        //Save the original parameters for later.
        for(int i=0;i<xLength;i++)
        {
                origSolution[i]=*x[i];
        }

        if(isFine>0) convergence = XconvergenceFine;
        else convergence = XconvergenceRough;
        //integer to keep track of how many times calc is called
        int ftimes=0;
        //Calculate Function at the starting point:
        double f0;
        f0 = calc(cons,consLength);
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
                temper= *x[j];
                *x[j]= temper-pert;
                first = calc(cons,consLength);
                *x[j]= temper+pert;
                second = calc(cons,consLength);
                grad[j]=.5*(second-first)/pert;
                ftimes++;
#ifdef DEBUG
                cstr << "gradient: " << grad[j];
                debugprint(cstr.str());
                cstr.clear();
#endif
                *x[j]=temper;
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
                xold[i]=*x[i];//Copy last values to xold
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
                *x[i]=xold[i]+alpha2*s[i];//calculate the new x
        }
        f2 = calc(cons,consLength);
        ftimes++;

        //Take a step of alpha 3 that is 2*alpha2
        alpha3 = alpha*2;
        for(int i=0;i<xLength;i++)
        {
                *x[i]=xold[i]+alpha3*s[i];//calculate the new x
        }
        f3=calc(cons,consLength);
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
                                *x[i]=xold[i]+alpha2*s[i];//calculate the new x
                        }
                        f2=calc(cons,consLength);
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
                                *x[i]=xold[i]+alpha3*s[i];//calculate the new x
                        }
                        f3=calc(cons,consLength);
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
                *x[i]=xold[i]+alphaStar*s[i];//calculate the new x
        }
        fnew=calc(cons,consLength);
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
                deltaX[i]=*x[i]-xold[i];//Calculate the difference in x for the Hessian update
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
                temper=*x[i];
                *x[i]=temper-pert;
                first = calc(cons,consLength);
                *x[i]=temper+pert;
                second= calc(cons,consLength);
                gradnew[i]=.5*(second-first)/pert;
                ftimes++;
                *x[i]=temper;
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
                xold[i]=*x[i];//Copy last values to xold
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
                *x[i]=xold[i]+alpha2*s[i];//calculate the new x
        }
        f2 = calc(cons,consLength);
        ftimes++;

        //Take a step of alpha 3 that is 2*alpha2
        alpha3 = alpha2*2;
        for(int i=0;i<xLength;i++)
        {
                *x[i]=xold[i]+alpha3*s[i];//calculate the new x
        }
        f3=calc(cons,consLength);
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
                                *x[i]=xold[i]+alpha2*s[i];//calculate the new x
                        }
                        f2=calc(cons,consLength);
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
                                *x[i]=xold[i]+alpha3*s[i];//calculate the new x
                        }
                        f3=calc(cons,consLength);
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
                *x[i]=xold[i]+alphaStar*s[i];//calculate the new x
        }
        fnew=calc(cons,consLength);
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
                deltaX[i]=*x[i]-xold[i];//Calculate the difference in x for the hessian update
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
                        *x[i]=origSolution[i];
                }
                return noSolution;
                }

}



double calc(constraint * cons, int consLength)
{
        double error=0;
        double temp,dx,dy,m,n,Ex,Ey,rad1,rad2,t,Xint,Yint,dx2,dy2,hyp1,hyp2,temp2;
        for(int i=0;i<consLength;i++)
        {
			switch(cons[i].type)
			{
				case pointOnPoint:
                {
                        //Hopefully avoid this constraint, make coincident points use the same parameters
                        error += (P1_x - P2_x) * (P1_x - P2_x) + (P1_y - P2_y) * (P1_y - P2_y);
                }
				break;

				case P2PDistance:
                {
                        error+= (P1_x - P2_x) * (P1_x - P2_x) + (P1_y - P2_y) * (P1_y - P2_y) - distance * distance;

                }
				break;

				case P2PDistanceVert:
                {
                        error+= (P1_y - P2_y) * (P1_y - P2_y) - distance * distance;
                }
				break;

				case P2PDistanceHorz:
                {
                        error+= (P1_x - P2_x) * (P1_x - P2_x) - distance * distance;
                }
				break;

				case pointOnLine:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;

                        m=dy/dx; //Slope
                        n=dx/dy; //1/Slope

                        if(m<=1 && m>=-1)
                        {
                                //Calculate the expected y point given the x coordinate of the point
                                Ey=L1_P1_y+m*(P1_x-L1_P1_x);
                                error+=(Ey-P1_y)*(Ey-P1_y);
                        }
                        else
                        {
                                //Calculate the expected x point given the y coordinate of the point
                                Ex=L1_P1_x+n*(P1_y-L1_P1_y);
                                error+=(Ex-P1_x)*(Ex-P1_x);
                        }
                }
				break;

				case P2LDistance:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;

                        double Xint,Yint;
                        t=-(L1_P1_x*dx-P1_x*dx+L1_P1_y*dy-P1_y*dy)/(dx*dx+dy*dy);
                        Xint=L1_P1_x+dx*t;
                        Yint=L1_P1_y+dy*t;
                        temp= _hypot((P1_x - Xint),(P1_y - Yint)) - distance;
                        error += temp*temp/10;

                }
				break;

				case P2LDistanceVert:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;

                        t=(P1_x- L1_P1_x)/dx;
                        Yint=L1_P1_y+dy*t;
                        temp= fabs((P1_y - Yint)) - distance;
                        error += temp*temp;

                }
				break;

				case P2LDistanceHorz:
                {
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;

                        t=(P1_y- L1_P1_y)/dy;
                        Xint=L1_P1_x+dx*t;
                        temp= fabs((P1_x - Xint)) - distance;
                        error += temp*temp/10;

                }
				break;


				case vertical:
                {
                        double odx = L1_P2_x - L1_P1_x;
                        /*
                        double ody = L1_P2_y - L1_P1_y;


                        double hyp=_hypot(odx,ody);
                        dx = odx/hyp;
                        dy = ody/hyp;

                        double theta = atan2(dy,dx);
                        double p1 = odx;//-cos(theta)*cos(theta)*ody;
                        error+=p1*p1*10;
                        */
                        error+=odx*odx*1000;
                }
				break;

				case horizontal:
                {
                        //double odx = L1_P2_x - L1_P1_x;
                        double ody = L1_P2_y - L1_P1_y;
                        /*
                        double hyp=_hypot(odx,ody);
                        dx = odx/hyp;
                        dy = ody/hyp;

                        double theta = atan2(dy,dx);
                        double p1 = (ody);//-sin(theta)*sin(theta)*odx);
                        error+=p1*p1*10;
                        */
                        error+=ody*ody*1000;
                }
				break;

				case tangentToCircle:
                {
                        double dx,dy,Rpx,Rpy,RpxN,RpyN,hyp,error1,error2;
                        dx = L1_P2_x-L1_P1_x;
                        dy = L1_P2_y-L1_P1_y;
                        hyp=_hypot(dx,dy);
                        //Calculate the expected tangent intersection points
                        Rpx =C1_Center_x - dy / hyp * C1_rad;
                        Rpy =C1_Center_y + dx / hyp * C1_rad;
                        RpxN=C1_Center_x + dy / hyp * C1_rad;
                        RpyN=C1_Center_y - dx / hyp * C1_rad;

                        error1=(-dy * Rpx + dx * Rpy   + (L1_P1_x * L1_P2_y - L1_P2_x * L1_P1_y))/hyp;
                        error2=(-dy * RpxN + dx * RpyN + (L1_P1_x * L1_P2_y - L1_P2_x * L1_P1_y))/hyp;
                        error1=error1 * error1;
                        error2=error2 * error2;
                        if(error1<error2) error+=error1;
                        else error+=error2;

                }
				break;

				case tangentToArc:
                {
                        
                        double dx,dy;
                        dx = L1_P2_x - L1_P1_x;
                        dy = L1_P2_y - L1_P1_y;


                        double Xint,Yint,radsq;
                        radsq = A1_radius * A1_radius;
                        t=-(L1_P1_x*dx-A1_Center_x*dx+L1_P1_y*dy-A1_Center_y*dy)/(dx*dx+dy*dy);
                        Xint=L1_P1_x+dx*t;
                        Yint=L1_P1_y+dy*t;
                        temp= sqrt((A1_Center_x - Xint)*(A1_Center_x - Xint)+(A1_Center_y - Yint)*(A1_Center_y - Yint)) - sqrt(radsq);
                        error += temp*temp/100;
                }
				break;

				case arcRules:
                {
                        //rad1=_hypot(A1_Center_x - A1_Start_x , A1_Center_y - A1_Start_y);
                        //rad2=_hypot(A1_Center_x - A1_End_x , A1_Center_y - A1_End_y);
                        //error += (rad1-rad2)*(rad1-rad2);
                        //double dx,dy,Rpx,Rpy,RpxN,RpyN,hyp,error1,error2,rad;
                        //dx = A1_End_x - A1_Start_x;
                        //dy = A1_End_y - A1_Start_y;

                        //hyp=_hypot(dx,dy);

                        //double u = (A1_Center_x - A1_Start_x) * (A1_End_x - A1_Start_x) + (A1_Center_y - A1_Start_y) * (A1_End_y - A1_Start_y);
                        //u/=hyp*hyp;

                        //temp = sin(u - .5);
                        //error+=temp*temp*temp*temp*100000;
                        //error+=pow(-2*A1_Center_x*A1_End_y - 2*A1_Center_y*A1_End_y + A1_End_x*A1_End_y + pow(A1_End_y,2) + 2*A1_Center_x*A1_Start_x - 2*A1_Center_y*A1_Start_x - A1_End_x*A1_Start_x + 4*A1_End_y*A1_Start_x - 3*pow(A1_Start_x,2) +  2*A1_Center_y*A1_Start_y + A1_Start_x*A1_Start_y - pow(A1_Start_y,2),2)/(8*pow(A1_End_y,2) + 8*pow(A1_Start_x,2) - 8*A1_End_y*A1_Start_y -  8*A1_Start_x*A1_Start_y + 4*pow(A1_Start_y,2));
#ifndef NEWARC
                        double a1endx2 = A1_End_x * A1_End_x;
                        double a1endy2 = A1_End_y * A1_End_y;
                        double a1startx2 = A1_Start_x*A1_Start_x;
                        double a1starty2 = A1_Start_y*A1_Start_y;
                        double num = -2*A1_Center_x*A1_End_x+a1endx2-2*A1_Center_y*A1_End_y+a1endy2+2*A1_Center_x*A1_Start_x-a1startx2+2*A1_Center_y*A1_Start_y-a1starty2;
                        error += num * num /(4.*a1endx2+a1endy2-2*A1_End_x*A1_Start_x+a1startx2-2*A1_End_y*A1_Start_y+a1starty2);
#endif
                }
				break;

				case lineLength:
                {
                        temp= sqrt(pow(L1_P2_x - L1_P1_x,2) + pow(L1_P2_y - L1_P1_y,2)) - length;
                        //temp=_hypot(L1_P2_x - L1_P1_x , L1_P2_y - L1_P1_y) - length;
                        error += temp*temp*100;
                }
				break;

				case equalLegnth:
                {
                        temp=_hypot(L1_P2_x - L1_P1_x , L1_P2_y - L1_P1_y) - _hypot(L2_P2_x - L2_P1_x , L2_P2_y - L2_P1_y);
                        error += temp*temp;
                }
				break;

				case arcRadius:
                {
                        //rad1 = _hypot(A1_Center_x - A1_Start_x , A1_Center_y - A1_Start_y);
                        //rad2 = _hypot(A1_Center_x - A1_End_x , A1_Center_y - A1_End_y);
                        temp= A1_radius - radius;
                        error += temp*temp;
                }
				break;

				case equalRadiusArcs:
                {
                        //rad1 = _hypot(A1_Center_x - A1_Start_x , A1_Center_y - A1_Start_y);
                        //rad2 = _hypot(A2_Center_x - A2_Start_x , A2_Center_y - A2_Start_y);
                        temp = A1_radius-A2_radius;
                        error += temp*temp;
                }
				break;

				case equalRadiusCircles:
                {
                        temp = C1_rad - C2_rad;
                        error += temp*temp;
                }
				break;

				case equalRadiusCircArc:
                {
                        //rad1 = _hypot(A1_Center_x - A1_Start_x , A1_Center_y - A1_Start_y);
                        temp = A1_radius-C1_rad;
                        error += temp*temp;
                }
				break;

				case concentricArcs:
                {
                        temp = _hypot(A1_Center_x - A2_Center_x , A1_Center_y - A2_Center_y);
                        error += temp*temp;
                }
				break;

				case concentricCircles:
                {
                        temp = _hypot(C1_Center_x - C2_Center_x , C1_Center_y - C2_Center_y);
                        error += temp*temp;
                }
				break;

				case concentricCircArc:
                {
                        temp = _hypot(A1_Center_x - C1_Center_x , A1_Center_y - C1_Center_y);
                        error += temp*temp;
                }
				break;

				case circleRadius:
                {
                        error += (C1_rad - radius)*(C1_rad - radius);
                }
				break;

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

				case parallel:
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

                        temp = dy*dx2-dx*dy2;
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

				case pointOnArcStart:
                {
                        error += (P1_x-A1_Start_x)*(P1_x-A1_Start_x)+(P1_y-A1_Start_y)*(P1_y-A1_Start_y);
                }
				break;


				case pointOnArcEnd:
                {
                        error += (P1_x-A1_End_x)*(P1_x-A1_End_x)+(P1_y-A1_End_y)*(P1_y-A1_End_y);
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
