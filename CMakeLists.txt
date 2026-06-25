cmake_minimum_required(VERSION 3.20)

project(Marshal LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)



option(MARSHAL_HAVE_AVX2 "Enable AVX2 fused microkernels" ON)

option(MARSHAL_WERROR "Treat compiler warnings as errors" ON)

option(MARSHAL_BUILD_BENCH "Build performance benchmarks" ON)



find_package(OpenMP)

find_package(Python3 COMPONENTS Interpreter)



set(MARSHAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)

set(MARSHAL_GENERATED_DIR ${MARSHAL_SOURCE_DIR}/Generated)



include_directories(

    ${MARSHAL_SOURCE_DIR}

    ${MARSHAL_SOURCE_DIR}/Marshal

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn

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

    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/CombinedDiracFast.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/FusedHotPaths.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/ScaleHotPaths.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Kernel/SoaStreaming.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Numerics/Support.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Quotient/QuotientToy.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Config.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/TraceApi.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/TraceEngine.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/IO/ZeroLoader.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/IO/ZeroIngest.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Ntz/NtzGenerate.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Ntz/ZeroVerify.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/ArchDetail.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/Induction.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/SpectralHp.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/SpectralDiagnostic.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/MeasureLimitSweep.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/WeilConvergenceStudy.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/CrossSectorWeilStudy.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/CrossSectorAnalyticBounds.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/CrossSectorWeilOperator.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/ScrewFunctionKernel.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/CrossSectorStep5Bounds.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/CrossSectorPfZeroCoupling.cxx

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

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/DualityGoldStandard.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/ConnesCrossedValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/AdelicCauchy.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/HeightMap.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/SpectralDeterminant.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/AssemblyCache.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/AssemblySearchValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/CompletionValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/ArchimedeanBoundary.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/ArchimedeanBoundaryValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Analysis/ConvergenceStudy.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Analysis/PairCorrelation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Induction/PairCorrelationRun.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsParser.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsModuleParser.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsKernel.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsInfer.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsMath.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsAlgebra.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsTransform.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsTypeInfer.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsForallExtension.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsInductionExtension.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsConvergenceExtension.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsProofLogic.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsProveSpine.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsProofAudit.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsSym.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/AnaVm.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/FormalCalibration.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/AnaFormal.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/AnaProofEngine.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsProofGate.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsLadderProofGate.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/MrsLadderProofEngine.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/AnaVM/ValidationRouter.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/SelfAdjointExtensionSweep.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/TraceFormulaGate.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/BerryKeatingValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/AnalyticConstructionValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/CombinedConnesDirac.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GLnArchimedeanOperator.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GLnLogPrimeBlock.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GLnTwistedCoupling.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/MarshalGLnDirac.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GLnLadderValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GLnLadderMetrics.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GL2EllipseHeegnerValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GL2GoldbachEffectiveValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GL2BSDEngine.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GL3HodgeEngine.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GLn/GL4YMEngine.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GlobalConnesLimit.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Quotient/RootedCausalDAG.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GlobalDiracLimitValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/AnalyticLemmaValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GenusOneWedgeValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GenusOneLogBoundsValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/MarshalRhUnconditionalProof.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/GenusOneLogBoundsNativeValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/XiHadamardEngine.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/SpectralActionValidation.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/T1TopologyValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Heat/PrimeArchCancelBudgetValidation.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Investigation/InvestigationTypes.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Investigation/DiagnosticExport.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Investigation/InvestigationRunner.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Investigation/DiagnosticEngine.cxx
    ${MARSHAL_SOURCE_DIR}/Marshal/Investigation/InvestigationConfig.cxx

    ${MARSHAL_SOURCE_DIR}/Marshal/Inference/InferenceEngine.cxx

)



add_library(MarshalKernel OBJECT ${MARSHAL_KERNEL_SOURCES})

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



