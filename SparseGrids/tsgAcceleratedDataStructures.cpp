/*
 * Copyright (c) 2017, Miroslav Stoyanov
 *
 * This file is part of
 * Toolkit for Adaptive Stochastic Modeling And Non-Intrusive ApproximatioN: TASMANIAN
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * UT-BATTELLE, LLC AND THE UNITED STATES GOVERNMENT MAKE NO REPRESENTATIONS AND DISCLAIM ALL WARRANTIES, BOTH EXPRESSED AND IMPLIED.
 * THERE ARE NO EXPRESS OR IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR THAT THE USE OF THE SOFTWARE WILL NOT INFRINGE ANY PATENT,
 * COPYRIGHT, TRADEMARK, OR OTHER PROPRIETARY RIGHTS, OR THAT THE SOFTWARE WILL ACCOMPLISH THE INTENDED RESULTS OR THAT THE SOFTWARE OR ITS USE WILL NOT RESULT IN INJURY OR DAMAGE.
 * THE USER ASSUMES RESPONSIBILITY FOR ALL LIABILITIES, PENALTIES, FINES, CLAIMS, CAUSES OF ACTION, AND COSTS AND EXPENSES, CAUSED BY, RESULTING FROM OR ARISING OUT OF,
 * IN WHOLE OR IN PART THE USE, STORAGE OR DISPOSAL OF THE SOFTWARE.
 */

#ifndef __TASMANIAN_SPARSE_GRID_ACCELERATED_DATA_STRUCTURES_CPP
#define __TASMANIAN_SPARSE_GRID_ACCELERATED_DATA_STRUCTURES_CPP

#include "tsgAcceleratedDataStructures.hpp"

#include "tsgCudaMacros.hpp"

#ifdef Tasmanian_ENABLE_MAGMA
#include "magma_v2.h"
#include "magmasparse.h"
#endif

