# The app_eir extension
PROJECT("EIR Diameter Application" C)

SET(CMAKE_BUILD_TYPE debug)

# Parser files
BISON_FILE(app_eir_conf.y)
FLEX_FILE(app_eir_conf.l)

include_directories(~/project/utils/src)
include_directories(..)

SET_SOURCE_FILES_PROPERTIES(lex.app_eir_conf.c app_eir_conf.tab.c PROPERTIES COMPILE_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}" )

# List of source files
SET( EIR_APP_SRC
	lex.app_eir_conf.c
	app_eir_conf.tab.c
	app_eir.c
	app_eir_server.c
	dict_eir.c
	app_eir_blacklist.cpp
	~/project/utils/src/utils/timemeasurer/timemeasurer.cpp
	~/project/utils/src/utils/log/log.cpp
	~/project/utils/src/utils/stat/stat.cpp
	~/project/utils/src/utils/ipconnector/ipconnector.cpp
	~/project/utils/src/utils/pspacket/pspacket.cpp
)

# Compile as a module
FD_ADD_EXTENSION(app_eir ${EIR_APP_SRC})


####
## INSTALL section ##

INSTALL(TARGETS app_eir
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-s13)
