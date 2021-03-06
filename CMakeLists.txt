######################################################################
# CMake version and policies
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.12.0)

# CMP0074: CMake find_package will use <PackageName>_ROOT CMake variable
# and environment variable in search path.
# various Find<Package>.cmake modules may not follow this policy
# CMP0075: check_include_file_cxx and other follow CMAKE_REQUIRED_LIBRARIES
# when doing link check
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0075 NEW)
include(CMakeDependentOption)

######################################################################
# QMCPACK
######################################################################
PROJECT(qmcpack)

SET(QMCPACK_VERSION_MAJOR 3)
SET(QMCPACK_VERSION_MINOR 10)
SET(QMCPACK_VERSION_PATCH 9)
SET(QMCPACK_VERSION "${QMCPACK_VERSION_MAJOR}.${QMCPACK_VERSION_MINOR}.${QMCPACK_VERSION_PATCH}")

######################################################
# Directory where customize cmake files reside
######################################################
SET (PROJECT_CMAKE ${qmcpack_SOURCE_DIR}/CMake)

SET(QMCPACK_UNIT_TEST_DIR ${qmcpack_BINARY_DIR}/tests/bin)

######################################################################
# Verify Python3 available
######################################################################
INCLUDE(CMake/python.cmake)
find_package(Python3)
IF ( NOT Python3_FOUND )
  MESSAGE( FATAL_ERROR "Could not find required python3" )
ENDIF ( NOT Python3_FOUND )

######################################################################
# Check is PySCF is availabile within Python
######################################################################
TEST_PYTHON_MODULE(pyscf HAVE_PYSCF)
IF (NOT HAVE_PYSCF)
  MESSAGE( STATUS "Unable to import PySCF python module. PySCF tests will not be run." )
ELSE()
  MESSAGE( STATUS "Successfully imported PySCF python module." )
ENDIF()
  
######################################################################
# Verify QE executables present if QE_BIN specified
######################################################################
IF ( DEFINED QE_BIN )
  INCLUDE(CMake/run_qe.cmake)
  VERIFY_QE_PRESENT( ${QE_BIN} )
ENDIF()

######################################################################
# CTest
######################################################################
INCLUDE( "${qmcpack_SOURCE_DIR}/CMake/macros.cmake" )
SET( DROP_METHOD "http" )
SET( DROP_SITE "cdash.qmcpack.org" )
SET( DROP_LOCATION "/CDash/submit.php?project=QMCPACK" )
SET( TRIGGER_SITE "" )
SET( DROP_SITE_CDASH TRUE )
# Increase timeout per test over the default of 1500 seconds (25 minutes)
SET( DART_TESTING_TIMEOUT 3600 CACHE STRING "Maximum time for one test")
ENABLE_TESTING()
INCLUDE( CTest )

OPTION(QMC_SYMLINK_TEST_FILES "Use symbolic links for test files to save space.  Set to false to copy files instead." ON)


IF (QMC_SYMLINK_TEST_FILES)
  SET(SYMLINK_MSG "Using symbolic links for large test files may cause test failures if the build is installed on a separate filesystem from the source.  For example, Titan at OLCF.")
ELSE()
  SET(SYMLINK_MSG "Copying large test files uses more disk space than using symbolic links.")
ENDIF()
MESSAGE(STATUS "QMC_SYMLINK_TEST_FILES = ${QMC_SYMLINK_TEST_FILES}.  ${SYMLINK_MSG}")

# The following option disables the extremely slow setup of full system run tests
# This is a developer option
# It offers a more reasonable compile debug loop if CMakeLists.txt files are being changed
# These constom commands execution time far exceeds even the feature and compiler checks for time consumed
# when rerunning cmake.
#
# On leconte: rerun of cmake with
# slow custom commands 27 secs
# without 8 secs
OPTION(QMC_NO_SLOW_CUSTOM_TESTING_COMMANDS "Disable the slow cmake custom commands for integration tests." OFF)
MARK_AS_ADVANCED(QMC_NO_SLOW_CUSTOM_TESTING_COMMANDS)


######################################################################
# Build level
######################################################################

OPTION(QMC_BUILD_SANDBOX_ONLY "Build only applications in Sandbox directory" OFF)
IF ( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_TOOLCHAIN_FILE)
  SET( CMAKE_BUILD_TYPE Release )