namespace TasGrid{

#ifdef Tasmanian_ENABLE_CUDA
cudaInts::cudaInts() : num(0), gpu_data(0){}
cudaInts::cudaInts(const std::vector<int> &cpu_data) : num(cpu_data.size()){
    gpu_data = TasCUDA::cudaNew<int>(num);
    TasCUDA::cudaSend<int>(num, cpu_data.data(), gpu_data);
}
cudaInts::~cudaInts(){ clear(); }

size_t cudaInts::size() const{ return num; }
int* cudaInts::data(){ return gpu_data; }
const int* cudaInts::data() const{ return gpu_data; }
void cudaInts::resize(size_t cnum){
    if (num != cnum) clear();
    num = cnum;
    if (gpu_data == 0) gpu_data = TasCUDA::cudaNew<int>(num);
}
void cudaInts::clear(){
    if (gpu_data != 0) TasCUDA::cudaDel<int>(gpu_data);
    gpu_data = 0;
    num = 0;
}

void cudaInts::load(const std::vector<int> &cpu_data){
    if (num != cpu_data.size()) clear();
    num = cpu_data.size();
    if (gpu_data == 0) gpu_data = TasCUDA::cudaNew<int>(num);
    TasCUDA::cudaSend<int>(num, cpu_data.data(), gpu_data);
}
void cudaInts::unload(std::vector<int> &cpu_data) const{
    cpu_data.resize(num);
    TasCUDA::cudaRecv<int>(num, gpu_data, cpu_data.data());
}
void cudaInts::eject(int* &destination){
    destination = gpu_data;
    gpu_data = 0;
    num = 0;
}

cudaDoubles::cudaDoubles() : num(0), gpu_data(0){}
cudaDoubles::cudaDoubles(size_t cnum) : num(cnum){ gpu_data = TasCUDA::cudaNew<double>(num); }
cudaDoubles::cudaDoubles(int a, int b) : num(((size_t) a) * ((size_t) b)){ gpu_data = TasCUDA::cudaNew<double>(num); }
cudaDoubles::cudaDoubles(int a, int b, const double *cpu_data) : num(((size_t) a) * ((size_t) b)){
    gpu_data = TasCUDA::cudaNew<double>(num);
    TasCUDA::cudaSend<double>(num, cpu_data, gpu_data);
}
cudaDoubles::cudaDoubles(const std::vector<double> &cpu_data) : num(cpu_data.size()){
    gpu_data = TasCUDA::cudaNew<double>(num);
    TasCUDA::cudaSend<double>(num, cpu_data.data(), gpu_data);
}
cudaDoubles::~cudaDoubles(){ clear(); }

size_t cudaDoubles::size() const{ return num; }
double* cudaDoubles::data(){ return gpu_data; }
const double* cudaDoubles::data() const{ return gpu_data; }
void cudaDoubles::resize(size_t cnum){
    if (num != cnum) clear();
    num = cnum;
    if (gpu_data == 0) gpu_data = TasCUDA::cudaNew<double>(num);
}
void cudaDoubles::clear(){
    if (gpu_data != 0) TasCUDA::cudaDel<double>(gpu_data);
    gpu_data = 0;
    num = 0;
}

void cudaDoubles::load(size_t cnum, const double *cpu_data){
    if (num != cnum) clear();
    num = cnum;
    if (gpu_data == 0) gpu_data = TasCUDA::cudaNew<double>(num);
    TasCUDA::cudaSend<double>(num, cpu_data, gpu_data);
}
void cudaDoubles::load(const std::vector<double> &cpu_data){
    if (num != cpu_data.size()) clear();
    num = cpu_data.size();
    if (gpu_data == 0) gpu_data = TasCUDA::cudaNew<double>(num);
    TasCUDA::cudaSend<double>(num, cpu_data.data(), gpu_data);
}
void cudaDoubles::unload(double *cpu_data) const{ TasCUDA::cudaRecv<double>(num, gpu_data, cpu_data); }
void cudaDoubles::eject(double* &destination){
    destination = gpu_data;
    gpu_data = 0;
    num = 0;
}

LinearAlgebraEngineGPU::LinearAlgebraEngineGPU() : cublasHandle(0), cusparseHandle(0)
#ifdef Tasmanian_ENABLE_MAGMA
    , magma_initialized(false), // call init once per object (must simplify later)
    magmaCudaStream(0), magmaCudaQueue(0)
#endif
{}
LinearAlgebraEngineGPU::~LinearAlgebraEngineGPU(){ reset(); }

void LinearAlgebraEngineGPU::reset(){
    if (cublasHandle != 0){
        cublasDestroy((cublasHandle_t) cublasHandle);
        cublasHandle = 0;
    }
    if (cusparseHandle != 0){
        cusparseDestroy((cusparseHandle_t) cusparseHandle);
        cusparseHandle = 0;
    }
    #ifdef Tasmanian_ENABLE_MAGMA
    if (magmaCudaQueue != 0) magma_queue_destroy((magma_queue*) magmaCudaQueue);
    magmaCudaQueue = 0;
    if (magma_initialized) magma_finalize();
    if (magmaCudaStream != 0) cudaStreamDestroy((cudaStream_t) magmaCudaStream);
    magmaCudaStream = 0;
    #endif
}

void LinearAlgebraEngineGPU::makeCuBlasHandle(){
    if (cublasHandle == 0){
        cublasHandle_t cbh;
        cublasCreate(&cbh);
        cublasHandle = (void*) cbh;
    }
}
void LinearAlgebraEngineGPU::makeCuSparseHandle(){
    if (cusparseHandle == 0){
        cusparseHandle_t csh;
        cusparseCreate(&csh);
        cusparseHandle = (void*) csh;
    }
}

void LinearAlgebraEngineGPU::cublasDGEMM(int M, int N, int K, double alpha, const cudaDoubles &A, const cudaDoubles &B, double beta, cudaDoubles &C){
    makeCuBlasHandle();
    size_t num_result = ((size_t) M) * ((size_t) N);
    if (C.size() != num_result) C.resize(num_result);
    if (N > 1){ // matrix-matrix mode
        cublasStatus_t stat = cublasDgemm((cublasHandle_t) cublasHandle, CUBLAS_OP_N, CUBLAS_OP_N, M, N, K,
                                        &alpha, A.data(), M, B.data(), K, &beta, C.data(), M);
        AccelerationMeta::cublasCheckError((void*) &stat, "cublasDgemm() in DGEMM");
    }else{ // matrix-vector mode
        cublasStatus_t stat= cublasDgemv((cublasHandle_t) cublasHandle, CUBLAS_OP_N, M, K,
                                        &alpha, A.data(), M, B.data(), 1, &beta, C.data(), 1);
        AccelerationMeta::cublasCheckError((void*) &stat, "cublasDgemv() in DGEMM");
    }
}
void LinearAlgebraEngineGPU::cublasDGEMM(int M, int N, int K, double alpha, const cudaDoubles &A, const std::vector<double> &B, double beta, cudaDoubles &C){
    cudaDoubles gpuB(B);
    size_t num_result = ((size_t) M) * ((size_t) N);
    if (C.size() != num_result) C.resize(num_result);
    cublasDGEMM(M, N, K, alpha, A, gpuB, beta, C);
}
void LinearAlgebraEngineGPU::cublasDGEMM(int M, int N, int K, double alpha, const cudaDoubles &A, const std::vector<double> &B, double beta, double C[]){
    cudaDoubles gpuB(B);
    size_t num_result = ((size_t) M) * ((size_t) N);
    cudaDoubles gpuC(num_result);
    cublasDGEMM(M, N, K, alpha, A, gpuB, beta, gpuC);
    gpuC.unload(C);
}

void LinearAlgebraEngineGPU::cusparseMatmul(int M, int N, int K, double alpha, const cudaDoubles &A, const std::vector<int> &spntr, const std::vector<int> &sindx, const std::vector<double> &svals, double beta, double C[]){
    cudaInts gpu_pntr(spntr);
    cudaInts gpu_indx(sindx);
    cudaDoubles gpu_vals(svals);
    size_t num_result = ((size_t) M) * ((size_t) N);
    cudaDoubles gpuC(num_result);
    cusparseMatmul(M, N, K, alpha, A, gpu_pntr, gpu_indx, gpu_vals, beta, gpuC);
    gpuC.unload(C);
}
void LinearAlgebraEngineGPU::cusparseMatmul(int M, int N, int K, double alpha, const cudaDoubles &A, const cudaInts &spntr, const cudaInts &sindx, const cudaDoubles &svals, double beta, cudaDoubles &C){
    makeCuBlasHandle();
    makeCuSparseHandle();

    size_t num_result = ((size_t) M) * ((size_t) N);
    if (C.size() != num_result) C.resize(num_result);
    cudaDoubles tempC(num_result);

    cusparseStatus_t stat_cuspar;
    cusparseMatDescr_t mat_desc;
    stat_cuspar = cusparseCreateMatDescr(&mat_desc);
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseCreateMatDescr() in LinearAlgebraEngineGPU::cusparseMatmul()");
    cusparseSetMatType(mat_desc, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(mat_desc, CUSPARSE_INDEX_BASE_ZERO);
    cusparseSetMatDiagType(mat_desc, CUSPARSE_DIAG_TYPE_NON_UNIT);

    stat_cuspar = cusparseDcsrmm2((cusparseHandle_t) cusparseHandle,
                                  CUSPARSE_OPERATION_NON_TRANSPOSE, CUSPARSE_OPERATION_TRANSPOSE, N, M, K, (int) sindx.size(),
                                  &alpha, mat_desc, svals.data(), spntr.data(), sindx.data(), A.data(), M, &beta, tempC.data(), N);
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseDcsrmm2() in LinearAlgebraEngineGPU::cusparseMatmul()");

    cusparseDestroyMatDescr(mat_desc);

    double talpha = 1.0, tbeta = 0.0;
    cublasStatus_t stat_cublas;
    stat_cublas = cublasDgeam((cublasHandle_t) cublasHandle,
                        CUBLAS_OP_T, CUBLAS_OP_T, M, N,
                        &talpha, tempC.data(), N, &tbeta, tempC.data(), N, C.data(), M);
    AccelerationMeta::cublasCheckError((void*) &stat_cublas, "cublasDgeam() in LinearAlgebraEngineGPU::cusparseMatmul()");
}
void LinearAlgebraEngineGPU::cusparseMatvec(int M, int N, double alpha, const cudaInts &spntr, const cudaInts &sindx, const cudaDoubles &svals, const cudaDoubles &x, double beta, double y[]){
    makeCuSparseHandle();
    cusparseStatus_t stat_cuspar;
    cusparseMatDescr_t mat_desc;
    stat_cuspar = cusparseCreateMatDescr(&mat_desc);
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseCreateMatDescr() in LinearAlgebraEngineGPU::cusparseMatvec()");
    cusparseSetMatType(mat_desc, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(mat_desc, CUSPARSE_INDEX_BASE_ZERO);
    cusparseSetMatDiagType(mat_desc, CUSPARSE_DIAG_TYPE_NON_UNIT);

    stat_cuspar = cusparseDcsrmv((cusparseHandle_t) cusparseHandle,
                                 CUSPARSE_OPERATION_NON_TRANSPOSE, M, N, (int) sindx.size(),
                                 &alpha, mat_desc, svals.data(), spntr.data(), sindx.data(), x.data(), &beta, y);
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseDcsrmv() in LinearAlgebraEngineGPU::cusparseMatvec()");

    cusparseDestroyMatDescr(mat_desc);
}
void LinearAlgebraEngineGPU::cusparseMatveci(int M, int K, double alpha, const cudaDoubles &A, const std::vector<int> &sindx, const std::vector<double> &svals, double beta, double C[]){
    makeCuSparseHandle();
    cusparseStatus_t stat_cuspar;

    // quote from Nvidia CUDA cusparse manual at https://docs.nvidia.com/cuda/cusparse/index.html#cusparse-lt-t-gt-gemvi
    // "This function requires no extra storage for the general matrices when operation CUSPARSE_OPERATION_NON_TRANSPOSE is selected."
    // Yet, buffer is required when num_nz exceeds 32 even with CUSPARSE_OPERATION_NON_TRANSPOSE
    int buffer_size;
    cudaDoubles gpu_buffer;
    stat_cuspar = cusparseDgemvi_bufferSize((cusparseHandle_t) cusparseHandle, CUSPARSE_OPERATION_NON_TRANSPOSE,
                                             M, K, (int) sindx.size(), &buffer_size);
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseDgemvi_bufferSize() in cusparseMatveci()");
    gpu_buffer.resize((size_t) buffer_size);

    cudaInts gpu_indx(sindx);
    cudaDoubles gpu_vals(svals);
    cudaDoubles gpuC(M);

    stat_cuspar = cusparseDgemvi((cusparseHandle_t) cusparseHandle,
                                  CUSPARSE_OPERATION_NON_TRANSPOSE, M, K, &alpha, A.data(), M, (int) sindx.size(), gpu_vals.data(),
                                  gpu_indx.data(), &beta, gpuC.data(), CUSPARSE_INDEX_BASE_ZERO, gpu_buffer.data());
    AccelerationMeta::cusparseCheckError((void*) &stat_cuspar, "cusparseDgemvi() in cusparseMatveci()");

    gpuC.unload(C);
}

#ifdef Tasmanian_ENABLE_MAGMA
void LinearAlgebraEngineGPU::initializeMagma(int gpuID){
    if (!magma_initialized){
        magma_init();
        magma_initialized = true;
    }
    makeCuBlasHandle();
    makeCuSparseHandle();
    magma_queue_create_from_cuda(gpuID, (cudaStream_t) magmaCudaStream, (cublasHandle_t) cublasHandle, (cusparseHandle_t) cusparseHandle, ((magma_queue**) &magmaCudaQueue));
}

void LinearAlgebraEngineGPU::magmaCudaDGEMM(int gpuID, int M, int N, int K, double alpha, const cudaDoubles &A, const cudaDoubles &B, double beta, cudaDoubles &C){
    initializeMagma(gpuID);
    size_t num_result = ((size_t) M) * ((size_t) N);
    magma_trans_t noTranspose = MagmaNoTrans;
    if (C.size() != num_result) C.resize(num_result);
    if (N > 1){ // matrix-matrix mode
        magma_dgemm(noTranspose, noTranspose, M, N, K, alpha, A.data(), M,
                    B.data(), K, beta, C.data(), M, (magma_queue_t) magmaCudaQueue);
    }else{ // matrix-vector mode
        magma_dgemv(noTranspose, M, K, alpha, A.data(), M,
                    B.data(), 1, beta, C.data(), 1, (magma_queue_t) magmaCudaQueue);
    }
}
void LinearAlgebraEngineGPU::magmaCudaDGEMM(int gpuID, int M, int N, int K, double alpha, const cudaDoubles &A, const std::vector<double> &B, double beta, cudaDoubles &C){
    cudaDoubles gpuB(B);
    size_t num_result = ((size_t) M) * ((size_t) N);
    if (C.size() != num_result) C.resize(num_result);
    magmaCudaDGEMM(gpuID, M, N, K, alpha, A, gpuB, beta, C);
}
void LinearAlgebraEngineGPU::magmaCudaDGEMM(int gpuID, int M, int N, int K, double alpha, const cudaDoubles &A, const std::vector<double> &B, double beta, double C[]){
    cudaDoubles gpuB(B);
    size_t num_result = ((size_t) M) * ((size_t) N);
    cudaDoubles gpuC(num_result);
    magmaCudaDGEMM(gpuID, M, N, K, alpha, A, gpuB, beta, gpuC);
    gpuC.unload(C);
}
#endif

#endif


TypeAcceleration AccelerationMeta::getIOAccelerationString(const char * name){
    if (strcmp(name, "cpu-blas") == 0){
        return accel_cpu_blas;
    }else if (strcmp(name, "gpu-default") == 0){
        return accel_gpu_default;
    }else if (strcmp(name, "gpu-cublas") == 0){
        return accel_gpu_cublas;
    }else if (strcmp(name, "gpu-cuda") == 0){
        return accel_gpu_cuda;
    }else if (strcmp(name, "gpu-magma") == 0){
        return accel_gpu_magma;
    }else{
        return accel_none;
    }
}
const char* AccelerationMeta::getIOAccelerationString(TypeAcceleration accel){
    switch (accel){
        case accel_cpu_blas:       return "cpu-blas";
        case accel_gpu_default:    return "gpu-default";
        case accel_gpu_cublas:     return "gpu-cublas";
        case accel_gpu_cuda:       return "gpu-cuda";
        case accel_gpu_magma:      return "gpu-magma";
        default: return "none";
    }
}
int AccelerationMeta::getIOAccelerationInt(TypeAcceleration accel){
    switch (accel){
        case accel_cpu_blas:       return 1;
        case accel_gpu_default:    return 3;
        case accel_gpu_cublas:     return 4;
        case accel_gpu_cuda:       return 5;
        case accel_gpu_magma:      return 6;
        default: return 0;
    }
}
TypeAcceleration AccelerationMeta::getIOIntAcceleration(int accel){
    switch (accel){
        case 1:  return accel_cpu_blas;
        case 3:  return accel_gpu_default;
        case 4:  return accel_gpu_cublas;
        case 5:  return accel_gpu_cuda;
        case 6:  return accel_gpu_magma;
        default: return accel_none;
    }
}
bool AccelerationMeta::isAccTypeFullMemoryGPU(TypeAcceleration accel){
    switch (accel){
        case accel_gpu_default:
        case accel_gpu_cublas:
        case accel_gpu_cuda:
        case accel_gpu_magma: return true;
        default:
            return false;
    }
}
bool AccelerationMeta::isAccTypeGPU(TypeAcceleration accel){
    switch (accel){
        case accel_gpu_default:
        case accel_gpu_cublas:
        case accel_gpu_cuda:
        case accel_gpu_magma: return true;
        default:
            return false;
    }
}

TypeAcceleration AccelerationMeta::getAvailableFallback(TypeAcceleration accel){
    // sparse grids are evaluated in 2 stages:
    // - s1: convert multi-index to matrix B
    // - s2: multiply matrix B by stored matrix A
    // Mode   | Stage 1 device | Stage 2 device | Library for stage 2
    // CUBLAS |      CPU       |     GPU        | Nvidia cuBlas (or cuSparse)
    // CUDA   |      GPU       |     GPU        | Nvidia cuBlas (or cuSparse)
    // MAGMA  |      GPU*      |     GPU        | UTK magma and magma_sparse
    // BLAS   |      CPU       |     CPU        | BLAS
    // none   | all done on CPU, still using OpenMP (if available)
    // *if CUDA is not simultaneously available with MAGMA, then MAGMA will use the CPU for stage 1
    // Note: using CUDA without either cuBlas or MAGMA is a bad idea (it will still work, just slow)

    // accel_gpu_default should always point to the potentially "best" option (currently MAGMA)
    if (accel == accel_gpu_default) accel = accel_gpu_magma;
    #if !defined(Tasmanian_ENABLE_CUDA) || !defined(Tasmanian_ENABLE_MAGMA) || !defined(Tasmanian_ENABLE_BLAS)
    // if any of the 3 acceleration modes is missing, then add a switch statement to guard against setting that mode
    switch(accel){
        #ifndef Tasmanian_ENABLE_CUDA
        // if CUDA is missing: just use the CPU
        case accel_gpu_cublas:
        case accel_gpu_cuda:
            #ifdef Tasmanian_ENABLE_BLAS
            accel = accel_cpu_blas;
            #else
            accel = accel_none;
            #endif
            break;
        #endif // Tasmanian_ENABLE_CUDA
        #ifndef Tasmanian_ENABLE_MAGMA
        // MAGMA tries to use CUDA kernels with magma linear algebra, this CUDA is the next best thing
        case accel_gpu_magma:
            #ifdef Tasmanian_ENABLE_CUDA
            accel = accel_gpu_cuda;
            #elif defined(Tasmanian_ENABLE_BLAS)
            accel = accel_cpu_blas;
            #else
            accel = accel_none;
            #endif
            break;
        #endif // Tasmanian_ENABLE_MAGMA
        #ifndef Tasmanian_ENABLE_BLAS
        // if BLAS is missing, do not attempt to use the GPU but go directly to "none" mode
        case accel_cpu_blas:
            accel = accel_none;
            break;
        #endif // Tasmanian_ENABLE_BLAS
        default: // compiler complains if there is no explicit "default", even if empty
            break;
    }
    #endif
    return accel;
}

#ifdef Tasmanian_ENABLE_CUDA
void AccelerationMeta::cudaCheckError(void *cudaStatus, const char *info){
    if (*((cudaError_t*) cudaStatus) != cudaSuccess){
        std::string message = "ERROR: cuda failed at ";
        message += info;
        message += " with error: ";
        message += cudaGetErrorString(*((cudaError_t*) cudaStatus));
        throw std::runtime_error(message);
    }
}
void AccelerationMeta::cublasCheckError(void *cublasStatus, const char *info){
    if (*((cublasStatus_t*) cublasStatus) != CUBLAS_STATUS_SUCCESS){
        std::string message = "ERROR: cuBlas failed with code: ";
        if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_NOT_INITIALIZED){
            message += "CUBLAS_STATUS_NOT_INITIALIZED";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_ALLOC_FAILED){
            message += "CUBLAS_STATUS_ALLOC_FAILED";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_INVALID_VALUE){
            message += "CUBLAS_STATUS_INVALID_VALUE";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_ARCH_MISMATCH){
            message += "CUBLAS_STATUS_ARCH_MISMATCH";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_MAPPING_ERROR){
            message += "CUBLAS_STATUS_MAPPING_ERROR";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_EXECUTION_FAILED){
            message += "CUBLAS_STATUS_EXECUTION_FAILED";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_INTERNAL_ERROR){
            message += "CUBLAS_STATUS_INTERNAL_ERROR";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_NOT_SUPPORTED){
            message += "CUBLAS_STATUS_NOT_SUPPORTED";
        }else if (*((cublasStatus_t*) cublasStatus) == CUBLAS_STATUS_LICENSE_ERROR){
            message += "CUBLAS_STATUS_LICENSE_ERROR";
        }else{
            message += "UNKNOWN";
        }
        message += " at ";
        message += info;
        throw std::runtime_error(message);
    }
}
void AccelerationMeta::cusparseCheckError(void *cusparseStatus, const char *info){
    if (*((cusparseStatus_t*) cusparseStatus) != CUSPARSE_STATUS_SUCCESS){
        std::string message = "ERROR: cuSparse failed with code: ";
        if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_NOT_INITIALIZED){
            message += "CUSPARSE_STATUS_NOT_INITIALIZED";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_ALLOC_FAILED){
            message += "CUSPARSE_STATUS_ALLOC_FAILED";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_INVALID_VALUE){
            message += "CUSPARSE_STATUS_INVALID_VALUE";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_ARCH_MISMATCH){
            message += "CUSPARSE_STATUS_ARCH_MISMATCH";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_INTERNAL_ERROR){
            message += "CUSPARSE_STATUS_INTERNAL_ERROR";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED){
            message += "CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED";
        }else if (*((cusparseStatus_t*) cusparseStatus) == CUSPARSE_STATUS_EXECUTION_FAILED){
            message += "CUSPARSE_STATUS_EXECUTION_FAILED";
        }else{
            message += "UNKNOWN";
        }
        message += " at ";
        message += info;
        throw std::runtime_error(message);
    }
}
#else
void AccelerationMeta::cudaCheckError(void *, const char *){}
void AccelerationMeta::cublasCheckError(void *, const char *){}
void AccelerationMeta::cusparseCheckError(void *, const char *){}
#endif // Tasmanian_ENABLE_CUDA


#ifdef Tasmanian_ENABLE_CUDA
AccelerationDomainTransform::AccelerationDomainTransform() : num_dimensions(0), padded_size(0){}
AccelerationDomainTransform::~AccelerationDomainTransform(){}

void AccelerationDomainTransform::clear(){
    num_dimensions = 0;
    padded_size = 0;
    gpu_trans_a.clear();
    gpu_trans_b.clear();
}
bool AccelerationDomainTransform::empty(){ return (num_dimensions == 0); }
void AccelerationDomainTransform::load(const std::vector<double> &transform_a, const std::vector<double> &transform_b){
    num_dimensions = (int) transform_a.size();
    padded_size = num_dimensions;
    while(padded_size < 512) padded_size += num_dimensions;

    std::vector<double> rate(padded_size);
    std::vector<double> shift(padded_size);
    int c = 0;
    for(int i=0; i<padded_size; i++){
        double diff = transform_b[c] - transform_a[c];
        rate[i] = 2.0 / diff;
        shift[i] = (transform_b[c] + transform_a[c]) / diff;
        c++;
        c = (c % num_dimensions);
    }

    gpu_trans_a.load(rate);
    gpu_trans_b.load(shift);
}
void AccelerationDomainTransform::getCanonicalPoints(bool use01, const double *gpu_transformed_x, int num_x, cudaDoubles &gpu_canonical_x){
    gpu_canonical_x.resize(((size_t) num_dimensions) * ((size_t) num_x));
    TasCUDA::dtrans2can(use01, num_dimensions, num_x, padded_size, gpu_trans_a.data(), gpu_trans_b.data(), gpu_transformed_x, gpu_canonical_x.data());
}
#endif // Tasmanian_ENABLE_CUDA


}

#endif
