cmake_minimum_required(VERSION 3.20)

project(Marshal LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)



option(MARSHAL_HAVE_AVX2 "Enable AVX2 fused microkernels" ON)

option(MARSHAL_WERROR "Treat compiler warnings as errors" ON)

option(MARSHAL_BUILD_BENCH "Build performance benchmarks" ON)

option(MARSHAL_REQUIRE_LEAN "Require Lean for verify-formal target" OFF)



find_package(OpenMP)

find_package(Python3 COMPONENTS Interpreter)



set(MARSHAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)

set(MARSHAL_GENERATED_DIR ${MARSHAL_SOURCE_DIR}/Generated)



include_directories(

    ${MARSHAL_SOURCE_DIR}

    ${MARSHAL_SOURCE_DIR}/Marshal

    ${MARSHAL_GENERATED_DIR}

)



if(MARSHAL_WERROR)

    if(MSVC)

        set(MARSHAL_WARN_FLAGS /W4 /WX)

    else()

        set(MARSHAL_WARN_FLAGS -Wall -Wextra -Wpedantic -Werror)

    endif()

else()

    if(NOT MSVC)

        set(MARSHAL_WARN_FLAGS -Wall -Wextra)

    endif()

endif()



if(MARSHAL_HAVE_AVX2)

    if(MSVC)

        add_compile_definitions(MARSHAL_HAVE_AVX2)

        set(MARSHAL_SIMD_FLAGS /arch:AVX2)

    else()

        include(CheckCXXCompilerFlag)

        check_cxx_compiler_flag("-mavx2" MARSHAL_COMPILER_HAS_AVX2)

        if(MARSHAL_COMPILER_HAS_AVX2)

            add_compile_definitions(MARSHAL_HAVE_AVX2)

            set(MARSHAL_SIMD_FLAGS -mavx2 -mfma)

        endif()

    endif()

endif()



set(MARSHAL_KERNEL_SOURCES

    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/FusedHotPaths.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/ScaleHotPaths.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Numerics/Support.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Quotient/QuotientToy.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Config.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/TraceApi.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/TraceEngine.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/IO/ZeroLoader.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/ArchDetail.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/Induction.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/SpectralHp.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/SpectralDiagnostic.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/MeasureLimitSweep.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/OperatorCandidateEval.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/OperatorInference.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/OperatorTraits.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/HpProof.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/HeatLadder.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/InductionCli.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/PrimeSieve.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/PrimeCache.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/PrimeCatalog.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/HeatCylinderOperator.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/HeatTraceSweep.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/LogPrimeValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Analysis/ConvergenceStudy.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Analysis/PairCorrelation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/PairCorrelationRun.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsParser.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsSym.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/AnaVm.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/FormalCalibration.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/AnaFormal.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Inference/InferenceEngine.cxx

)



add_library(MarshalKernel STATIC ${MARSHAL_KERNEL_SOURCES})

target_compile_options(MarshalKernel PRIVATE -O3 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})

if(OpenMP_CXX_FOUND)

    target_link_libraries(MarshalKernel PUBLIC OpenMP::OpenMP_CXX)

endif()



add_executable(Marshal ${MARSHAL_SOURCE_DIR}/Marshal/MarshalDriver.cxx)

target_compile_options(Marshal PRIVATE -O3 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})

target_link_libraries(Marshal PRIVATE MarshalKernel m)

if(OpenMP_CXX_FOUND)

    target_link_libraries(Marshal PRIVATE OpenMP::OpenMP_CXX)

endif()



add_executable(test-unit ${CMAKE_SOURCE_DIR}/tests/Unit/SupportTest.cxx)

add_executable(test-log-prime ${CMAKE_SOURCE_DIR}/tests/Unit/LogPrimeOperatorTest.cxx)

add_executable(test-anavm ${CMAKE_SOURCE_DIR}/tests/Unit/AnaVmSymTest.cxx)