add_executable(marshal-zero-verify ${MARSHAL_SOURCE_DIR}/Marshal/Ntz/ZeroVerifyCli.cxx)
target_compile_options(marshal-zero-verify PRIVATE -O3 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(marshal-zero-verify PRIVATE MarshalKernel m)
if(OpenMP_CXX_FOUND)
    target_link_libraries(marshal-zero-verify PRIVATE OpenMP::OpenMP_CXX)
endif()

add_executable(test-unit ${CMAKE_SOURCE_DIR}/tests/Unit/SupportTest.cxx)

add_executable(test-log-prime ${CMAKE_SOURCE_DIR}/tests/Unit/LogPrimeOperatorTest.cxx)

add_executable(test-anavm ${CMAKE_SOURCE_DIR}/tests/Unit/AnaVmSymTest.cxx)

add_executable(test-mrs-module ${CMAKE_SOURCE_DIR}/tests/Unit/MrsModuleTest.cxx)
target_compile_options(test-mrs-module PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-module PRIVATE MarshalKernel m)

add_executable(test-mrs-kernel ${CMAKE_SOURCE_DIR}/tests/Unit/MrsKernelTest.cxx)
target_compile_options(test-mrs-kernel PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-kernel PRIVATE MarshalKernel m)

add_executable(test-anavm-proof-engine ${CMAKE_SOURCE_DIR}/tests/Unit/AnaProofEngineTest.cxx)
target_compile_options(test-anavm-proof-engine PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-anavm-proof-engine PRIVATE MarshalKernel m)

add_executable(test-mrs-hadamard-proof-gate ${CMAKE_SOURCE_DIR}/tests/Unit/MrsHadamardProofGateTest.cxx)
target_compile_options(test-mrs-hadamard-proof-gate PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-hadamard-proof-gate PRIVATE MarshalKernel m)

add_executable(test-mrs-proof-audit ${CMAKE_SOURCE_DIR}/tests/Unit/MrsProofAuditTest.cxx)
target_compile_options(test-mrs-proof-audit PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-proof-audit PRIVATE MarshalKernel m)

add_executable(test-anavm-xi-hadamard-pipeline
    ${CMAKE_SOURCE_DIR}/tests/Integration/AnaVmXiHadamardPipelineTest.cxx)
target_compile_options(test-anavm-xi-hadamard-pipeline PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-anavm-xi-hadamard-pipeline PRIVATE MarshalKernel m)

add_executable(test-mrs-math ${CMAKE_SOURCE_DIR}/tests/Unit/MrsMathTest.cxx)
target_compile_options(test-mrs-math PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-math PRIVATE MarshalKernel m)
add_test(NAME MrsMathTest COMMAND test-mrs-math)

add_executable(test-mrs-transform ${CMAKE_SOURCE_DIR}/tests/Unit/MrsTransformTest.cxx)
target_compile_options(test-mrs-transform PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-transform PRIVATE MarshalKernel m)
add_test(NAME MrsTransformTest COMMAND test-mrs-transform)

add_executable(test-mrs-ladder-proof-gate ${CMAKE_SOURCE_DIR}/tests/Unit/MrsLadderProofGateTest.cxx)
target_compile_options(test-mrs-ladder-proof-gate PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-mrs-ladder-proof-gate PRIVATE MarshalKernel m)

add_executable(test-anavm-ladder-pipeline
    ${CMAKE_SOURCE_DIR}/tests/Integration/AnaVmLadderPipelineTest.cxx)
target_compile_options(test-anavm-ladder-pipeline PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-anavm-ladder-pipeline PRIVATE MarshalKernel m)

target_compile_options(test-unit PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})

target_link_libraries(test-unit PRIVATE MarshalKernel m)

target_compile_options(test-log-prime PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-log-prime PRIVATE MarshalKernel m)

target_compile_options(test-anavm PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-anavm PRIVATE MarshalKernel m)

add_executable(test-completion ${CMAKE_SOURCE_DIR}/tests/Unit/CompletionValidationTest.cxx)
target_compile_options(test-completion PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-completion PRIVATE MarshalKernel m)

add_executable(test-archimedean ${CMAKE_SOURCE_DIR}/tests/Unit/ArchimedeanBoundaryTest.cxx)
target_compile_options(test-archimedean PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-archimedean PRIVATE MarshalKernel m)

add_executable(test-bk ${CMAKE_SOURCE_DIR}/tests/Unit/BerryKeatingTest.cxx)
target_compile_options(test-bk PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-bk PRIVATE MarshalKernel m)

add_executable(test-validation-router ${CMAKE_SOURCE_DIR}/tests/Unit/ValidationRouterTest.cxx)
target_compile_options(test-validation-router PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-validation-router PRIVATE MarshalKernel m)

add_executable(test-investigation ${CMAKE_SOURCE_DIR}/tests/Unit/InvestigationTypesTest.cxx)
target_compile_options(test-investigation PRIVATE -O2 ${MARSHAL_WARN_FLAGS} ${MARSHAL_SIMD_FLAGS})
target_link_libraries(test-investigation PRIVATE MarshalKernel m)



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