ENDIF()
STRING( TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE )
MESSAGE( STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

######################################################################
#define the paths for library and executable for out-of-soruce compilation
######################################################################
SET (LIBRARY_OUTPUT_PATH ${qmcpack_BINARY_DIR}/lib CACHE PATH "Single output directory for building all libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${qmcpack_BINARY_DIR}/bin CACHE PATH "Single output directory for building all executables.")

##################################
# Set C++ standard used by QMCPACK
##################################
SET(QMC_CXX_STANDARD 14 CACHE STRING "QMCPACK C++ language standard")
IF(NOT QMC_CXX_STANDARD EQUAL 14)
  MESSAGE(WARNING "C++14 is the only language standard officially supported by this QMCPACK version. "
                  "Using other versions of the C++ standard is unsupported and done entirely at user's own risk.")
ENDIF()

######################################################################
# Set the compiler-time parameters
# WALKER_MAX_PROPERTIES max number of observables + 12 or so standard
#   props. Things like forces are per particle so you may need this to
#   rather large.  Each property increases walker size by
#   sizeof(FULLPRECREALTYPE)
# OHMMS_DIM =  dimension of the problem
# OHMMS_INDEXTYPE = type of index
# OHMMS_PRECISION  = base precision, float, double etc
# OHMMS_PRECISION_FULL  = full precision, double etc
# QMC_COMPLEX = true if using complex wavefunctions
# QMC_MPI =  enable MPI
# QMC_OMP = enable OMP
######################################################################
SET(WALKER_MAX_PROPERTIES 2048 CACHE STRING "Maximum number of properties tracked by walkers")
MARK_AS_ADVANCED(WALKER_MAX_PROPERTIES)
SET(OHMMS_DIM 3 CACHE STRING "Select physical dimension")
SET(OHMMS_INDEXTYPE int)
MESSAGE(STATUS "defining the float point precision")
SET(OHMMS_PRECISION_FULL double)
IF(QMC_CUDA)
  SET(QMC_MIXED_PRECISION 1 CACHE BOOL "Enable/disable mixed precision")
  SET(OHMMS_PRECISION double)
  SET(CUDA_PRECISION_FULL double)
  IF(QMC_MIXED_PRECISION)
    SET(CUDA_PRECISION float)
  ELSE(QMC_MIXED_PRECISION)
    SET(CUDA_PRECISION double)
  ENDIF(QMC_MIXED_PRECISION)
  MESSAGE("   Base precision = ${OHMMS_PRECISION}")
  MESSAGE("   Full precision = ${OHMMS_PRECISION_FULL}")
  MESSAGE("   CUDA base precision = ${CUDA_PRECISION}")
  MESSAGE("   CUDA full precision = ${CUDA_PRECISION_FULL}")
ELSE(QMC_CUDA)
  SET(QMC_MIXED_PRECISION 0 CACHE BOOL "Enable/disable mixed precision")
  IF(QMC_MIXED_PRECISION)
    SET(OHMMS_PRECISION float)
    SET(MIXED_PRECISION 1)
  ELSE(QMC_MIXED_PRECISION)
    SET(OHMMS_PRECISION double)
  ENDIF(QMC_MIXED_PRECISION)
  MESSAGE("   Base precision = ${OHMMS_PRECISION}")
  MESSAGE("   Full precision = ${OHMMS_PRECISION_FULL}")
ENDIF(QMC_CUDA)

# Code coverage
SET(GCOV_SUPPORTED FALSE)
OPTION(ENABLE_GCOV "Enable code coverage" OFF)

######################################################################
# enable MPI and OPENMP  if possible
######################################################################
OPTION(QMC_MPI "Enable/disable MPI" ON)
OPTION(QMC_OMP "Enable/disable OpenMP" ON)
OPTION(QMC_COMPLEX "Build for complex binary" OFF)
SET(ENABLE_SOA 1 CACHE BOOL "Enable/disable SoA optimization")
SET(QMC_CUDA 0 CACHE BOOL "Build with GPU support through CUDA")
OPTION(ENABLE_CUDA "Build with the second generation of GPU support through CUDA (production quality for AFQMC, experimental for real space)" OFF)
OPTION(ENABLE_HIP "Build with with GPU support through HIP" OFF)
OPTION(ENABLE_ROCM "Build with with GPU support through ROCM" OFF)
OPTION(ENABLE_ONEAPI "Build with GPU support through OneAPI" OFF)
OPTION(ENABLE_OFFLOAD "Enable OpenMP offload" OFF)
# Use CMake object library targets to workaround clang linker not being able to handle fat
# binary archives which contain both host and device codes, for example OpenMP offload regions.
# CMake does not propagate indirect object files by design.
# So the dependency must be explicitly specified with USE_OBJECT_TARGET.
CMAKE_DEPENDENT_OPTION(USE_OBJECT_TARGET "Use CMake object library target" OFF "ENABLE_OFFLOAD" OFF)

IF(USE_OBJECT_TARGET)
   IF(${CMAKE_VERSION} VERSION_LESS "3.14.0")
     MESSAGE(FATAL_ERROR "USE_OBJECT_TARGET=ON requires cmake 3.14.0 or above!")
   ENDIF()
ENDIF()
IF(ENABLE_CUDA AND QMC_CUDA)
  MESSAGE(FATAL_ERROR "ENABLE_CUDA=1 and QMC_CUDA=1 can not be set together!")
ENDIF(ENABLE_CUDA AND QMC_CUDA)

OPTION(QMC_VERBOSE_CONFIGURATION "Give details why specific tests are/are not added" OFF)

######################################################################
# set debug printout
# DEBUG_PSIBUFFER_ON keep track buffer's locator
######################################################################
SET(PRINT_DEBUG 0 CACHE BOOL "Enable/disable debug printing")
SET(DEBUG_PSIBUFFER_ON 0 CACHE BOOL "Debug messages for PSIBUFFER locator")
MARK_AS_ADVANCED(DEBUG_PSIBUFFER_ON)
SET(DEBUG_PER_STEP_ACCEPT_REJECT 0 CACHE BOOL "Print accepts and rejects at each step")
MARK_AS_ADVANCED(DEBUG_PER_STEP_ACCEPT_REJECT)

######################################################################
# Standard test
######################################################################
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})


######################################################################
# add macros to compile einspline if necessary
######################################################################
INCLUDE(${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckCXXSymbolExists.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)

CHECK_FUNCTION_EXISTS(posix_memalign HAVE_POSIX_MEMALIGN)

CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)

######################################################################
# Build options
# QMC_BUILD_STATIC build static/dynamic  libraries
# BUILD_QMCTOOLS   build utility programs
# MPIP_PROFILE     profile mpi performance
######################################################################
SET(BUILD_UNIT_TESTS 1 CACHE BOOL "Build unit tests")
SET(BUILD_LMYENGINE_INTERFACE 1 CACHE BOOL "Build LMY engine")
IF (QMC_CUDA AND BUILD_LMYENGINE_INTERFACE)
  MESSAGE(STATUS "LMY engine is not compatible with CUDA build! Disabling LMY engine")
  SET(BUILD_LMYENGINE_INTERFACE 0)
ENDIF()
IF(MIXED_PRECISION AND BUILD_LMYENGINE_INTERFACE)
  MESSAGE(STATUS "LMY engine is not compatible with CPU mixed precision build! Disabling LMY engine")
  SET(BUILD_LMYENGINE_INTERFACE 0)
ENDIF()
SET(BUILD_AFQMC 0 CACHE BOOL "Build with AFQMC")
SET(BUILD_AFQMC_WITH_NCCL 0 CACHE BOOL "Build AFQMC with NCCL library.")
# AFQMC requires MPI.
If (BUILD_AFQMC AND NOT QMC_MPI)
  MESSAGE(FATAL_ERROR "AFQMC requires building with MPI (QMC_MPI=1). Set BUILD_AFQMC=0 or configure MPI.")
