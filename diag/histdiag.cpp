#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm> 

#include "dg/algorithm.h"
#include "dg/backend/interpolation.cuh"
#include "dg/backend/xspacelib.cuh"
#include "dg/functors.h"
#include "file/read_input.h"
#include "file/nc_utilities.h"


/**
 * @brief returns histogram 
 * @tparam container 
 */ 
template <class container = thrust::host_vector<double> >
struct Histogram
{
     /**
     * @brief Construct from number of bins and input vector
     * @param g1d   grid of output vector
     * @param in input vector
     */
    Histogram(const dg::Grid1d<double>& g1d, const std::vector<double>& in) :
    g1d_(g1d),
    in_(in),
    binwidth_(g1d_.h()),
    count_(dg::evaluate(dg::zero,g1d_))
    {
        for (unsigned j=0;j<in_.size();j++)
        {            
            unsigned bin =floor( (in_[j]-g1d_.x0())/binwidth_ );
            bin = std::max(bin,(unsigned) 0);
            bin = std::min(bin,(unsigned)(g1d_.size()-1));
            count_[bin ]+=1.;
        }
        //Normalize
        unsigned Ampmax = (unsigned)thrust::reduce( count_.begin(), count_.end(),0.,   thrust::maximum<double>()  );
        dg::blas1::scal(count_,1./Ampmax);
        
    }
    double binwidth() {return binwidth_;}
    double operator()(double x)
    {    
        unsigned bin = floor((x-g1d_.x0())/binwidth_+0.5);
        bin = std::max(bin,(unsigned) 0);
        bin = std::min(bin,(unsigned)(g1d_.size()-1));
        return count_[bin];
    }

    private:
    dg::Grid1d<double> g1d_;
    const std::vector<double> in_;
    double binwidth_;
    container  count_;
};
template <class container = thrust::host_vector<double> >
struct Histogram2D
{
     /**
     * @brief Construct from number of bins and input vector
     * @param g2d   grid of output vector
     * @param inx input vector in x direction
     * @param iny input vector in y direction
     */
    Histogram2D(const dg::Grid2d<double>& g2d, const std::vector<double>& inx,const std::vector<double>& iny) :
    g2d_(g2d),
    inx_(inx),
    iny_(iny),
    binwidthx_(g2d_.hx()),
    binwidthy_(g2d_.hy()),
    count_(dg::evaluate(dg::zero,g2d_))
    {

        for (unsigned j=0;j<iny_.size();j++)
        {
            unsigned biny =floor((iny_[j]-g2d_.y0())/binwidthy_) ;
            biny = std::max(biny,(unsigned) 0);
            biny = std::min(biny,(unsigned)(g2d_.Ny()-1));

                unsigned binx =floor((inx_[j]-g2d_.x0())/binwidthx_) ;
                binx = std::max(binx,(unsigned) 0);
                binx = std::min(binx,(unsigned)(g2d_.Nx()-1));
                count_[biny*g2d_.Nx()+binx ]+=1.;
            
        }
        //Normalize
        unsigned Ampmax =  (unsigned)thrust::reduce( count_.begin(),   count_.end(),0.,thrust::maximum<double>()  );   
        dg::blas1::scal(count_,  1./Ampmax);

    }

    double operator()(double x, double y)
    {
        unsigned binx = floor((x-g2d_.x0())/binwidthx_+0.5) ;
        binx = std::max(binx,(unsigned) 0);
        binx = std::min(binx,(unsigned)(g2d_.Nx()-1));
        unsigned biny = floor((y-g2d_.y0())/binwidthy_+0.5) ;
        biny = std::max(biny,(unsigned) 0);
        biny = std::min(biny,(unsigned)(g2d_.Ny()-1));
        return count_[biny*g2d_.Nx()+binx ]; 

    }
    private:
    dg::Grid2d<double> g2d_;
    const std::vector<double> inx_,iny_;
    double binwidthx_,binwidthy_;
    container count_;
};
/**
 * @brief normalizes input vector 
 */ 
void NormalizeToFluc(std::vector<double>& in) {
    double ex= 0.;
    double exx= 0.;
    double ex2= 0.;
    double sigma = 0.;    
    for (unsigned j=0;j<in.size();j++)
    {
        ex+=in[j];
        exx+=in[j]*in[j];
    }
    ex/=in.size();
    exx/=in.size();
    ex2=ex*ex;
    sigma=sqrt(exx-ex2);
    for (unsigned j=0;j<in.size();j++)
    {
        in[j] = (in[j]-  ex)/sigma; 
    }
    std::cout << "Sigma = " <<sigma << " Meanvalue = " << ex << std::endl;
}

