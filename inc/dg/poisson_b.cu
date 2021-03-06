#include <iostream>
#include <iomanip>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include "backend/evaluation.cuh"
#include "poisson.h"
#include "blas.h"
#include "backend/typedefs.cuh"

#include "backend/timer.cuh"


const double lx = 2*M_PI;
const double ly = 2*M_PI;
//const double lx = 1.;
//const double ly = 1.;


//choose some mean function (attention on lx and ly)
//THESE ARE NOT PERIODIC
/*
double left( double x, double y) { return sin(x)*cos(y);}
double right( double x, double y){ return exp(0.1*(x+y)); }
double jacobian( double x, double y) 
{
    return exp( x-M_PI)*(sin(x)+cos(x))*sin(y) * exp(y-M_PI)*sin(x)*(sin(y) + cos(y)) - sin(x)*exp(x-M_PI)*cos(y) * cos(x)*sin(y)*exp(y-M_PI); 
}
*/

dg::bc bcx = dg::PER;
dg::bc bcy = dg::PER;
double left( double x, double y) {return sin(x)*cos(y);}
double right( double x, double y) {return cos(x)*sin(y);}
double jacobian( double x, double y) 
{
    return cos(x)*cos(y)*cos(x)*cos(y) - sin(x)*sin(y)*sin(x)*sin(y); 
}
////These are for comparing to FD arakawa results
//double left( double x, double y) {return sin(2.*M_PI*(x-hx/2.));}
//double right( double x, double y) {return y;}
//double jacobian( double x, double y) {return 2.*M_PI*cos(2.*M_PI*(x-hx/2.));}

int main()
{
    dg::Timer t;
    unsigned n, Nx, Ny;
    std::cout << "Type n, Nx and Ny! \n";
    std::cin >> n >> Nx >> Ny;
    dg::Grid2d grid( 0, lx, 0, ly, n, Nx, Ny, dg::PER, dg::PER);
    //S2D<double > s2d( grid);
    dg::DVec w2d = dg::create::weights( grid);
    std::cout << "# of 2d cells                     " << Nx*Ny <<std::endl;
    std::cout << "# of Legendre nodes per dimension "<< n <<std::endl;
    dg::DVec lhs = dg::evaluate ( left, grid), jac(lhs);
    dg::DVec rhs = dg::evaluate ( right,grid);
    const dg::DVec sol = dg::evaluate( jacobian, grid );
    dg::DVec eins = dg::evaluate( dg::one, grid );
    std::cout<< std::setprecision(2);


    dg::Poisson<dg::CartesianGrid2d, dg::DMatrix, dg::DVec> poiss( grid);
    unsigned multi=20;
    t.tic(); 
    for( unsigned i=0; i<multi; i++)
        poiss( lhs, rhs, jac);
    t.toc();
    std::cout << "\nArakawa took "<<t.diff()*1000/(double)multi<<"ms\n\n";

    std::cout << std::scientific;
    std::cout << "Mean     Jacobian is "<<dg::blas2::dot( eins, w2d, jac)<<"\n";
    std::cout << "Mean rhs*Jacobian is "<<dg::blas2::dot( rhs, w2d, jac)<<"\n";
    std::cout << "Mean   n*Jacobian is "<<dg::blas2::dot( lhs, w2d, jac)<<"\n";
    dg::blas1::axpby( 1., sol, -1., jac);
    std::cout << "Distance to solution "<<sqrt(dg::blas2::dot( w2d, jac))<<std::endl; //don't forget sqrt when comuting errors

    //periocid bc       |  dirichlet in x per in y
    //n = 1 -> p = 2    |        1.5
    //n = 2 -> p = 1    |        1
    //n = 3 -> p = 3    |        3
    //n = 4 -> p = 3    |        3
    //n = 5 -> p = 5    |        5
    // quantities are all conserved to 1e-15 for periodic bc
    // for dirichlet bc these are not better conserved than normal jacobian

    return 0;
}
