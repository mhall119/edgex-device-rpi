find_path (LIBMRAA_INCLUDE_DIR mraa.h)
find_library (LIBMRAA_LIBRARIES NAMES mraa libmraa)
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (LIBMRAA DEFAULT_MSG LIBMRAA_LIBRARIES LIBMRAA_INCLUDE_DIR)