ENDIF()
SET(BUILD_FCIQMC 0 CACHE BOOL "Build with FCIQMC")
#SET(BUILD_QMCTOOLS 1 CACHE BOOL "Build tools for QMCPACK")
#SET(MPIP_PROFILE 0 CACHE BOOL "Build with mpip for mpi profile")
#SET(BUILD_QMCTOOLS 1)
#SET(MPIP_PROFILE 0)
OPTION(QMC_BUILD_STATIC "Link to static libraries" OFF)
OPTION(ENABLE_TIMERS "Enable internal timers" ON)
OPTION(ENABLE_STACKTRACE "Enable use of boost::stacktrace" OFF)

######################################################################
# Install options
# INSTALL_NEXUS install Nexus alongside QMCPACK
######################################################################
SET(INSTALL_NEXUS 1 CACHE BOOL "Install Nexus alongside QMCPACK")

######################################################################
#list of options to determine libraries. First, try to use libraries available
#FindXYZ.cmake will set the values if successful
######################################################################
SET(HAVE_EINSPLINE 1) # to be removed
SET(HAVE_LIBXMLPP 0)
SET(HAVE_LIBBLITZ 0)
SET(HAVE_LIBGSL 0)
SET(MAC_VECLIB 0)

######################################################
# Git information (see src/CMakeLists.txt for the
# command to get repository information)
######################################################
FIND_PACKAGE(Git)

