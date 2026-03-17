# #####################################
# ## D3D12 support
# #####################################

set(EZ_BUILD_EXPERIMENTAL_DX12 OFF CACHE BOOL "Whether to enable experimental / work-in-progress D3D12 code")

# #####################################
# ## ez_requires_dx12()
# #####################################
macro(ez_requires_dx12)
	ez_requires(EZ_CMAKE_PLATFORM_WINDOWS)
	ez_requires(EZ_BUILD_EXPERIMENTAL_DX12)
endmacro()

# #####################################
# ## ez_link_target_dx12(<target>)
# #####################################
function(ez_link_target_dx12 TARGET_NAME)
	ez_requires_dx12()

	target_link_libraries(${TARGET_NAME}
		PRIVATE
		d3d12.lib
		dxgi.lib
		dxguid.lib
	)

    target_compile_definitions(${TARGET_NAME}
        PRIVATE
        BUILDSYSTEM_ENABLE_D3D12_SUPPORT
    )
endfunction()
