#pragma once

namespace dg
{

// general multiply kernel
template<class value_type>
 __global__ void ell_multiply_kernel(
         const value_type* data, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols, const int blocks_per_line,
         const int n, const int size,
         const int right, 
         const value_type* x, value_type *y
         )
{
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int row = thread_id; row<size; row += grid_size)
    {
        int rr = row/right, rrn = rr/n;
        int s=rrn/num_rows, 
            i = (rrn)%num_rows, 
            k = (rr)%n, 
            j=row%right;
        int B, J;
        value_type temp=0;
        for( int d=0; d<blocks_per_line; d++)
        {
            B = (data_idx[i*blocks_per_line+d]*n+k)*n;
            J = (s*num_cols+cols_idx[i*blocks_per_line+d])*n;
            for( int q=0; q<n; q++) //multiplication-loop
                temp +=data[ B+q]* x[(J+q)*right+j];
            y[row]=temp;
        }
    }

}

// multiply kernel, n=3, 3 blocks per line
template<class value_type>
 __global__ void ell_multiply_kernel33(
         const value_type* data, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols,
         const int size,
         const int right, 
         const value_type* x, value_type *y
         )
{
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int row = thread_id; row<size; row += grid_size)
    {
        int rr = row/right, rrn = rr/3;
        int s=rrn/num_rows, 
            i = (rrn)%num_rows, 
            k = (rr)%3, 
            j=row%right;
        int B, J;
        value_type temp=0;
        {
            B = (data_idx[i*3+0]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+0])*3;
            temp +=data[ B+0]* x[(J+0)*right+j];
            temp +=data[ B+1]* x[(J+1)*right+j];
            temp +=data[ B+2]* x[(J+2)*right+j];
            B = (data_idx[i*3+1]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+1])*3;
            temp +=data[ B+0]* x[(J+0)*right+j];
            temp +=data[ B+1]* x[(J+1)*right+j];
            temp +=data[ B+2]* x[(J+2)*right+j];
            B = (data_idx[i*3+2]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+2])*3;
            temp +=data[ B+0]* x[(J+0)*right+j];
            temp +=data[ B+1]* x[(J+1)*right+j];
            temp +=data[ B+2]* x[(J+2)*right+j];
            y[row]=temp;
        }
    }
}

// multiply kernel, n=3, 2 blocks per line
template<class value_type>
 __global__ void ell_multiply_kernel32(
         const value_type* data, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols, 
         const int size,
         const int right, 
         const value_type* x, value_type *y
         )
{
    //int size = left*num_rows*n*right;
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int row = thread_id; row<size; row += grid_size)
    {
        int rr = row/right, rrn = rr/3;
        int s=rrn/num_rows, 
            i = (rrn)%num_rows, 
            k = (rr)%3, 
            j=row%right;
        int B, J;
        value_type temp=0;
        {
            B = (data_idx[i*2+0]*3+k)*3;
            J = (s*num_cols+cols_idx[i*2+0])*3;
            temp +=data[ B+0]* x[(J+0)*right+j];
            temp +=data[ B+1]* x[(J+1)*right+j];
            temp +=data[ B+2]* x[(J+2)*right+j];
            B = (data_idx[i*2+1]*3+k)*3;
            J = (s*num_cols+cols_idx[i*2+1])*3;
            temp +=data[ B+0]* x[(J+0)*right+j];
            temp +=data[ B+1]* x[(J+1)*right+j];
            temp +=data[ B+2]* x[(J+2)*right+j];
            y[row]=temp;
        }
    }

}

// multiply kernel, n=3, 3 blocks per line, right = 1
template<class value_type>
 __global__ void ell_multiply_kernel33x(
         const value_type* data, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols,
         const int size,
         const value_type* x, value_type *y
         )
{
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int row = thread_id; row<size; row += grid_size)
    {
        int rr = row/1, rrn = rr/3;
        int s=rrn/num_rows, 
            i = (rrn)%num_rows, 
            k = (rr)%3;
        int B, J;
        value_type temp=0;
        {
            B = (data_idx[i*3+0]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+0])*3;
            temp +=data[ B+0]* x[(J+0)];
            temp +=data[ B+1]* x[(J+1)];
            temp +=data[ B+2]* x[(J+2)];
            B = (data_idx[i*3+1]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+1])*3;
            temp +=data[ B+0]* x[(J+0)];
            temp +=data[ B+1]* x[(J+1)];
            temp +=data[ B+2]* x[(J+2)];
            B = (data_idx[i*3+2]*3+k)*3;
            J = (s*num_cols+cols_idx[i*3+2])*3;
            temp +=data[ B+0]* x[(J+0)];
            temp +=data[ B+1]* x[(J+1)];
            temp +=data[ B+2]* x[(J+2)];
            y[row]=temp;
        }
    }
}

// multiply kernel, n=3, 2 blocks per line, right = 1
template<class value_type>
 __global__ void ell_multiply_kernel32x(
         const value_type* data, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols,
         const int size,
         const value_type* x, value_type *y
         )
{
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int row = thread_id; row<size; row += grid_size)
    {
        int rrn = row/3, k = (row%3);
        int s=rrn/num_rows, i = (rrn)%num_rows;
        int B0,B1, J0, J1;
        value_type temp=0;
        {
            B0 = (data_idx[i*2+0]*3+k)*3;
            B1 = (data_idx[i*2+1]*3+k)*3;
            J0 = (s*num_cols+cols_idx[i*2+0])*3;
            J1 = (s*num_cols+cols_idx[i*2+1])*3;
            temp +=data[ B0+0]* x[(J0+0)];
            temp +=data[ B0+1]* x[(J0+1)];
            temp +=data[ B0+2]* x[(J0+2)];
            temp +=data[ B1+0]* x[(J1+0)];
            temp +=data[ B1+1]* x[(J1+1)];
            temp +=data[ B1+2]* x[(J1+2)];
            y[row]=temp;
        }
    }

}

