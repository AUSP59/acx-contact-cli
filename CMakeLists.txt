cmake_minimum_required(VERSION 3.16)

option(ACX_WITH_SQLITE "Build with SQLite3 support" ON)
option(ACX_WITH_ZSTD "Build with Zstandard support" OFF)
option(ACX_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ACX_WITH_FUZZ "Build fuzz targets (clang/libFuzzer)" OFF)
option(ACX_WITH_COVERAGE "Build with coverage flags" OFF)
option(ACX_WITH_ROCKSDB "Build with RocksDB adapter" OFF)
project(acx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ACX_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND ACX_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

add_library(acxlib
  src/utils.cpp
  src/jsonl.cpp
  src/validation.cpp
  src/model.cpp
  src/storage.cpp
  src/audit.cpp
  src/log.cpp
  src/hmac.cpp
  src/sbom.cpp
  src/metrics.cpp
  src/index.cpp
  src/csv.cpp
  src/disk_index.cpp
  src/storage_jsonl.cpp
  src/storage_sqlite.cpp
  src/zstd_wrap.cpp
  src/lock.cpp
  src/merge.cpp
  src/doctor.cpp
  src/diff.cpp
  src/policy.cpp
  src/verify.cpp
  src/storage_rocksdb.cpp
  src/otel.cpp
)
target_include_directories(acxlib PUBLIC src)

add_executable(acx src/cli.cpp)
target_link_libraries(acx PRIVATE acxlib)

enable_testing()
add_executable(selftest tests/selftest.cpp)
target_link_libraries(selftest PRIVATE acxlib)
add_test(NAME selftest COMMAND selftest)


if(ACX_WITH_SQLITE)
  find_package(SQLite3)
  if(SQLite3_FOUND)
    add_compile_definitions(ACX_HAVE_SQLITE3)
    message(STATUS "SQLite3 found: enabling SQLite engine")
  else()
    message(STATUS "SQLite3 not found: SQLite engine will be disabled")
  endif()
endif()

if(ACX_WITH_ZSTD)
  find_package(ZSTD)
  if(ZSTD_FOUND)
    add_compile_definitions(ACX_HAVE_ZSTD)
    message(STATUS "ZSTD found: enabling compression in backups")
  else()
    message(STATUS "ZSTD not found: compression disabled")
  endif()
endif()

if(SQLite3_FOUND)
  target_link_libraries(acxlib PRIVATE SQLite::SQLite3)
endif()
if(ZSTD_FOUND)
  target_link_libraries(acxlib PRIVATE ZSTD::ZSTD)
endif()

# Install + CPack
install(TARGETS acx RUNTIME DESTINATION bin)
install(FILES docs/man/acx.1 DESTINATION share/man/man1)

set(CPACK_PACKAGE_NAME "acx")
set(CPACK_PACKAGE_VERSION "5.0.0")
\1TGZ;ZIP;DEB;RPM\3
include(CPack)

# Reproducible build flags (best-effort)
option(ACX_REPRODUCIBLE "Enable reproducible build flags" ON)
if(ACX_REPRODUCIBLE)
  add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
  add_link_options(-Wl,--build-id=0x0000000000000000000000000000000000000000)
endif()



if(ACX_WITH_COVERAGE)
  message(STATUS "Coverage enabled")
  add_compile_options(-O0 -fprofile-instr-generate -fcoverage-mapping)
  add_link_options(-fprofile-instr-generate -fcoverage-mapping)
endif()

if(ACX_WITH_FUZZ)
  message(STATUS "Building fuzzers")
  add_executable(csv_fuzz fuzz/csv_fuzz.cpp)
  target_link_libraries(csv_fuzz PRIVATE acxlib)
  target_compile_options(csv_fuzz PRIVATE -fsanitize=address,fuzzer-no-link -fno-omit-frame-pointer)
  target_link_options(csv_fuzz PRIVATE -fsanitize=fuzzer,address -fno-omit-frame-pointer)
endif()

if(ACX_WITH_ROCKSDB)
  find_package(rocksdb)
  if(rocksdb_FOUND)
    add_compile_definitions(ACX_HAVE_ROCKSDB)
    message(STATUS "RocksDB found (adapter not yet implemented)")
  else()
    message(STATUS "RocksDB not found")
  endif()
endif()


option(ACX_HARDENED "Enable hardened compile/link flags" ON)
if(ACX_HARDENED)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=2 -D_GLIBCXX_ASSERTIONS -fPIC -fPIE -fno-omit-frame-pointer)
    add_link_options(-Wl,-z,relro -Wl,-z,now -pie)
  endif()
endif()