target_compile_options(test-unit PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})

target_link_libraries(test-unit PRIVATE MarshalKernel m)

target_compile_options(test-log-prime PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-log-prime PRIVATE MarshalKernel m)

target_compile_options(test-anavm PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-anavm PRIVATE MarshalKernel m)



add_executable(test-batch ${CMAKE_SOURCE_DIR}/tests/Unit/BatchKernelTest.cxx)

target_compile_options(test-batch PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})

target_link_libraries(test-batch PRIVATE MarshalKernel m)

if(OpenMP_CXX_FOUND)

    target_link_libraries(test-batch PRIVATE OpenMP::OpenMP_CXX)

endif()



if(MARSHAL_BUILD_BENCH)

    add_executable(bench-micro ${CMAKE_SOURCE_DIR}/tests/Benchmarks/MicrokernelBench.cxx)

    target_compile_options(bench-micro PRIVATE -O3 ${MARSHAL_SIMD_FLAGS})

    target_link_libraries(bench-micro PRIVATE MarshalKernel m)

    add_executable(bench-heat ${CMAKE_SOURCE_DIR}/tests/Benchmarks/HeatTraceBench.cxx)

    target_compile_options(bench-heat PRIVATE -O3 ${MARSHAL_SIMD_FLAGS})

    target_link_libraries(bench-heat PRIVATE MarshalKernel m)

endif()



add_executable(test-integration ${CMAKE_SOURCE_DIR}/tests/Integration/NtzParityTest.cxx)

target_compile_options(test-integration PRIVATE -O2)

target_include_directories(test-integration PRIVATE ${MARSHAL_SOURCE_DIR})



enable_testing()

add_test(NAME unit COMMAND test-unit)

add_test(NAME log-prime COMMAND test-log-prime)

add_test(NAME batch-kernels COMMAND test-batch)

add_test(NAME integration COMMAND test-integration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})



add_custom_target(validate-lemmas

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateLemmas.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Validate cert proof_status against LemmaManifest.json"

)



add_custom_target(validate-formal-calibration

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateFormalCalibration.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Validate measure-limit / formal calibration JSON (v1)"

)



add_custom_target(validate-numerics

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateInduction.py

            ${CMAKE_SOURCE_DIR}/tests/Fixtures/Certs/mini_induction.json

            --thresholds ${CMAKE_SOURCE_DIR}/tests/Fixtures/Thresholds.json

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Validate induction JSON against thresholds"

)



find_program(LAKE_EXECUTABLE lake)

if(LAKE_EXECUTABLE)

    add_custom_target(verify-formal

        COMMAND ${LAKE_EXECUTABLE} build

        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/Formal

        COMMENT "Lean formal skeleton check"

    )

else()

    add_custom_target(verify-formal

        COMMAND ${CMAKE_COMMAND} -E echo "SKIP: lake not found (install Lean 4 for verify-formal)"

    )

endif()



add_custom_target(generate-docs

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Ansatz/GenerateAnsatz.py

            --cert ${CMAKE_SOURCE_DIR}/build/cert/demo_cert.json

            --out ${CMAKE_SOURCE_DIR}/docs/Generated

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    DEPENDS validate-lemmas

    COMMENT "Generate ansatz docs (gated on validate-lemmas)"

)



add_custom_target(validate-analytical

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateAnalyticalWorkload.py

            ${CMAKE_SOURCE_DIR}/build/cert/demo_cert.json

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Cross-check cert vs Analysis/Heat docs (manifest v1)"

)



add_custom_target(e2e

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Workload/RunE2E.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Full numerical-analytical E2E workload"

)



add_custom_target(hp-catalog

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Workload/RunHpCatalog.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "HP catalog proof workload (664k primes, high-precision NTZ)"

)



add_custom_target(check

    DEPENDS Marshal test-unit test-batch validate-lemmas validate-numerics

)


