#include <iostream>

#include <cusp/print.h>

#include "timer.cuh"
#include "evaluation.cuh"
#include "dz.cuh"
#include "functions.h"
#include "../functors.h"
#include "../blas.h"
#include "interpolation.cuh"


struct Field
{
    Field( double R_0, double I_0):R_0(R_0), I_0(I_0){}
    void operator()( const std::vector<dg::HVec>& y, std::vector<dg::HVec>& yp)
    {
        for( unsigned i=0; i<y[0].size(); i++)
        {
            double gradpsi = ((y[0][i]-R_0)*(y[0][i]-R_0) + y[1][i]*y[1][i])/I_0/I_0;
            yp[2][i] = y[0][i]*sqrt(1 + gradpsi);
            yp[0][i] = y[0][i]*y[1][i]/I_0;
            yp[1][i] = -y[0][i]*y[0][i]/I_0 + R_0/I_0*y[0][i] ;
        }
    }
    private:
    double R_0, I_0;
};

double R_0 = 10;
double I_0 = 40;
double func(double R, double Z, double phi)
{
    double r2 = (R-R_0)*(R-R_0)+Z*Z;
    return r2*sin(phi);
}
double deri(double R, double Z, double phi)
{
    double r2 = (R-R_0)*(R-R_0)+Z*Z;
    return I_0/R/sqrt(I_0*I_0 + r2)* r2*cos(phi);
}


int main()
{
    Field field( R_0, I_0);
    std::cout << "Type n, Nx, Ny, Nz\n";
    unsigned n, Nx, Ny, Nz;
    std::cin >> n>> Nx>>Ny>>Nz;
    dg::Grid3d<double> g3d( R_0 - 1, R_0+1, -1, 1, 0, 2.*M_PI, n, Nx, Ny, Nz);
    const dg::DVec w3d = dg::create::weights( g3d);
    dg::Timer t;
    t.tic();
    dg::DZ<dg::DMatrix, dg::DVec> dz( field, g3d);
    t.toc();
    std::cout << "Creation of parallel Derivative took "<<t.diff()<<"s\n";

    dg::DVec function = dg::evaluate( func, g3d), derivative(function);
    const dg::DVec solution = dg::evaluate( deri, g3d);
    t.tic();
    dz( function, derivative);
    t.toc();
    std::cout << "Application of parallel Derivative took "<<t.diff()<<"s\n";
    dg::blas1::axpby( 1., solution, -1., derivative);
    double norm = dg::blas2::dot( w3d, solution);
    std::cout << "Norm Solution "<<sqrt( norm)<<"\n";
    std::cout << "Relative Difference Is "<< sqrt( dg::blas2::dot( derivative, w3d, derivative)/norm )<<"\n";


    
    return 0;
}