// multiply kernel
template<class value_type>
 __global__ void coo_multiply_kernel(
         const value_type* data, const int* rows_idx, const int* cols_idx, const int* data_idx, 
         const int num_rows, const int num_cols, const int entry,
         const int n, 
         const int left, const int right, 
         value_type alpha, const value_type* x, value_type *y
         )
{
    int size = left*n*right;
    const int thread_id = blockDim.x * blockIdx.x + threadIdx.x;
    const int grid_size = gridDim.x*blockDim.x;
    //every thread takes num_rows/grid_size rows
    for( int idx = thread_id; idx<size; idx += grid_size)
    {
        int s=idx/(n*right), 
            k=(idx/right)%n, 
            j=idx%right;
        int I = ((s*num_rows+rows_idx[entry])*n+k)*right+j;
        value_type temp = 0;
        int B = data_idx[entry];
        int J = cols_idx[entry];
        for( int q=0; q<n; q++) //multiplication-loop
            temp += data[ (B*n + k)*n+q]* x[((s*num_cols + J)*n+q)*right+j];
        y[I] += alpha*temp;
    }

}

template<class value_type>
template<class DeviceContainer>
void EllSparseBlockMatDevice<value_type>::launch_multiply_kernel( const DeviceContainer& x, DeviceContainer& y) const
{
    assert( y.size() == (unsigned)num_rows*n*left*right);
    assert( x.size() == (unsigned)num_cols*n*left*right);
    //set up kernel parameters
    const size_t BLOCK_SIZE = 256; 
    const size_t size = left*right*num_rows*n;
    const size_t NUM_BLOCKS = std::min<size_t>((size-1)/BLOCK_SIZE+1, 65000);

    const value_type* data_ptr = thrust::raw_pointer_cast( &data[0]);
    const int* cols_ptr = thrust::raw_pointer_cast( &cols_idx[0]);
    const int* block_ptr = thrust::raw_pointer_cast( &data_idx[0]);
    const value_type* x_ptr = thrust::raw_pointer_cast( &x[0]);
    value_type* y_ptr = thrust::raw_pointer_cast( &y[0]);
    if( n == 3)
    {
        if( blocks_per_line == 3)
        {
            if( right == 1)
                ell_multiply_kernel33x<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( data_ptr, cols_ptr, block_ptr, num_rows, num_cols, size, x_ptr,y_ptr);
            else
                ell_multiply_kernel33<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( data_ptr, cols_ptr, block_ptr, num_rows, num_cols, size, right, x_ptr,y_ptr);
        }
        else if( blocks_per_line == 2)
        {
            if( right == 1)
                ell_multiply_kernel32x<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( data_ptr, cols_ptr, block_ptr, num_rows, num_cols, size, x_ptr,y_ptr);
            else
                ell_multiply_kernel32<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( data_ptr, cols_ptr, block_ptr, num_rows, num_cols, size, right, x_ptr,y_ptr);
        }
        else
            ell_multiply_kernel<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( 
                data_ptr, cols_ptr, block_ptr, num_rows, num_cols, blocks_per_line, 3, size, right, x_ptr,y_ptr);
    }
    else
        ell_multiply_kernel<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( 
            data_ptr, cols_ptr, block_ptr, num_rows, num_cols, blocks_per_line, n, size, right, x_ptr,y_ptr);
}

template<class value_type>
template<class DeviceContainer>
void CooSparseBlockMatDevice<value_type>::launch_multiply_kernel( value_type alpha, const DeviceContainer& x, value_type beta, DeviceContainer& y) const
{
    assert( y.size() == (unsigned)num_rows*n*left*right);
    assert( x.size() == (unsigned)num_cols*n*left*right);
    assert( beta == 1);
    //set up kernel parameters
    const size_t BLOCK_SIZE = 256; 
    const size_t size = left*right*n;
    const size_t NUM_BLOCKS = std::min<size_t>((size-1)/BLOCK_SIZE+1, 65000);

    const value_type* data_ptr = thrust::raw_pointer_cast( &data[0]);
    const int* rows_ptr = thrust::raw_pointer_cast( &rows_idx[0]);
    const int* cols_ptr = thrust::raw_pointer_cast( &cols_idx[0]);
    const int* block_ptr = thrust::raw_pointer_cast( &data_idx[0]);
    const value_type* x_ptr = thrust::raw_pointer_cast( &x[0]);
    value_type* y_ptr = thrust::raw_pointer_cast( &y[0]);
    for( int i=0; i<num_entries; i++)
    {
        coo_multiply_kernel<value_type> <<<NUM_BLOCKS, BLOCK_SIZE>>> ( 
            data_ptr, rows_ptr, cols_ptr, block_ptr, num_rows, num_cols, i, n, left, right, alpha, x_ptr,y_ptr);
    }
}

}//namespace dg