include(CMakePackageConfigHelpers)
install(TARGETS acx acxlib EXPORT ACXTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
# Install public headers (current headers live in src/*.hpp)
install(DIRECTORY src/ DESTINATION include/acx FILES_MATCHING PATTERN "*.hpp")

# Generate and install package config
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ACXConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/ACXConfig.cmake
  INSTALL_DESTINATION lib/cmake/ACX
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/ACXConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ACXConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/ACXConfigVersion.cmake
  DESTINATION lib/cmake/ACX
)
install(EXPORT ACXTargets
  FILE ACXTargets.cmake
  NAMESPACE ACX::
  DESTINATION lib/cmake/ACX
)

add_executable(acx-bench src/bench.cpp)
add_executable(test_validation tests/test_validation.cpp)
target_include_directories(test_validation PRIVATE .)
add_test(NAME test_validation COMMAND test_validation)

add_executable(test_hmac tests/test_hmac.cpp)
target_include_directories(test_hmac PRIVATE .)
add_test(NAME test_hmac COMMAND test_hmac)

add_executable(test_roundtrip tests/test_roundtrip.cpp)
target_include_directories(test_roundtrip PRIVATE .)
add_test(NAME test_roundtrip COMMAND test_roundtrip)

# Optional OpenSSL for crypto
option(ACX_WITH_OPENSSL "Use system OpenSSL for HMAC/SHA256" ON)
if(ACX_WITH_OPENSSL)
  find_package(OpenSSL)
  if(OPENSSL_FOUND)
    message(STATUS "OpenSSL found: using OpenSSL for crypto")
    target_link_libraries(acxlib PRIVATE OpenSSL::Crypto)
    target_compile_definitions(acxlib PRIVATE ACX_HAVE_OPENSSL=1)
  else()
    message(WARNING "OpenSSL not found. Falling back to internal SHA-256")
  endif()
endif()

# Packaging metadata (deb/rpm)
set(CPACK_PACKAGE_VENDOR "ACX Contributors")
set(CPACK_PACKAGE_CONTACT "security@acx.example.org")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ACX Maintainers")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")

install(DIRECTORY completions/ DESTINATION share/acx/completions FILES_MATCHING PATTERN "acx.*")
install(FILES src/config.hpp src/policy.hpp src/redact.hpp DESTINATION include/acx)

install(FILES src/log_level.hpp DESTINATION include/acx)


# Project version and build metadata
set(ACX_VERSION "2.0.0")
execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE ACX_GIT_SHA
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET)
string(TIMESTAMP ACX_BUILD_TIME_UTC "%Y-%m-%dT%H:%M:%SZ" UTC)
add_definitions(-DACX_VERSION=\"${ACX_VERSION}\" -DACX_BUILD_TIME=\"${ACX_BUILD_TIME_UTC}\" -DACX_GIT_SHA=\"${ACX_GIT_SHA}\")

# Ensure version.cpp is compiled
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp)
  # If target_sources isn't used elsewhere, append via set_property
  set_property(TARGET acxlib APPEND PROPERTY SOURCES src/version.cpp)
endif()

# Install version header
install(FILES src/version.hpp DESTINATION include/acx)

option(ACX_WITH_LIBPHONENUMBER "Use libphonenumber for E.164 validation" OFF)
if(ACX_WITH_LIBPHONENUMBER)
  find_package(absl QUIET)
  find_path(PHONEINC phonenumbers/phonenumberutil.h)
  find_library(PHONELIB libphonenumber NAMES phonenumber phonenumberlite)
  if(PHONEINC AND PHONELIB)
    message(STATUS "libphonenumber found")
    add_definitions(-DACX_HAVE_LIBPHONENUMBER=1)
    include_directories(${PHONEINC})
    set(ACX_PHONELIB ${PHONELIB})
  else()
    message(WARNING "libphonenumber not found; falling back to internal normalizer")
  endif()
endif()
if(ACX_PHONELIB)
  target_link_libraries(acxlib PRIVATE ${ACX_PHONELIB})
endif()

if(ACX_WITH_FUZZ)
  add_executable(fuzz_jsonl fuzz/jsonl_fuzz.cpp)
  target_link_libraries(fuzz_jsonl PRIVATE acxlib)
  add_executable(fuzz_csv fuzz/csv_fuzz.cpp)
  target_link_libraries(fuzz_csv PRIVATE acxlib)
endif()
install(FILES man/acx.1 DESTINATION share/man/man1)

add_executable(test_phone tests/test_phone.cpp)
target_include_directories(test_phone PRIVATE .)
add_test(NAME test_phone COMMAND test_phone)

add_executable(test_policy tests/test_policy.cpp)
target_include_directories(test_policy PRIVATE .)
add_test(NAME test_policy COMMAND test_policy)

