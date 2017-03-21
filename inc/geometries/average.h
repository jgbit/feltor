#pragma once

#include <thrust/host_vector.h>

/*!@file
 *
 * The flux surface average
 */
namespace dg
{
namespace geo
{
/**
 * @brief Delta function for poloidal flux \f$ B_Z\f$
     \f[ |\nabla \psi_p|\delta(\psi_p(R,Z)-\psi_0) = \frac{\sqrt{ (\nabla \psi_p)^2}}{\sqrt{2\pi\varepsilon}} \exp\left(-\frac{(\psi_p(R,Z) - \psi_{0})^2}{2\varepsilon} \right)  \f]
 */
template<class Collective>
struct DeltaFunction
{
    DeltaFunction(const Collective& c, double epsilon,double psivalue) :
        c_(c),
        epsilon_(epsilon),
        psivalue_(psivalue){
    }
    /**
    * @brief Set a new \f$ \varepsilon\f$
    *
    * @param eps new value
    */
    void setepsilon(double eps ){epsilon_ = eps;}
    /**
    * @brief Set a new \f$ \psi_0\f$
    *
    * @param psi_0 new value
    */
    void setpsi(double psi_0 ){psivalue_ = psi_0;}

    /**
     *@brief \f[ \frac{\sqrt{ (\nabla \psi_p)^2}}{\sqrt{2\pi\varepsilon}} \exp\left(-\frac{(\psi_p(R,Z) - \psi_{0})^2}{2\varepsilon} \right)  \f]
     */
    double operator()( double R, double Z) const
    {
        double psip = c_.psip(R,Z), psipR = c_.psipR(R,Z), psipZ = c_.psipZ(R,Z);
        return 1./sqrt(2.*M_PI*epsilon_)*
               exp(-( (psip-psivalue_)* (psip-psivalue_))/2./epsilon_)*sqrt(psipR*psipR +psipZ*psipZ);
    }
    /**
     * @brief == operator()(R,Z)
     */ 
    double operator()( double R, double Z, double phi) const
    {
        return (*this)(R,Z);
    }
    private:
    Collective c_;
    double epsilon_;
    double psivalue_;
};

/**
 * @brief Global safety factor
\f[ \alpha(R,Z) = \frac{|B^\varphi|}{R|B^\eta|} = \frac{I_{pol}(R,Z)}{R|\nabla\psi_p|} \f]
 */
template<class Collective>
struct Alpha
{
    Alpha( const Collective& c):c_(c){}

