@echo off

REM === environment =======
set vc_install_path="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC"

REM === build target =======
set target_architecture=x64


REM === arguments =======
set source_path=%1
set builds_path=%2
set build_type=%3

REM === constants =======
if %build_type% == debug (
   set buildtype_release=0
   set buildtype_internal=1
   set debuglevel_expensive_checks=1
   set performance_spam_level=0
)
if %build_type% == release (
   set buildtype_release=1
   set buildtype_internal=0
   set debuglevel_expensive_checks=0
   set performance_spam_level=0
)

REM === warnings ======
REM this is disabled because potentially unsafe things are totally fine
set disable_potentially_unsafe_methods_warning_flag=/wd4996
REM this is disabled because I cannot set stuff to positive float infinity if this is on
set disable_overflow_in_constant_arithmetic_warning_flag=/wd4756
REM this is disabled because I like to do while(1) loops
set disable_conditional_expression_is_constant_warning_flag=/wd4127
set disable_function_deprecated_warnings=/wd4995
REM this is not disabled for now, turn it on if it gets too annoying
REM (or just turn it on sometimes when cleaning up)
REM set disable_unreferenced_local_variable_warning_flag=/wd4100


REM TODO: remove first few, we want these warnings!
set disabled_warning_flags=^
    %disable_function_deprecated_warnings%^
    %disable_potentially_unsafe_methods_warning_flag%^
    %disable_overflow_in_constant_arithmetic_warning_flag%^
    %disable_conditional_expression_is_constant_warning_flag%
REM    %disable_unreferenced_local_variable_warning_flag%

set generate_intrinsic_functions_flag=/Oi
set disable_optimizations_flag=/Od
set generate_7_0_compatible_debug_info_flag=/Z7

set optimization_flags =^
    %generate_intrinsic_functions_flag%

set debug_flags=^
    %disable_optimizations_flag%^
    %generate_7_0_compatible_debug_info_flag%

set common_compiler_flags=^
    /nologo^
    /MT^
    /Gm-^
    /EHa-^
    /WX^
    /W4^
    %disabled_warning_flags%

set preprocessor_define_prefix=HELLO_D3D11_WINDOW

set common_linker_flags=/incremental:no

set output_switches=^
    /Fo%builds_path%\^
    /Fe%builds_path%\^
    /Fd%builds_path%\
    
set libs=^
    user32.lib^
    d3d11.lib^
    winmm.lib

set buildtype_def=/D%preprocessor_define_prefix%_BUILDTYPE=%buildtype_internal%
set debuglevel_def=/D%preprocessor_define_prefix%_DEBUGLEVEL=%debuglevel_expensive_checks%

set defs=^
    /D%preprocessor_define_prefix%_BUILDTYPE_RELEASE=%buildtype_release%^
    /D%preprocessor_define_prefix%_BUILDTYPE_INTERNAL=%buildtype_internal%^
    /D%preprocessor_define_prefix%_PERFORMANCE_SPAM_LEVEL=%performance_spam_level%^
    /D%preprocessor_define_prefix%_DEBUGLEVEL_EXPENSIVE_CHECKS=%debuglevel_expensive_checks%
    
REM make sure that the output direcotry exists
IF NOT EXIST %builds_path% mkdir %builds_path%

REM set the compiler environment variables for 64 bit builds
call %vc_install_path%\"\vcvarsall.bat" %target_architecture%

if %build_type% == debug (
   set build_type_specific_flags=%debug_flags%
   set platform_debug_def=/DPLATFORM_DEBUG=1
)
if %build_type% == release (
   set build_type_specific_flags=%optimization_flags%
   set platform_debug_def=/DPLATFORM_DEBUG=0
)

REM compile release build!
call cl^
     %common_compiler_flags%^
     %platform_debug_def%^
     %output_switches%^
     %build_type_specific_flags%^
     %libs%^
     %defs%^
     %buildtype_def%^
     %debuglevel_def%^
     %source_path%\demo.cpp^
     /link %common_linker_flags%^
     /OUT:%builds_path%\hello_%build_type%.exe

REM exit early if compile failed
if %ERRORLEVEL% gtr 0 (
    exit /b %ERRORLEVEL%
    )
    
call fxc %fxc_flags% %source_path%\shaders.hlsl /T vs_5_0 /E vertex_shader /Fo %builds_path%\vertex_shader.cso
if %ERRORLEVEL% gtr 0 (exit /b %ERRORLEVEL% )
call fxc %fxc_flags% %source_path%\shaders.hlsl /T ps_5_0 /E pixel_shader /Fo %builds_path%\pixel_shader.cso
if %ERRORLEVEL% gtr 0 (exit /b %ERRORLEVEL% )

call fxc %fxc_flags% %source_path%\shaders.hlsl /T vs_5_0 /E flat_vertex_shader /Fo %builds_path%\flat_vertex_shader.cso
if %ERRORLEVEL% gtr 0 (exit /b %ERRORLEVEL% )
call fxc %fxc_flags% %source_path%\shaders.hlsl /T ps_5_0 /E flat_pixel_shader /Fo %builds_path%\flat_pixel_shader.cso
if %ERRORLEVEL% gtr 0 (exit /b %ERRORLEVEL% )