option(ACX_WERROR "Treat warnings as errors" ON)
if (ACX_WERROR AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Werror)
endif()
option(ACX_LTO "Enable Link-Time Optimization" OFF)
if (ACX_LTO AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
  if (ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  else()
    message(WARNING "IPO/LTO not supported: ${ipo_msg}")
  endif()
endif()
option(ACX_PGO "Enable simple PGO (profile-generate/use toggles)" OFF)
if (ACX_PGO AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if (NOT DEFINED ACX_PGO_PHASE)
    set(ACX_PGO_PHASE "generate" CACHE STRING "PGO phase: generate|use")
  endif()
  if (ACX_PGO_PHASE STREQUAL "generate")
    add_compile_options(-fprofile-generate)
    add_link_options(-fprofile-generate)
  elseif (ACX_PGO_PHASE STREQUAL "use")
    add_compile_options(-fprofile-use -fprofile-correction)
    add_link_options(-fprofile-use -fprofile-correction)
  endif()
endif()

# EVEREST: include crypto_file and diff sources
set_property(TARGET acxlib APPEND PROPERTY SOURCES src/crypto_file.cpp src/diff.cpp)
install(FILES src/crypto_file.hpp src/diff.hpp DESTINATION include/acx)


# Install fish completion
include(GNUInstallDirs)
install(FILES completions/acx.fish DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/fish/vendor_completions.d)

# Added: script-based CLI smoke tests
add_test(NAME cli_version COMMAND ${CMAKE_BINARY_DIR}/acx --version)
add_test(NAME cli_help COMMAND ${CMAKE_BINARY_DIR}/acx --help)


# --- Generated version header ---
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/acx/version.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/generated/acx/version.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/acx/version.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/acx)



# ---- Size/Static build options ----
option(ACX_STATIC "Link statically on supported platforms" OFF)
option(ACX_TINY "Optimize for minimum size: disable RTTI/exceptions where safe" OFF)

if (ACX_TINY)
  add_compile_options(-Os -fno-rtti -fno-exceptions)
endif()

if (ACX_STATIC AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_link_options(-static)
endif()


# ---- Build info header ----
find_package(Git QUIET)
if (GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty
                  OUTPUT_VARIABLE GIT_DESC OUTPUT_STRIP_TRAILING_WHITESPACE
                  ERROR_QUIET)
endif()
set(ACX_GIT_DESC "${GIT_DESC}")
set(ACX_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
set(ACX_COMPILER_VER "${CMAKE_CXX_COMPILER_VERSION}")
set(ACX_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/acx/buildinfo.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/generated/acx/buildinfo.hpp @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/acx/buildinfo.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/acx)


# Install PowerShell completion script
include(GNUInstallDirs)
install(FILES completions/powershell/acx.ps1
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/acx/powershell)


add_executable(test_config_env tests/test_config_env.cpp)
target_link_libraries(test_config_env PRIVATE acxlib)
add_test(NAME test_config_env COMMAND test_config_env)

add_executable(test_config_loader tests/test_config_loader.cpp)
target_link_libraries(test_config_loader PRIVATE acxlib)
add_test(NAME test_config_loader COMMAND test_config_loader)


# --- MSVC hardening flags ---
if (MSVC)
  add_compile_options(/guard:cf /Qspectre /sdl /W4)
  add_link_options(/CETCOMPAT /DYNAMICBASE /NXCOMPAT)
endif()


# ---- Compiler hardening & pedantic options ----
option(ACX_PEDANTIC "Enable -Wall -Wextra -Wpedantic etc." ON)
option(ACX_WERROR "Treat warnings as errors" OFF)
option(ACX_HARDEN "Enable strong security flags" ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if (ACX_PEDANTIC)
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
  endif()
  if (ACX_WERROR)
    add_compile_options(-Werror)
  endif()
  if (ACX_HARDEN)
    add_compile_options(-fstack-protector-strong -D_FORTIFY_SOURCE=3 -fPIC)
    add_link_options(-Wl,-z,relro -Wl,-z,now)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
      add_link_options(-pie)
    endif()
  endif()
endif()


# ---- Sanitizers / LTO / Coverage ----
option(ACX_ASAN "Enable AddressSanitizer" OFF)
option(ACX_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ACX_TSAN "Enable ThreadSanitizer" OFF)
option(ACX_LTO "Enable Link-Time Optimization" OFF)
option(ACX_COVERAGE "Enable GCC/Clang coverage flags" OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if (ACX_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
  endif()
  if (ACX_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
  endif()
  if (ACX_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
  endif()
  if (ACX_LTO)
    add_link_options(-flto)
  endif()
  if (ACX_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
  endif()
endif()


# ---- Reproducible builds support ----
if(DEFINED ENV{SOURCE_DATE_EPOCH})
  message(STATUS "Using SOURCE_DATE_EPOCH=$ENV{SOURCE_DATE_EPOCH}")
  add_compile_definitions(ACX_SOURCE_DATE_EPOCH=$ENV{SOURCE_DATE_EPOCH})
endif()