add_test(NAME mrs-module COMMAND test-mrs-module WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME mrs-kernel COMMAND test-mrs-kernel WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME anavm COMMAND test-anavm WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME anavm-proof-engine COMMAND test-anavm-proof-engine WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME mrs-hadamard-proof-gate COMMAND test-mrs-hadamard-proof-gate WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_test(NAME mrs-proof-audit COMMAND test-mrs-proof-audit WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME anavm-xi-hadamard-pipeline COMMAND test-anavm-xi-hadamard-pipeline
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME completion COMMAND test-completion)

add_test(NAME archimedean COMMAND test-archimedean)

add_test(NAME batch-kernels COMMAND test-batch)

add_test(NAME validation-router COMMAND test-validation-router WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_test(NAME investigation-types COMMAND test-investigation)

add_test(NAME integration COMMAND test-integration WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})



add_custom_target(validate-lemmas

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateLemmas.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Validate cert proof_status against LemmaManifest.json"

)



add_custom_target(validate-epistemic-discipline

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateEpistemicDiscipline.py

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Epistemic discipline scan (tautology, hardcode, circular, status fraud)"

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



add_custom_target(verify-marshal-cert

    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalCert.py --check

    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMENT "Pinned Marshal cert matches analytic_lemma_demo.json"

)

add_custom_target(verify-odilyzko-zeros
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalOdilyzkoZeroCert.py --run --count 10000
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalOdilyzkoZeroCert.py --check
    DEPENDS marshal-zero-verify Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "NtzGenerate refine + Riemann-Siegel zero verification + cert sync"
)

add_custom_target(verify-xi-hadamard
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitWeilChebyshevReductionAudit.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitWeilChebyshevReductionAudit.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitGLnPositivitySynthesisCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitGLnPositivitySynthesisCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitScrewPcircleBridgeCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitScrewPcircleBridgeCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitPrimeTailLimitCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitPrimeTailLimitCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/RunCrossSectorWeilStudy.py --fast
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitCrossSectorWeilBattlePlanCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitCrossSectorBalanceCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitCrossSectorBalanceCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/CrossSectorWeilGridStudy.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/SuzukiDominationAttackStudy.py
    COMMAND Marshal --anavm programs/marshal_xi_hadamard.mrs --xi-hadamard-proof
            --zeros ${CMAKE_SOURCE_DIR}/tests/Fixtures/Zeros/NtzMergedOneLine.txt
            --max-zeros 10000 --prime-limit 50000
            --export-xi-hadamard-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_xi_hadamard_proof.json
            --export-xi-hadamard-proof-graph ${CMAKE_SOURCE_DIR}/docs/generated/anavm_xi_hadamard_proof_graph.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalXiHadamardEngineCert.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "AnaVM XiHadamard MRS proof engine + audit cert sync"
)

add_custom_target(verify-mrs-proof
    COMMAND test-mrs-module
    COMMAND test-mrs-kernel
    COMMAND test-anavm-proof-engine
    COMMAND test-mrs-proof-audit
    COMMAND test-mrs-hadamard-proof-gate
    COMMAND test-anavm-xi-hadamard-pipeline
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-xi-hadamard
    DEPENDS test-mrs-module test-mrs-kernel test-anavm-proof-engine test-mrs-proof-audit
            test-mrs-hadamard-proof-gate test-anavm-xi-hadamard-pipeline Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "MRS module/kernel + Hadamard proof gate + XiHadamard pipeline (primary RH track)"
)

add_custom_target(verify-gln-proofs-codegen
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GLnProofsCodegenTest.py
    DEPENDS verify-mrs-ladder Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "GLnProofs MRS prove-spine audit (JSON)"
)

add_custom_target(verify-gln-ladder
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/RunGLnLadderSweep.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GLnCertEmitter.py --rank 3
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GLnCertEmitter.py --check --rank 3
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/RunRootedDAGValidation.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/RunRootedDAGValidation.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/HolyFunctionDemo.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/HolyFunctionDemo.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GL4OutlookCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GL4OutlookCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/GLnCertEmitter.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "GL(n) ladder sweep + rank-3 contract + GL(4) outlook contract checks"
)

add_custom_target(verify-gln-balanced
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-gln-ladder
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-hodge-proof
    DEPENDS verify-gln-ladder verify-hodge-proof
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Balanced GL(n): C++ ladder + cert checks + MRS closure"
)

add_custom_target(verify-bsd-proof
    COMMAND Marshal --bsd-proof-engine --export-bsd-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_bsd_proof.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalBSDCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalBsdMillenniumCert.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "BSD MRS proof engine + rank + Millennium formula cert check"
)

add_custom_target(verify-hodge-proof
    COMMAND Marshal --hodge-proof-engine --export-hodge-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_hodge_proof.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalHodgeCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalHodgeMillenniumCert.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Hodge MRS proof engine + (1,1) + Millennium cert check"
)

add_custom_target(verify-ym-proof
    COMMAND Marshal --ym-proof-engine --export-ym-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_ym_proof.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalYMCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitYMFiniteVolumeGapCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalYMMillenniumCert.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "GL(4) Clay YM MRS proof engine + Millennium cert check"
)

add_custom_target(verify-ymh-proof
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitYMHHiggsCert.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "GL(4) Yang-Mills-Higgs tree-level mass-spectrum cert (REDUCTION rung)"
)

add_custom_target(verify-wdw-proof
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitWheelerDeWittCert.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Wheeler-DeWitt minisuperspace self-adjoint + spectral-gap cert (ANALYTIC_OPEN rung)"
)

add_custom_target(verify-qft-extensions
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-ym-proof
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitYMHHiggsCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitWheelerDeWittCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/QFTExtensionsMrsClosure.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "All QFT/QM rungs: YM continuum bridge + YMH Higgs sector + Wheeler-DeWitt"
)

add_custom_target(verify-goldbach-proof
    COMMAND Marshal --goldbach-proof-engine --export-goldbach-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_goldbach_proof.json
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalGoldbachCert.py --check
    DEPENDS Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Goldbach MRS proof engine + cert check"
)

add_custom_target(verify-t1-uniform
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitT1UniformResolventCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitT1UniformResolventCert.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "T1 infinite admissibility: uniform resolvent lower bound inf_p R_p(theta0) >= delta > 0 (unconditional)"
)

add_custom_target(verify-mrs-ladder
    COMMAND test-mrs-ladder-proof-gate
    COMMAND test-anavm-ladder-pipeline
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-mrs-proof
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitT1UniformResolventCert.py --check
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-gln-ladder
    COMMAND Marshal --mrs-ladder-proof-engine
        --export-mrs-ladder-audit ${CMAKE_SOURCE_DIR}/docs/generated/mrs_ladder_proof_audit.json
        --export-mrs-ladder-closure ${CMAKE_SOURCE_DIR}/docs/generated/mrs_ladder_closure.json
        --export-bsd-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_bsd_proof.json
        --export-hodge-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_hodge_proof.json
        --export-goldbach-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_goldbach_proof.json
        --export-ym-proof ${CMAKE_SOURCE_DIR}/docs/generated/anavm_ym_proof.json
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-ym-proof
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalLadderMrsClosure.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalLadderCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MrsChainHardening.py --check
    DEPENDS test-mrs-ladder-proof-gate test-anavm-ladder-pipeline Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Full MRS ladder: RH + BSD + Hodge + Goldbach + YM analytic closure"
)

add_custom_target(verify-clay-dossier
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitMarshalLadderCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MarshalYMCert.py --check
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/MrsChainHardening.py --check
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-qft-extensions
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Validators/ValidateEpistemicDiscipline.py
    DEPENDS verify-gln-proofs-codegen verify-qft-extensions Marshal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Clay dossier gate: full MRS ladder closure + pinned numerics + YM cert"
)

add_custom_target(verify-formal
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-mrs-proof
    COMMENT "Alias: formal verification = MRS proof gate (Lean removed)"
)

# CI gate: fixture manifest + full clay dossier (used by .github/workflows/ci.yml).
add_custom_target(verify-ci
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/CI/ValidateFixtures.py --check
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target verify-clay-dossier
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "CI gate: fixtures + full MRS ladder / clay dossier closure"
)

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



add_custom_target(verify-theorem-a-moment
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitTheoremAMomentCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitTheoremAMomentCert.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Theorem A moment balance cert (Hurwitz + rational bracket)"
)

add_custom_target(verify-prime-arch-cancel-budget
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitPrimeArchCancelBudgetCert.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/Analysis/EmitPrimeArchCancelBudgetCert.py --check
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Prime-arch cancellation budget cert at theta0=144/25"
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