    /**
    * @brief \f[ \frac{ I_{pol}(R,Z)}{R \sqrt{\nabla\psi_p}} \f]
    */
    double operator()( double R, double Z) const
    {
        double psipR = c_.psipR(R,Z), psipZ = c_.psipZ(R,Z);
        return (1./R)*(c_.ipol(R,Z)/sqrt(psipR*psipR + psipZ*psipZ )) ;
    }
    /**
     * @brief == operator()(R,Z)
     */ 
    double operator()( double R, double Z, double phi) const
    {
        return operator()(R,Z);
    }
    private:
    Collective c_;
};
}//namespace geo

/**
 * @brief Flux surface average over quantity
 *
 * The average is computed using the formula
 \f[ <f>(\psi_0) = \frac{1}{A} \int dV \delta(\psi_p(R,Z)-\psi_0) |\nabla\psi_p|f(R,Z) \f]
 with \f$ A = \int dV \delta(\psi_p(R,Z)-\psi_0)|\nabla\psi_p|\f$
 * @tparam Collective This collective needs to contain at least the 2d functors, 
 * psip and its derivatives psipR and psipZ. 
 * @tparam container  The container class of the vector to average
 * @ingroup misc
 */
template <class Collective, class container = thrust::host_vector<double> >
struct FluxSurfaceAverage
{
     /**
     * @brief Construct from a field and a grid
     * @param g2d 2d grid
     * @param c contains psip, psipR and psipZ
     * @param f container for global safety factor
     */
    FluxSurfaceAverage(const dg::Grid2d& g2d, const Collective& c, const container& f) :
    g2d_(g2d),
    f_(f),
    deltaf_(geo::DeltaFunction<Collective>(c,0.0,0.0)),
    w2d_ ( dg::create::weights( g2d_)),
    oneongrid_(dg::evaluate(dg::one,g2d_))              
    {
        thrust::host_vector<double> psipRog2d  = dg::evaluate( c.psipR, g2d_);
        thrust::host_vector<double> psipZog2d  = dg::evaluate( c.psipZ, g2d_);
        double psipRmax = (double)thrust::reduce( psipRog2d.begin(), psipRog2d.end(),  0.,     thrust::maximum<double>()  );    
        //double psipRmin = (double)thrust::reduce( psipRog2d.begin(), psipRog2d.end(),  psipRmax,thrust::minimum<double>()  );
        double psipZmax = (double)thrust::reduce( psipZog2d.begin(), psipZog2d.end(), 0.,      thrust::maximum<double>()  );    
        //double psipZmin = (double)thrust::reduce( psipZog2d.begin(), psipZog2d.end(), psipZmax,thrust::minimum<double>()  );   
        double deltapsi = fabs(psipZmax/g2d_.Ny()/g2d_.n() +psipRmax/g2d_.Nx()/g2d_.n());
        //deltaf_.setepsilon(deltapsi/4.);
        deltaf_.setepsilon(deltapsi); //macht weniger Zacken
    }
    /**
     * @brief Calculate the Flux Surface Average
     *
     * @param psip0 the actual psi value for q(psi)
     */
    double operator()(double psip0)
    {
        deltaf_.setpsi( psip0);
        container deltafog2d = dg::evaluate( deltaf_, g2d_);    
        double psipcut = dg::blas2::dot( f_,w2d_,deltafog2d); //int deltaf psip
        double vol     = dg::blas2::dot( oneongrid_ , w2d_,deltafog2d); //int deltaf
        double fsa = psipcut/vol;
        return fsa;
    }
    private:
    dg::Grid2d g2d_;
    container f_;
    geo::DeltaFunction<Collective> deltaf_;    
    const container w2d_;
    const container oneongrid_;
};
/**
 * @brief Class for the evaluation of the safety factor q
 * \f[ q(\psi_0) = \frac{1}{2\pi} \int dV |\nabla\psi_p| \delta(\psi_p-\psi_0) \alpha( R,Z) \f]
 * @tparam container  The container class to use
 * @tparam Collective This collective needs to contain at least the 2d functors, 
 * psip and its derivatives psipR and psipZ. 
 * @ingroup misc
 *
 */
template <class Collective, class container = thrust::host_vector<double> >
struct SafetyFactor
{
     /**
     * @brief Construct from a field and a grid
     * @param g2d 2d grid
     * @param c contains psip, psipR and psipZ
     * @param f container for global safety factor
     */
    SafetyFactor(const dg::Grid2d& g2d, const Collective& c, const container& f) :
    g2d_(g2d),
    f_(f), //why not directly use Alpha??
    deltaf_(geo::DeltaFunction<Collective>(c,0.0,0.0)),
    w2d_ ( dg::create::weights( g2d_)),
    oneongrid_(dg::evaluate(dg::one,g2d_))              
    {
      thrust::host_vector<double> psipRog2d  = dg::evaluate( c.psipR, g2d_);
      thrust::host_vector<double> psipZog2d  = dg::evaluate( c.psipZ, g2d_);
      double psipRmax = (double)thrust::reduce( psipRog2d.begin(), psipRog2d.end(), 0.,     thrust::maximum<double>()  );    
      //double psipRmin = (double)thrust::reduce( psipRog2d.begin(), psipRog2d.end(),  psipRmax,thrust::minimum<double>()  );
      double psipZmax = (double)thrust::reduce( psipZog2d.begin(), psipZog2d.end(), 0.,      thrust::maximum<double>()  );    
      //double psipZmin = (double)thrust::reduce( psipZog2d.begin(), psipZog2d.end(), psipZmax,thrust::minimum<double>()  );   
      double deltapsi = fabs(psipZmax/g2d_.Ny() +psipRmax/g2d_.Nx());
      //deltaf_.setepsilon(deltapsi/4.);
      deltaf_.setepsilon(4.*deltapsi); //macht weniger Zacken
    }
    /**
     * @brief Calculate the q profile over the function f which has to be the global safety factor
     * \f[ q(\psi_0) = \frac{1}{2\pi} \int dV |\nabla\psi_p| \delta(\psi_p-\psi_0) \alpha( R,Z) \f]
     *
     * @param psip0 the actual psi value for q(psi)
     */
    double operator()(double psip0)
    {
        deltaf_.setpsi( psip0);
        container deltafog2d = dg::evaluate( deltaf_, g2d_);    
        double q = dg::blas2::dot( f_,w2d_,deltafog2d)/(2.*M_PI);
        return q;
    }
    private:
    dg::Grid2d g2d_;
    container f_;
    geo::DeltaFunction<Collective> deltaf_;    
    const container w2d_;
    const container oneongrid_;
};

}//namespace dg
