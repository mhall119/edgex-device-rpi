find_path (LIBCSDK_INCLUDE_DIR edgex/edgex.h edgex/devsdk.h)
find_library (LIBCSDK_LIBRARIES NAMES csdk libcsdk)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LIBCSDK DEFAULT_MSG LIBCSDK_LIBRARIES LIBCSDK_INCLUDE_DIR)
