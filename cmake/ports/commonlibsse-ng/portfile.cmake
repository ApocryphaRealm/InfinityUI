set(VCPKG_POLICY_SKIP_ABSOLUTE_PATHS_CHECK enabled)

file(READ "${CMAKE_CURRENT_LIST_DIR}/version.conf" VERSION_INFO)

string(REGEX MATCH "COMMONLIBSSE_REF=([a-f0-9]+)" _ "${VERSION_INFO}")
set(COMMONLIBSSE_REF "${CMAKE_MATCH_1}")

string(REGEX MATCH "COMMONLIBSSE_SHA512=([a-f0-9]+)" _ "${VERSION_INFO}")
set(COMMONLIBSSE_SHA512 "${CMAKE_MATCH_1}")

vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH
	REPO alandtse/CommonLibVR
	REF ${COMMONLIBSSE_REF}
	SHA512 ${COMMONLIBSSE_SHA512}
	HEAD_REF ng
)

# Download OpenVR submodule
vcpkg_from_github(
	OUT_SOURCE_PATH SOURCE_PATH_OPENVR
	REPO ValveSoftware/openvr
	REF ebdea152f8aac77e9a6db29682b81d762159df7e # 01/09/17
	SHA512 4fb668d933ac5b73eb4e97eb29816176e500a4eaebe2480cd0411c95edfb713d58312036f15db50884a2ef5f4ca44859e108dec2b982af9163cefcfc02531f63
	HEAD_REF master
)
file(COPY "${SOURCE_PATH_OPENVR}/" DESTINATION "${SOURCE_PATH}/extern/openvr")
file(REMOVE_RECURSE "${SOURCE_PATH_OPENVR}/")

vcpkg_configure_cmake(
	SOURCE_PATH "${SOURCE_PATH}"
	PREFER_NINJA
	OPTIONS -DBUILD_TESTS=off -DSKSE_SUPPORT_XBYAK=on
)
vcpkg_install_cmake()

file(GLOB OPENVR_API_LIB "${CURRENT_BUILDTREES_DIR}/src/**/extern/openvr/lib/win64/openvr_api.lib")

file(COPY "${OPENVR_API_LIB}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib/")

vcpkg_cmake_config_fixup(PACKAGE_NAME CommonLibSSE CONFIG_PATH lib/cmake)

file(GLOB_RECURSE TARGETS_FILES "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/*targets.cmake")

foreach(targets_file ${TARGETS_FILES})
    file(READ "${targets_file}" TARGETS_CONTENT)

	string(REPLACE
		"${OPENVR_API_LIB}"
		"\${_IMPORT_PREFIX}/lib/openvr_api.lib"
		TARGETS_CONTENT
		"${TARGETS_CONTENT}"
	)

    file(WRITE "${targets_file}" "${TARGETS_CONTENT}")
endforeach()

vcpkg_copy_pdbs()

file(INSTALL "${SOURCE_PATH}/extern/openvr/headers/openvr.h" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
file(GLOB CMAKE_CONFIGS "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE/*.cmake")
file(INSTALL ${CMAKE_CONFIGS} DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")
file(INSTALL "${SOURCE_PATH}/cmake/CommonLibSSE.cmake" DESTINATION "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/share/CommonLibSSE/CommonLibSSE")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