IF (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  SET(IS_GIT_PROJECT 1)
ENDIF()


######################################################
#COMPILER choose one of the cmake files to customize the compiler options
#If nothing is chosen, default settings by cmake will  be used.
#If the automatic detection does not work, comment out everything
#upto COMPILER MANUAL SELECTION and use one of the customized cmake file.
######################################################
INCLUDE(${CMAKE_ROOT}/Modules/CheckCCompilerFlag.cmake)
INCLUDE(${CMAKE_ROOT}/Modules/CheckCXXCompilerFlag.cmake)

######################################################
# DO NOT MODIFY BELOW
# If you like to customize your build, create a toolchain as instructed
######################################################
######################################################
# now set the compiler flags and the macros in src/config.h
######################################################
IF(CMAKE_TOOLCHAIN_FILE)
  MESSAGE(STATUS "Using ${CMAKE_TOOLCHAIN_FILE} toolchain ")
ELSE(CMAKE_TOOLCHAIN_FILE)
  MESSAGE(STATUS "Trying to figure out .....")
  SET(ENABLE_OPENMP 0)
  SET(HAVE_ACML 0)

  #------------------------
  # On Cray's machine
  #------------------------
  GET_FILENAME_COMPONENT(BASE_CXX_COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
  IF($ENV{CRAYPE_VERSION} MATCHES "." AND NOT BASE_CXX_COMPILER_NAME STREQUAL "CC")
    MESSAGE(WARNING "Cray Programming Environment has been detected but C++ compiler wrapper CC is not used! "
                    "Please use cc/CC as the C/C++ compiler to ensure correct cross-compiling for the compute nodes "
                    "unless you understand cross-compiling and intend not to use Cray compiler wrappers.")
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "CrayLinuxEnvironment")
    MESSAGE(STATUS "Running on a Cray machine.")

    IF(NOT $ENV{CRAYPE_LINK_TYPE} STREQUAL "dynamic")
      MESSAGE(WARNING "Cray Programming Environment uses static linking by default. "
                      "We prefer dynamic linking which makes library searching easier. "
                      "Set environment variable CRAYPE_LINK_TYPE to \"dynamic\" enables dynamic linking.\n")
    ENDIF()

    IF ( NOT DEFINED MPIEXEC_EXECUTABLE )
      FIND_PROGRAM(MPIEXEC_EXECUTABLE aprun)
      IF(NOT MPIEXEC_EXECUTABLE)
        # For slurm use srun
        FIND_PROGRAM(MPIEXEC_EXECUTABLE srun)
      ENDIF(NOT MPIEXEC_EXECUTABLE)
    ENDIF()
  ELSE()
    IF($ENV{CRAYPE_VERSION} MATCHES "." AND BASE_CXX_COMPILER_NAME STREQUAL "CC")
      MESSAGE(FATAL_ERROR "Cray compiler wrapper detected. Empty the build folder and rerun cmake with -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment added.")
    ENDIF()
  ENDIF()

  #------------------------------------
  # Identify the compiler -- This serves only to deal with case where only C or CXX is set, refactor? 
  #------------------------------------
  IF ( CMAKE_C_COMPILER_WORKS OR CMAKE_C_COMPILER_WORKS )
    IF( (CMAKE_CXX_COMPILER_ID MATCHES "GNU") OR ( CMAKE_C_COMPILER_ID MATCHES "GNU") )
      SET( COMPILER GNU )
    ELSEIF( (CMAKE_CXX_COMPILER_ID MATCHES "XL") OR ( CMAKE_C_COMPILER_ID MATCHES "XL") )
      SET( COMPILER IBM )
    ELSEIF( (CMAKE_CXX_COMPILER_ID MATCHES "Intel") OR (CMAKE_C_COMPILER_ID MATCHES "Intel") )
      SET( COMPILER Intel )
    ELSEIF( (CMAKE_CXX_COMPILER_ID MATCHES "PGI") OR (CMAKE_C_COMPILER_ID MATCHES "PGI") )
      SET( COMPILER PGI )
    ELSEIF( (CMAKE_CXX_COMPILER_ID MATCHES "Cray") OR (CMAKE_C_COMPILER_ID MATCHES "Cray") )
      SET( COMPILER Cray )
    ELSEIF( (CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR (CMAKE_C_COMPILER_ID MATCHES "Clang") )
      SET( COMPILER Clang )
    ELSE()
        SET(USING_DEFAULT TRUE)
        MESSAGE("${CMAKE_C_COMPILER_ID}")
        MESSAGE(WARNING "Unknown C/C++ compiler, default flags will be used")
    ENDIF()
  ELSE()
    MESSAGE(WARNING "No compiler identified")
  ENDIF()
  MESSAGE(STATUS "Compiler: ${COMPILER}")

  #------------------------------------
  # Include compiler-specific cmake file
  #------------------------------------
  IF( ${COMPILER} MATCHES "IBM" )
    INCLUDE(${PROJECT_CMAKE}/IBMCompilers.cmake)
  ELSEIF( ${COMPILER} MATCHES "Intel" )
    INCLUDE(${PROJECT_CMAKE}/IntelCompilers.cmake)
  ELSEIF( ${COMPILER} MATCHES "GNU" )
    INCLUDE(${PROJECT_CMAKE}/GNUCompilers.cmake)
  ELSEIF( ${COMPILER} MATCHES "Clang" )
    INCLUDE(${PROJECT_CMAKE}/ClangCompilers.cmake)
  ELSEIF( ${COMPILER} MATCHES "PGI" )
    INCLUDE(${PROJECT_CMAKE}/PGICompilers.cmake)
  ELSE()
    MESSAGE(WARNING "No default file for compiler (${COMPILER})")
  ENDIF()

ENDIF(CMAKE_TOOLCHAIN_FILE)

###############################################
# Set C++ minimum standard and run basic checks
###############################################
set(CMAKE_CXX_STANDARD ${QMC_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-----------------------------------------------------------------------
# Check that a C++ compiler is compatiable with the underlying libstdc++
#-----------------------------------------------------------------------
include(CMake/Testlibstdc++.cmake)

#---------------------------------------------------------
# Check that a C++ standard library is configured properly
#---------------------------------------------------------
IF(QMC_CXX_STANDARD GREATER_EQUAL 17)
  include(CMake/TestCxx17Library.cmake)
ELSE()
  include(CMake/TestCxx14Library.cmake)
ENDIF()

IF (ENABLE_GCOV)
  IF (NOT GCOV_SUPPORTED)
    MESSAGE(FATAL_ERROR "ENABLE_GCOV was specified but compiler does not support GCC coverage flag")
  ENDIF()
ENDIF(ENABLE_GCOV)

IF (BUILD_AFQMC AND NOT APPLE)
  LINK_LIBRARIES("rt")
ENDIF()

#-------------------------------------------------------------------
#  check MPI installation
#-------------------------------------------------------------------
IF(QMC_MPI)
  # for backward compatibility with MPIEXEC
  IF(DEFINED MPIEXEC AND NOT DEFINED MPIEXEC_EXECUTABLE)
    MESSAGE(WARNING "Setting MPIEXEC_EXECUTABLE based on MPIEXEC. MPIEXEC has been deprecated! Use MPIEXEC_EXECUTABLE instead!")
    SET(MPIEXEC_EXECUTABLE ${MPIEXEC})
  ENDIF(DEFINED MPIEXEC AND NOT DEFINED MPIEXEC_EXECUTABLE)
  SET(MPI_CXX_SKIP_MPICXX TRUE)
  IF(NOT CMAKE_CROSSCOMPILING)
    SET(MPI_DETERMINE_LIBRARY_VERSION TRUE)
  ENDIF()
  FIND_PACKAGE(MPI COMPONENTS CXX)

  IF(NOT MPI_FOUND)
    MESSAGE(FATAL_ERROR "MPI support not found! Provide MPI compiler wrappers or build without MPI by passing '-DQMC_MPI=0' to cmake.")
  ENDIF(NOT MPI_FOUND)

  IF(${MPI_CXX_LIBRARY_VERSION_STRING} MATCHES "MVAPICH2")
    STRING(REGEX REPLACE "\n" ";" ONE_LINE "${MPI_CXX_LIBRARY_VERSION_STRING}")
    STRING(REGEX REPLACE " +|\t" ";" ONE_LINE "${ONE_LINE}")
    LIST (GET ONE_LINE 3 MVAPICH2_VERSION)
    MESSAGE(STATUS "MVAPICH2 version ${MVAPICH2_VERSION}")
    IF(${MVAPICH2_VERSION} VERSION_LESS "2.4" AND NOT ${MPI_CXX_LIBRARY_VERSION_STRING} MATCHES "disable-registration-cache")
        MESSAGE(FATAL_ERROR "MVAPICH2 (version < 2.4) with registration cache enabled breaks QMCPACK. "
                            "Use a different MPI library or a MVAPICH2 >=2.4 "
                            "or build MVAPICH2 with --disable-registration-cache configure option.")
    ENDIF()
  ENDIF()

  IF(${MPI_CXX_LIBRARY_VERSION_STRING} MATCHES "Open MPI")
    IF(NOT MPIEXEC_PREFLAGS)
      SET(MPIEXEC_PREFLAGS "--bind-to;none")
      MESSAGE(STATUS "Detected Open MPI. Setting bind-to options for thread affinity in MPIEXEC_PREFLAGS.")
    ELSE(NOT MPIEXEC_PREFLAGS)
      IF(NOT "${MPIEXEC_PREFLAGS}" MATCHES "--bind-to")
        MESSAGE(WARNING "Default Open MPI thread affinity policy may bind all the threads to a single core and "
                        "significantly slow down testing. Add proper --bind-to options via MPIEXEC_PREFLAGS. "
                        "If you don't know exactly what affinity to add, "
                        "add '--bind-to;none' to your current MPIEXEC_PREFLAGS to stop this warning.")
      ENDIF()
    ENDIF(NOT MPIEXEC_PREFLAGS)
  ENDIF()

  IF(NOT ${CMAKE_CXX_COMPILER} STREQUAL ${MPI_CXX_COMPILER})
    SET(MPI_WARNING_LIST
        "Building MPI version without using MPI compiler wrappers.\n"
        "This may not build qmcpack correctly. To ensure the correct version, specify the compiler wrappers to cmake.\n"
        "For example: cmake -DCMAKE_C_COMPILER=mpicc -DCMAKE_CXX_COMPILER=mpic++\n"
        "To build without MPI, pass '-DQMC_MPI=0' to cmake")
    MESSAGE(WARNING ${MPI_WARNING_LIST})
  ENDIF()

  IF ( NOT TEST_MAX_PROCS )
    SET( TEST_MAX_PROCS 100 )
  ENDIF()

  MESSAGE(STATUS "MPI runner MPIEXEC_EXECUTABLE : ${MPIEXEC_EXECUTABLE}")
  MESSAGE(STATUS "MPIEXEC_NUMPROC_FLAG : ${MPIEXEC_NUMPROC_FLAG}")
  MESSAGE(STATUS "MPIEXEC_PREFLAGS : ${MPIEXEC_PREFLAGS}")
  STRING(REPLACE ";" " " MPIEXEC_PREFLAGS_PRINT "${MPIEXEC_PREFLAGS}")
  MESSAGE(STATUS "Tests run as : ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} NUM_PROCS ${MPIEXEC_PREFLAGS_PRINT} EXECUTABLE")

  SET(HAVE_MPI 1)
  MESSAGE(STATUS "MPI is enabled")
ELSE(QMC_MPI)
  SET(HAVE_MPI 0)
  MESSAGE(STATUS "MPI is disabled")
ENDIF(QMC_MPI)

####################################################################
#First check the required libraries. Abort if these are not found.
#Changed the fall-back versions to use similar conventions to 2.8.0 and higher
####################################################################
# use customized cmake file for boost so that we can deal with multiple
# installataions
# list of libraries to be linked with the main applications using I/O

#-------------------------------------------------------------------
# check lapack/blas
#-------------------------------------------------------------------
# Add Math::BLAS_LAPACK target
ADD_LIBRARY(Math::BLAS_LAPACK INTERFACE IMPORTED)

FOREACH(MKL_LIB_PATH IN ITEMS ${MKL_ROOT} $ENV{MKL_ROOT} $ENV{MKLROOT} $ENV{MKL_HOME})
  SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${MKL_LIB_PATH}/lib/intel64)
ENDFOREACH()
IF(NOT BLA_VENDOR)
  MESSAGE(STATUS "Trying to find LAPACK from Intel MKL")
  IF(QMC_OMP AND CMAKE_CXX_COMPILER_ID MATCHES "Intel" )
    SET(BLA_VENDOR Intel10_64lp)
  ELSE()
    SET(BLA_VENDOR Intel10_64lp_seq)
  ENDIF()
  find_package(LAPACK)
  IF(NOT LAPACK_FOUND)
    MESSAGE(STATUS "Intel MKL library files not found via FindLAPACK.")
    MESSAGE(STATUS "Trying to find alternative LAPACK libraries")
    SET(BLA_VENDOR All)
    find_package(LAPACK REQUIRED)
  ENDIF()
ELSE()
  MESSAGE(STATUS "Trying to find LAPACK library as requested BLA_VENDOR=${BLA_VENDOR}")
  find_package(LAPACK REQUIRED)
ENDIF()

MESSAGE(STATUS "LAPACK linker flags: ${LAPACK_LINKER_FLAGS}")
MESSAGE(STATUS "LAPACK libraries: ${LAPACK_LIBRARIES}")
SET_TARGET_PROPERTIES(Math::BLAS_LAPACK PROPERTIES INTERFACE_LINK_LIBRARIES "${LAPACK_LINKER_FLAGS};${LAPACK_LIBRARIES}")

# Detects MKL header files and other components.
IF(LAPACK_LIBRARIES MATCHES "mkl_core")
  INCLUDE(CMake/FindMKL.cmake)
ENDIF()

# AFQMC requires MKL sparse for good performance (roughly a factor of 2x)
IF (BUILD_AFQMC AND NOT MKL_FOUND)
  MESSAGE(WARNING "AFQMC - MKL not found, using simple sparse matrix routines.  Link with MKL sparse libraries for better performance.")
ENDIF()

#-------------------------------------------------------------------
#  set up scalar/vector math libraries
#-------------------------------------------------------------------
SET(ENABLE_MASS FALSE CACHE BOOL "ENABLE MASS scalar and vector math libraries for Power architecture")

# This needs to go before HAVE_SINCOS
IF(ENABLE_MASS)
  INCLUDE(CMake/FindIBMMASS.cmake)
ENDIF(ENABLE_MASS)

IF(NOT MASS_FOUND AND NOT HAVE_MKL_VML)
  MESSAGE(STATUS "No usable vector math library detected.")
  ADD_LIBRARY(Math::scalar_vector_functions INTERFACE IMPORTED)
ENDIF(NOT MASS_FOUND AND NOT HAVE_MKL_VML)

# CheckSincos relies on SINCOS_INCLUDE which may be modified based on vendor libraries
IF(NOT SINCOS_INCLUDE)
  SET(SINCOS_INCLUDE cmath)
ENDIF(NOT SINCOS_INCLUDE)
MESSAGE(STATUS "SINCOS_INCLUDE : ${SINCOS_INCLUDE}")
INCLUDE(${PROJECT_CMAKE}/CheckSincos.cmake)

#-------------------------------------------------------------------
#  set up FFTW3 library
#-------------------------------------------------------------------
SET( FFTW_FOUND 0 )
IF ( HAVE_MKL )
  IF ( HAVE_MKL_FFTW3 )
    SET( FFTW_FOUND 1 )
  ENDIF()
ENDIF()
IF ( NOT FFTW_FOUND )
  find_package(FFTW)
ENDIF()
IF(FFTW_FOUND)
  SET(HAVE_LIBFFTW 1)
  MESSAGE(STATUS "Found FFTW")
ELSE(FFTW_FOUND)
#  MESSAGE(STATUS "Disabling FFTW")
  MESSAGE(FATAL_ERROR "Could not find required library FFTW. Verify FFTW_HOME")
ENDIF(FFTW_FOUND)

SET(QMC_UTIL_LIBS ${QMC_UTIL_LIBS} ${FORTRAN_LIBRARIES})

#-------------------------------------------------------------------
#  set up Libxml2 library
#-------------------------------------------------------------------
find_package(ZLIB)
find_package(LibXml2 REQUIRED)

#-------------------------------------------------------------------
#  set up HDF5 library
#-------------------------------------------------------------------
IF(HAVE_MPI)
  SET(HDF5_PREFER_PARALLEL TRUE CACHE BOOL "Request parallel/serial HDF5 library")
ELSE(HAVE_MPI)
  SET(HDF5_PREFER_PARALLEL FALSE CACHE BOOL "Request parallel/serial HDF5 library")
  IF(HDF5_PREFER_PARALLEL)
    MESSAGE(FATAL_ERROR "Parallel HDF5 library cannot be selected with QMCPACK non-MPI build. "
                        "Please set HDF5_PREFER_PARALLEL=0.")
  ENDIF(HDF5_PREFER_PARALLEL)
ENDIF(HAVE_MPI)

if(QMC_BUILD_STATIC)
  MESSAGE(STATUS "Linking static HDF5 library")
  set(HDF5_USE_STATIC_LIBRARIES on)
else()
  MESSAGE(STATUS "Linking dynamic HDF5 library")
  set(HDF5_USE_STATIC_LIBRARIES off)
endif()

find_package(HDF5 COMPONENTS C)

IF(HDF5_FOUND)
  SET(HAVE_LIBHDF5 1)

  IF(HDF5_IS_PARALLEL)
    MESSAGE(STATUS "Parallel HDF5 library found")
    OPTION(ENABLE_PHDF5 "Enable code paths using parallel HDF5" ON)
  ELSE(HDF5_IS_PARALLEL)
    MESSAGE(STATUS "Serial HDF5 library found")
    OPTION(ENABLE_PHDF5 "Enable code paths using parallel HDF5" OFF)
    IF(ENABLE_PHDF5)
      IF(HAVE_MPI)
        MESSAGE(FATAL_ERROR "Parallel HDF5 code paths requested but serial HDF5 library found! "
                            "Please either provide parallel HDF5 library or set ENABLE_PHDF5=0.")
      ELSE(HAVE_MPI)
        MESSAGE(FATAL_ERROR "Parallel HDF5 code paths cannot be enabled on non-MPI builds! Please set ENABLE_PHDF5=0.")
      ENDIF(HAVE_MPI)
    ENDIF(ENABLE_PHDF5)
  ENDIF(HDF5_IS_PARALLEL)

  IF(ENABLE_PHDF5)
    MESSAGE(STATUS "Using HDF5 parallel collective I/O code paths")
  ELSE(ENABLE_PHDF5)
    MESSAGE(STATUS "Using HDF5 non-scalable serial I/O code paths")
  ENDIF(ENABLE_PHDF5)

  IF(HAVE_MPI AND NOT ENABLE_PHDF5)
    MESSAGE(WARNING "MPI builds may have potential performance loss not using parallel HDF5!")
  ENDIF()

  IF ( CMAKE_BUILD_TYPE AND HDF5_LIBRARIES_DEBUG )
    IF ( CMAKE_BUILD_TYPE MATCHES DEBUG )
      SET( HDF5_LIBRARIES ${HDF5_LIBRARIES_DEBUG} )
    ELSE()
      SET( HDF5_LIBRARIES ${HDF5_LIBRARIES_RELEASE} )
    ENDIF()
  ENDIF()

  ADD_LIBRARY(IO::HDF5 INTERFACE IMPORTED)
  SET_TARGET_PROPERTIES(IO::HDF5 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${HDF5_INCLUDE_DIR}"
                                            INTERFACE_COMPILE_DEFINITIONS "HAVE_LIBHDF5;H5_USE_16_API"
                                            INTERFACE_LINK_LIBRARIES "${HDF5_LIBRARIES}")
  IF(ENABLE_PHDF5)
    SET_PROPERTY(TARGET IO::HDF5 APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "ENABLE_PHDF5")
  ENDIF(ENABLE_PHDF5)
ELSE(HDF5_FOUND)
  MESSAGE(FATAL_ERROR "HDF5 not found. Set HDF5_ROOT")
ENDIF(HDF5_FOUND)

#-------------------------------------------------------------------
#  set up libboost library, header only
#-------------------------------------------------------------------
#make sure we can find boost if it's not in /usr
set(Boost_NO_BOOST_CMAKE on)
find_package(Boost 1.61.0 REQUIRED)
IF(Boost_FOUND)
  SET_PROPERTY(TARGET Boost::boost APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS "HAVE_LIBBOOST")
  MESSAGE(STATUS "Setting Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
ELSE()
  MESSAGE(FATAL_ERROR "Need boost 1.61.0 or higher.  Set BOOST_ROOT")
ENDIF()

#-------------------------------------------------------------------
#  set up CUDA compiler options and libraries
#-------------------------------------------------------------------
SET(HAVE_CUDA 0)
IF(QMC_CUDA OR ENABLE_CUDA)
  # FindCUDA default CUDA_PROPAGATE_HOST_FLAGS to ON but we prefer OFF
  # It happened -ffast-math from host caused numerical issue in CUDA kernels.
  OPTION(CUDA_PROPAGATE_HOST_FLAGS "Propagate C/CXX_FLAGS and friends to the host compiler via -Xcompile" OFF)
  FIND_PACKAGE(CUDA REQUIRED)
  SET(CUDA_LINK_LIBRARIES_KEYWORD PRIVATE)
  #set(CUDA_NVCC_FLAGS
  #  "-arch=sm_20;-Drestrict=__restrict__;-DNO_CUDA_MAIN;-O3;-use_fast_math")
  if(CUDA_NVCC_FLAGS MATCHES "arch")
    # User defined NVCC flags
    MESSAGE(STATUS "Setting CUDA FLAGS=${CUDA_NVCC_FLAGS}")
  else(CUDA_NVCC_FLAGS MATCHES "arch")
    # Automatically set the default NVCC flags
    SET(CUDA_NVCC_FLAGS "-Drestrict=__restrict__;-DNO_CUDA_MAIN;-std=c++14")
    if (QMC_COMPLEX)
      SET(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS};-DQMC_COMPLEX")
    endif()
    if ( CMAKE_BUILD_TYPE STREQUAL "DEBUG" )
      SET(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS};-g;-G")
    else()
      # Temporarily disable fast_math because it causes multiple test failures
      # SET(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS};-O3;-use_fast_math")
      SET(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS};-O3")
    ENDIF()
    SET( CUDA_ARCH sm_70 CACHE STRING "CUDA architecture sm_XX")
    SET(CUDA_NVCC_FLAGS "-arch=${CUDA_ARCH};${CUDA_NVCC_FLAGS}")
  endif(CUDA_NVCC_FLAGS MATCHES "arch")
  INCLUDE_DIRECTORIES(${CUDA_INCLUDE_DIRS})
  SET(HAVE_CUDA 1)
  MESSAGE("   CUDA_NVCC_FLAGS=${CUDA_NVCC_FLAGS}")
ELSE(QMC_CUDA OR ENABLE_CUDA)
  MESSAGE(STATUS "Disabling CUDA")
ENDIF(QMC_CUDA OR ENABLE_CUDA)

OPTION(USE_NVTX_API "Enable/disable NVTX regions in CUDA code." OFF)
IF(USE_NVTX_API)
  IF(HAVE_CUDA OR ENABLE_OFFLOAD)
    FIND_LIBRARY(NVTX_API_LIB
      NAME nvToolsExt
      HINTS ${CUDA_TOOLKIT_ROOT_DIR}
      PATH_SUFFIXES lib lib64)
    IF(NOT NVTX_API_LIB)
      MESSAGE(FATAL_ERROR "USE_NVTX_API set but NVTX_API_LIB not found")
    ENDIF(NOT NVTX_API_LIB)
    MESSAGE("CUDA nvToolsExt library: ${NVTX_API_LIB}")
    LINK_LIBRARIES(${NVTX_API_LIB})
  ENDIF(HAVE_CUDA OR ENABLE_OFFLOAD)
ENDIF(USE_NVTX_API)

#-------------------------------------------------------------------
#  set up ROCM compiler options and libraries
#-------------------------------------------------------------------
IF(ENABLE_ROCM)
  MESSAGE(STATUS "ROCM_ROOT: ${ROCM_ROOT}")
  ADD_LIBRARY(ROCM::libraries INTERFACE IMPORTED)
  # temporarily put rocsolver rocrand here for convenience, should be moved to Platforms.
  SET_TARGET_PROPERTIES(ROCM::libraries PROPERTIES INTERFACE_INCLUDE_DIRECTORIES  "${ROCM_ROOT}/include"
                                                   INTERFACE_LINK_LIBRARIES "-L${ROCM_ROOT}/lib;-lrocsolver;-lrocrand")
ENDIF(ENABLE_ROCM)

#-------------------------------------------------------------------
#  set up HIP compiler options
#-------------------------------------------------------------------
IF(ENABLE_HIP)
  IF(NOT ENABLE_ROCM)
    MESSAGE(FATAL_ERROR "ROCM is required to use HIP. Please set ENABLE_ROCM=ON.")
  ENDIF()
  SET(CMAKE_MODULE_PATH "${ROCM_ROOT}/hip/cmake" ${CMAKE_MODULE_PATH})
  FIND_PACKAGE(HIP REQUIRED)

  ADD_LIBRARY(HIP::HIP INTERFACE IMPORTED)
  # temporarily put hipsparse hipblas here for convenience, should be moved to Platforms.
  SET_TARGET_PROPERTIES(HIP::HIP PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ROCM_ROOT}/include"
                                            INTERFACE_COMPILE_DEFINITIONS "ENABLE_HIP"
                                            INTERFACE_LINK_LIBRARIES "-L${ROCM_ROOT}/lib;-lhipsparse;-lhipblas;-lamdhip64")
ENDIF(ENABLE_HIP)


#-------------------------------------------------------------------
#  set up OneAPI compiler options and libraries
#-------------------------------------------------------------------
IF(ENABLE_ONEAPI)
  #IF(NOT ONEAPI_ROOT)
  #  MESSAGE(FATAL_ERROR "ONEAPI_ROOT not set!")
  #ENDIF()
  #MESSAGE(STATUS "ONEAPI_ROOT: ${ONEAPI_ROOT}")

  ADD_LIBRARY(ONEAPI::DPCPP INTERFACE IMPORTED)
  SET_TARGET_PROPERTIES(ONEAPI::DPCPP PROPERTIES INTERFACE_COMPILE_OPTIONS "-fsycl"
                                                 INTERFACE_COMPILE_DEFINITIONS "ENABLE_SYCL;MKL_ILP64"
                                                 INTERFACE_LINK_OPTIONS "-fsycl"
                                                 INTERFACE_LINK_LIBRARIES "-lmkl_sycl;-lmkl_intel_ilp64;-lmkl_sequential;-lmkl_core")
ENDIF(ENABLE_ONEAPI)

######################################################
# Add optional macros
######################################################

#include qmcpack/src build/src
INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)

IF (USE_VTUNE_TASKS)
  IF (NOT ENABLE_TIMERS)
    MESSAGE(FATAL_ERROR "USE_VTUNE_TASKS is set, but timers are not enabled.  Set ENABLE_TIMERS=ON.")
  ENDIF()
  SET(USE_VTUNE_API 1)
ENDIF()

IF (USE_VTUNE_API)
  include(CheckIncludeFileCXX)
  CHECK_INCLUDE_FILE_CXX(ittnotify.h HAVE_ITTNOTIFY_H)
  IF (NOT HAVE_ITTNOTIFY_H)
    MESSAGE(FATAL_ERROR "USE_VTUNE_API is defined, but the ittnotify.h include file is not found.  Check that the correct include directory is present in CMAKE_CXX_FLAGS.")
  ENDIF()

  FIND_LIBRARY(VTUNE_ITTNOTIFY_LIBRARY ittnotify)
  IF (NOT VTUNE_ITTNOTIFY_LIBRARY)
    MESSAGE(FATAL_ERROR "USE_VTUNE_API is defined, but the ittnotify library is not found.  Check that correct library path is present in CMAKE_LIBRARY_PATH.")
  ENDIF()
  LINK_LIBRARIES("${VTUNE_ITTNOTIFY_LIBRARY}")
ENDIF()

OPTION(QMC_EXP_THREADING "Experimental non openmp threading models" OFF)
MARK_AS_ADVANCED(QMC_EXP_THREADING)
IF(QMC_EXP_THREADING)
  include(CheckAtomic)
  IF(HAVE_LIBATOMIC)
    LINK_LIBRARIES("${HAVE_LIBATOMIC}")
  ENDIF(HAVE_LIBATOMIC)
  ADD_DEFINITIONS(-DQMC_EXP_THREADING)
ENDIF(QMC_EXP_THREADING)

#include(ExternalProject)
#  set(einspline_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/einspline")
#  set(einspline_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/einspline")
#  set(einspline_CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${einspline_INSTALL_DIR})
#ExternalProject_Add(
#   einspline
#   SVN_REPOSITORY http://qmctools.googlecode.com/svn/ext-packages/einspline
#   )
#    INSTALL_DIR ${einspline_INSTALL_DIR}
#    CMAKE_ARGS ${einspline_CMAKE_ARGS}
#    )
#  find_package(einspline REQUIRED)
#  set(HAVE_EINSPLINE 1)


###############################################################
# for specific linker in the case of profiling like hpctoolkit.
###############################################################
if(CMAKE_CXX_LINKER)
  set(CMAKE_CXX_LINK_EXECUTABLE
  "${CMAKE_CXX_LINKER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS>  <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
  MESSAGE(STATUS "Linker is specified as '${CMAKE_CXX_LINKER}' instead of '${CMAKE_CXX_COMPILER}'")
endif()

###################################################
# Add user extra flags
###################################################
IF(QMC_INCLUDE)
  FOREACH ( tmp ${QMC_INCLUDE} )
    MESSAGE(STATUS "Adding '${tmp}' to include directories")
    INCLUDE_DIRECTORIES( "${tmp}" )
  ENDFOREACH()
ENDIF()
IF(QMC_EXTRA_LIBS)
  string(REPLACE " " ";" QMC_EXTRA_LIBS ${QMC_EXTRA_LIBS})
  SET(QMC_UTIL_LIBS ${QMC_UTIL_LIBS} ${QMC_EXTRA_LIBS})
  MESSAGE("QMC_UTIL_LIBS=${QMC_UTIL_LIBS}")
ENDIF()

# Avoid unused variables warnings (for now) by using for information purposes
IF(QMC_DATA)
MESSAGE(STATUS "QMC_DATA is ${QMC_DATA}")
ENDIF()
IF(ENABLE_GCOV)
MESSAGE(STATUS "GCOV is enabled")
ENDIF()

############################################################
# FLAGS at the project level
############################################################
#this might be redundant but maybe not in all CMake versions.
STRING(TOUPPER "${CMAKE_BUILD_TYPE}" THIS_CONFIG)

FOREACH(lang IN ITEMS C CXX)
  SET(PROJECT_CMAKE_${lang}_FLAGS ${CMAKE_${lang}_FLAGS})
  #pre 3.0 cmake does not have string CONCAT
  SET(TMP_PROJECT_CMAKE_${lang}_FLAGS "${PROJECT_CMAKE_${lang}_FLAGS} ${CMAKE_${lang}_FLAGS_${THIS_CONFIG}}")
  MESSAGE("Project ${lang}_FLAGS: ${TMP_PROJECT_CMAKE_${lang}_FLAGS}")
ENDFOREACH(lang IN ITEMS C CXX)

get_directory_property(TMP_PROJECT_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
MESSAGE("Project INCLUDE_DIRECTORIES: ${TMP_PROJECT_INCLUDE_DIRECTORIES}")

FOREACH(target_type IN ITEMS EXE SHARED)
  SET(PROJECT_CMAKE_${target_type}_LINKER_FLAGS ${CMAKE_${target_type}_LINKER_FLAGS})
  SET(TMP_PROJECT_CMAKE_${target_type}_LINKER_FLAGS "${PROJECT_CMAKE_${target_type}_LINKER_FLAGS} ${CMAKE_${target_type}_LINKER_FLAGS_${THIS_CONFIG}}")
  MESSAGE("Project ${target_type}_LINKER_FLAGS: ${TMP_PROJECT_CMAKE_${target_type}_LINKER_FLAGS}")
ENDFOREACH(target_type IN ITEMS EXE SHARED)


###################################################
# Done searching libraries. Start building qmcpack
###################################################

MESSAGE(STATUS "Ready to build qmcpack")

####################################################################
# add definitions
####################################################################

ADD_DEFINITIONS(-DHAVE_CONFIG_H -DUSE_REAL_STRUCT_FACTOR -DADD_)

EXECUTE_PROCESS(COMMAND hostname OUTPUT_VARIABLE QMC_HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
CONFIGURE_FILE(${qmcpack_SOURCE_DIR}/src/qmcpack.settings ${qmcpack_BINARY_DIR}/bin/qmcpack.settings)
INSTALL(FILES ${qmcpack_BINARY_DIR}/bin/qmcpack.settings DESTINATION bin)
IF(INSTALL_NEXUS)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${qmcpack_SOURCE_DIR}/nexus/install --leave_paths ${CMAKE_INSTALL_PREFIX}/bin)")
ENDIF()

SUBDIRS(src)

IF(NOT QMC_BUILD_SANDBOX_ONLY)
  IF( NOT QMC_NO_SLOW_CUSTOM_TESTING_COMMANDS )
    SUBDIRS(tests)
    SUBDIRS(examples)
  ENDIF()
  IF(IS_DIRECTORY "${PROJECT_SOURCE_DIR}/nexus" AND INSTALL_NEXUS)
    SUBDIRS(nexus/tests)
  ENDIF()
ENDIF()
