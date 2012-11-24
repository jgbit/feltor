#ifndef _BLUEPRINT_
#define _BLUEPRINT_

#include <iostream>
#include "ghostmatrix" // holds boundary conditions
#include "message.h"

namespace toefl{
//toefl brauch libraries, um zu funktionieren
//z.B. fftw3 für dfts, cuda für graphikkarten, oder sparse matrix solver 
enum cap{ TL_CURVATURE, TL_COUPLING, TL_IMPURITY, TL_GLOBAL};
enum target{ TL_ELECTRONS, TL_IONS, TL_IMPURITIES, TL_POTENTIAL};


/*! @brief Holds the physical parameters of the problem.
 *
 * @note This is an aggregate and thus you can use initializer lists
 */
struct Physical
{
    double d;  //!< The coupling constant
    double nu; //!< The artificial viscosity
    double g[3]; //!< The gradient for electrons 0, ions 1 and impurities 2
    double kappa[2]; //!< The curvature in x 0 and y 1
    double a[2]; //!< Charge of ions 0 and impurities 1
    double mu_z; //!< The mass of impurities
    double tau[2]; //!< temperature of ions 0 and impurities 2
    /*! @brief This is a POD
     */ 
    Physical() = default;
    void display( std::ostream& os = std::cout) const
    {
        os << "Physical parameters are: \n"
            <<"Coupling = "<<d<<"\n"
            <<"viscosity = "<<nu<<"\n"
            <<"Curvature_x = "<<kappa[0]<<" Curvature_y = "<<kappa[1]<<"\n"
            <<"gradients: g[0] ="<<g[0]<<" g[1]="<<g[1]<<" g[2]="<<g[2]<<"\n"
            <<"Ions       a[0] ="<<a[0]<<" tau[0]="<<tau[0]<<"\n"
            <<"Impurities a[1] ="<<a[1]<<" mu_z="<<mu_z<<" tau[1]="<<tau[1]<<"\n";
    }
};

/*! @brief Describes the boundary and the boundary conditions of the problem.
 *
 * @note This is an aggregate and thus you can use initializer lists
 */
struct Boundary
{
    double lx; //!< Physical extension of x-direction
    double ly; //!< Physical extension of y-direction
    enum bc bc_x;  //!< Boundary condition in x (y is always periodic)
    Boundary() = default;
    void display( std::ostream& os = std::cout) const
    {
        os << "Boundary parameters are: \n"
            <<" lx="<<lx<<"\n"
            <<" ly="<<ly<<"\n"
            <<"Boundary conditions are ";
        switch(bc_x)
        {
            case(TL_PERIODIC): os << "periodic in x\n"; break;
            case(   TL_DST00): os << "dst 1 like \n"; break;
            case(   TL_DST01): os << "dst 2 like \n"; break;
            case(   TL_DST10): os << "dst 3 like \n"; break;
            case(   TL_DST11): os << "dst 4 like \n"; break;
        }
    }
};

/*! @brief Describes the algorithmic (notably discretization) issues of the solver.
 *
 * @note This is an aggregate and thus you can use initializer lists
 */
struct Algorithmic
{
    size_t nx;  //!< # of gridpoints in x
    size_t ny;  //!< # of gridpoints in y
    double h;  //!< ly/ny
    double dt; //!< The time step
    Algorithmic() = default;
    void display( std::ostream& os = std::cout) const
    {
        os << "Algorithmic parameters are: \n"
            <<"nx="<<nx<<"\n"
            <<"ny="<<ny<<"\n"
            <<"h ="<<h<<"\n"
            <<"dt="<<dt<<"\n";
    }
};


/*! @brief The Setting for the pipeline 
 *
 * The Setting consists of parameters and capacities!
 * With this construction plan you can go to 
 * the pipeline manufacturer who constructs the pipeline. 
 * It is recommended to call 
 * \code
 *  try{ blueprint.consistencyCheck();}
 *  catch( toefl::Message& m){m.display();}
 *  \endcode
 * before constructing a Pipeline to catch any Messages before construction.
 */
class Blueprint
{
    const Physical phys;
    const Boundary bound;
    const Algorithmic alg;
    bool curvature, coupling, imp, global;
  public:
    /*! @brief Init parameters
     *
     * All capacities are disabled by default!
     * @param phys The physical parameters of the equations including numeric viscosity
     */
    Blueprint( const Physical& phys, const Boundary& bound, const Algorithmic& alg): phys(phys), bound(bound), alg(alg)
    {
        curvature = coupling = imp = global = false; 
    }
    const Physical& getPhysical() const {return phys;}
    const Boundary& getBoundary() const {return bound;}
    const Algorithmic& getAlgorithmic() const {return alg;}
    void enable(enum cap capacity)
    {
        switch( capacity)
        {
            case( TL_CURVATURE): curvature = true;
                                 break;
            case( TL_COUPLING) : coupling = true;
                                 break;
            case( TL_IMPURITY) : imp = true;
                                 break;
            case( TL_GLOBAL):    global = true;
                                 break;
            default: throw toefl::Message( "Unknown Capacity\n", ping);
        }
    }
    bool isEnabled( enum cap capacity) const
    {
        switch( capacity)
        {
            case( TL_CURVATURE): return curvature;
            case( TL_COUPLING) : return coupling;
            case( TL_IMPURITY) : return imp;
            case( TL_GLOBAL):    return global;
            default: throw toefl::Message( "Unknown Capacity\n", ping);
        }
    }
    void consistencyCheck() const;
    void display( std::ostream& os = std::cout) const
    {
        phys.display( os);
        bound.display( os);
        alg.display( os);
        os << "Enabled capacities are \n"
            <<"curvature "<< curvature <<"\n"
            <<"coupling  "<<coupling<<"\n"
            <<"imp       "<<imp<<"\n"
            <<"global    "<<global<<"\n";
    }

};

void Blueprint::consistencyCheck() const
{
    //Check algorithm and boundaries
    if( alg.dt <= 0) 
        throw toefl::Message( "dt <= 0!\n", ping);
    if( alg.h - bound.lx/(double)alg.nx > 1e-15) 
        throw toefl::Message( "h != lx/nx\n", ping); 
    if( alg.h - bound.ly/(double)alg.ny > 1e-15) 
        throw toefl::Message( "h != ly/ny\n", ping);
    if( alg.nx == 0||alg.ny == 0) 
        throw toefl::Message( "Set nx and ny!\n", ping);
    //Check physical parameters
    if( curvature && phys.kappa_x == 0 && phys.kappa_y ==0 ) 
        throw toefl::Message( "Curvature enabled but zero!\n", ping);
    if( phys.nu < 0) 
        throw toefl::Message( "nu < 0!\n", ping);
    if( phys.a[0] <= 0 || phys.mu_i <= 0 || phys.tau[0] < 0) 
        throw toefl::Message( "Ion species badly set\n", ping);
    if( imp && (phys.a[1] <= 0 || phys.mu_z <= 0 || phys.tau[1] < 0)) 
        throw toefl::Message( "Impuritiy species badly set\n", ping);
    if( phys.a[0] + phys.a[1] != 1)
        throw toefl::Message( "a[0] + a[1] != 1\n", ping);
    if( phys.g[1] != (phys.g[0] - phys.a[1]*phys.g[2])/(1.-phys.a[1]))
        throw toefl::Message( "g[1] is wrong\n", ping);
    if( global) 
        throw toefl::Message( "Global solver not yet implemented\n", ping);
    //Some Warnings
    if( !curvature && (phys.kappa_x != 0 || phys.kappa_y != 0)) 
        std::cerr <<  "TL_WARNING: Curvature disabled but kappa not zero (will be ignored)!\n";
    if( !imp && (phys.a[1] != 0 || phys.mu_z != 0 || phys.tau[1] != 0)) 
        std::cerr << "TL_WARNING: Impurity disabled but z species not 0 (will be ignored)!\n";
        
}


}

#endif //_BLUEPRINT_