int main( int argc, char* argv[])
{

    if( argc != 3)
    {
        std::cerr << "Usage: "<<argv[0]<<" [input.nc] [output.nc]\n";
        return -1;
    }
    std::cout << argv[1]<< " -> "<<argv[2]<<std::endl;   
    //----------------
    const unsigned Nhist = 100; 
    const unsigned nhist = 1;
    const unsigned Ninput =50000;
    const double Nsigma =4.;
    std::vector<double> input1(Ninput,0.);    
    std::vector<double> input2(Ninput,0.);    

    thrust::random::minstd_rand generator;
    thrust::random::normal_distribution<double> d1;
    thrust::random::normal_distribution<double> d2;
    std::vector<double> rand1(Ninput,0.);    
    std::vector<double> rand2(Ninput,0.);    
    for (unsigned i=0;i<rand1.size();i++)  {  rand1[i] = d1(generator); }
    for (unsigned i=0;i<rand2.size();i++)  {  rand2[i] = d2(generator); }

    for (unsigned i=0;i<input1.size();i++)  {
        double t = (double)(i/(input1.size()-1));
        double omega1 =2.*M_PI* 20.;
        input1[i] = (rand1[i]*0.1*cos( omega1*t)+1.); 
    }
    for (unsigned i=0;i<input2.size();i++)  {
        double t = (double)(i/(input2.size()-1));
        double omega1 = 2.*M_PI*20.;
        double omega2= 2.*M_PI*30.;
        double phase = 0.5*M_PI;
//         input2[i] =input1[i];  //perfectly correlated
        input2[i] = (-rand1[i]*0.1*cos(omega1*t)+1.);//perfectly anticorrelated
//         input2[i] = (rand2[i]*0.001*cos(omega2*t)+3.);//perfectly uncorrelated
//         input2[i] = (rand2[i]*0.001*cos(omega2*t)+3.);//uncorrelated
    } 

    //normalize grid and compute sigma
    NormalizeToFluc(input1);
    NormalizeToFluc(input2);
    dg::Grid1d<double>  g1d1(-Nsigma,Nsigma, nhist, Nhist,dg::DIR);
    dg::Grid1d<double>  g1d2(-Nsigma,Nsigma, nhist, Nhist,dg::DIR); 
    dg::Grid2d<double>  g2d( -Nsigma,Nsigma,-Nsigma,Nsigma, nhist, Nhist,Nhist,dg::DIR,dg::DIR); 
    Histogram<dg::HVec> hist1(g1d1,input1);  
    Histogram<dg::HVec> hist2(g1d2,input2);    
    Histogram2D<dg::HVec> hist12(g2d,input1,input2);    

 
    dg::HVec PA1 = dg::evaluate(hist1,g1d1);
    dg::HVec A1 = dg::evaluate(dg::coo1,g1d1);
    dg::HVec PA2= dg::evaluate(hist2,g1d2);
    dg::HVec A2 = dg::evaluate(dg::coo1,g1d2);
    dg::HVec PA1A2= dg::evaluate(hist12,g2d);
    
    //-----------------NC output start
    int dataIDs1[2],dataIDs2[2],dataIDs12[1];
    int dim_ids1[1],dim_ids2[1],dim_ids12[2];
    int ncid;
    file::NC_Error_Handle err; 
    err = nc_create(argv[2],NC_NETCDF4|NC_CLOBBER, &ncid); 
    //plot 1
    err = file::define_dimension( ncid,"A1_", &dim_ids1[0],  g1d1);
    err = nc_def_var( ncid, "P(A1)",   NC_DOUBLE, 1, &dim_ids1[0], &dataIDs1[0]);
    err = nc_def_var( ncid, "A1",    NC_DOUBLE, 1, &dim_ids1[0], &dataIDs1[1]);
    err = nc_enddef( ncid);
    err = nc_put_var_double( ncid, dataIDs1[0], PA1.data() );
    err = nc_put_var_double( ncid, dataIDs1[1], A1.data() );
    err = nc_redef(ncid);
    //plot 2
    err = file::define_dimension( ncid,"A2_", &dim_ids2[0],  g1d2);
    err = nc_def_var( ncid, "P(A2)",   NC_DOUBLE, 1, &dim_ids2[0], &dataIDs2[0]);
    err = nc_def_var( ncid, "A2",    NC_DOUBLE, 1, &dim_ids2[0], &dataIDs2[1]);
    err = nc_enddef( ncid);
    err = nc_put_var_double( ncid, dataIDs2[0], PA2.data() );
    err = nc_put_var_double( ncid, dataIDs2[1], A2.data() );
    err = nc_redef(ncid);
    //plot12
//     dim_ids12[0]=dim_ids1[0];
//     dim_ids12[1]=dim_ids2[0];
    dim_ids12[0]=dataIDs1[0];
    dim_ids12[1]=dataIDs2[0];
    err = file::define_dimensions( ncid, &dim_ids12[0],  g2d);
    err = nc_def_var( ncid, "P(A1,A2)",   NC_DOUBLE, 2, &dim_ids12[0], &dataIDs12[0]);
    err = nc_enddef( ncid);
    err = nc_put_var_double( ncid, dataIDs12[0], PA1A2.data() );
    err = nc_redef(ncid);
    nc_close( ncid);

    return 0;
}

