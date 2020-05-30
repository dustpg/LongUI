##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=LongUI
ConfigurationName      :=Debug
WorkspacePath          :=D:/github/LongUI/CodeLite
ProjectPath            :=D:/github/LongUI/CodeLite
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=dustpg
Date                   :=30/05/2020
CodeLitePath           :="C:/Program Files (x86)/CodeLite"
LinkerName             :=C:/TDM-GCC-64/bin/g++.exe
SharedObjectLinkerName :=C:/TDM-GCC-64/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)_WIN32_WINNT=0x0603 $(PreprocessorSwitch)LUI_NO_ACCESSIBLE $(PreprocessorSwitch)LUI_NO_DXGIDEBUG 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="LongUI.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=C:/TDM-GCC-64/bin/windres.exe
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)../include/ 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)Gdi32 $(LibrarySwitch)dxguid $(LibrarySwitch)d2d1 $(LibrarySwitch)Winmm $(LibrarySwitch)Ole32 $(LibrarySwitch)Dbghelp $(LibrarySwitch)uuid $(LibrarySwitch)Dwrite $(LibrarySwitch)d3d11 $(LibrarySwitch)DXGI $(LibrarySwitch)windowscodecs $(LibrarySwitch)imm32 
ArLibs                 :=  "Gdi32" "dxguid" "d2d1" "Winmm" "Ole32" "Dbghelp" "uuid" "Dwrite" "d3d11" "DXGI" "windowscodecs" "imm32" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/TDM-GCC-64/bin/ar.exe rcu
CXX      := C:/TDM-GCC-64/bin/g++.exe
CC       := C:/TDM-GCC-64/bin/gcc.exe
CXXFLAGS :=  -g -O0 -std=c++14 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -std=c99 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := C:/TDM-GCC-64/bin/as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files (x86)\CodeLite
Objects0=$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_container_pod_hash.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_button.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_box.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_control.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_image.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_label.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_progress.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_radio.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_scale.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_tab.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_container_pod_vector.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_style_ui_win10.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_function.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_manager.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_animation.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_container_ui_string.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_core_ui_color.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(ObjectSuffix) $(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_window.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(ObjectSuffix) $(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(ObjectSuffix) 

Objects1=$(IntermediateDirectory)/up_src_control_ui_list.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_style_ui_style.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_node.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(ObjectSuffix) $(IntermediateDirectory)/up_src_control_ui_tree.cpp$(ObjectSuffix) 



Objects=$(Objects0) $(Objects1) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@$(MakeDirCommand) "$(ConfigurationName)"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "$(ConfigurationName)"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(ObjectSuffix): ../src/control/gapi/ui_textbox_g.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(DependSuffix) -MM ../src/control/gapi/ui_textbox_g.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/gapi/ui_textbox_g.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(PreprocessSuffix): ../src/control/gapi/ui_textbox_g.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_gapi_ui_textbox_g.cpp$(PreprocessSuffix) ../src/control/gapi/ui_textbox_g.cpp

$(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(ObjectSuffix): ../src/cfunciton/ui_namecolor.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(DependSuffix) -MM ../src/cfunciton/ui_namecolor.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/ui_namecolor.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(PreprocessSuffix): ../src/cfunciton/ui_namecolor.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_ui_namecolor.c$(PreprocessSuffix) ../src/cfunciton/ui_namecolor.c

$(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(ObjectSuffix): ../src/cfunciton/ui_util.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(DependSuffix) -MM ../src/cfunciton/ui_util.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/ui_util.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(PreprocessSuffix): ../src/cfunciton/ui_util.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_ui_util.c$(PreprocessSuffix) ../src/cfunciton/ui_util.c

$(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(ObjectSuffix): ../src/cfunciton/view_atof.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(DependSuffix) -MM ../src/cfunciton/view_atof.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/view_atof.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(PreprocessSuffix): ../src/cfunciton/view_atof.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_view_atof.c$(PreprocessSuffix) ../src/cfunciton/view_atof.c

$(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(ObjectSuffix): ../src/cfunciton/view_unicode.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(DependSuffix) -MM ../src/cfunciton/view_unicode.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/view_unicode.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(PreprocessSuffix): ../src/cfunciton/view_unicode.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_view_unicode.c$(PreprocessSuffix) ../src/cfunciton/view_unicode.c

$(IntermediateDirectory)/up_src_container_pod_hash.cpp$(ObjectSuffix): ../src/container/pod_hash.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_container_pod_hash.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_container_pod_hash.cpp$(DependSuffix) -MM ../src/container/pod_hash.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/container/pod_hash.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_container_pod_hash.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_container_pod_hash.cpp$(PreprocessSuffix): ../src/container/pod_hash.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_container_pod_hash.cpp$(PreprocessSuffix) ../src/container/pod_hash.cpp

$(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(ObjectSuffix): ../src/container/pod_sharedarray.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(DependSuffix) -MM ../src/container/pod_sharedarray.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/container/pod_sharedarray.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(PreprocessSuffix): ../src/container/pod_sharedarray.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_container_pod_sharedarray.cpp$(PreprocessSuffix) ../src/container/pod_sharedarray.cpp

$(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(ObjectSuffix): ../src/container/ui_string_view.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(DependSuffix) -MM ../src/container/ui_string_view.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/container/ui_string_view.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(PreprocessSuffix): ../src/container/ui_string_view.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_container_ui_string_view.cpp$(PreprocessSuffix) ../src/container/ui_string_view.cpp

$(IntermediateDirectory)/up_src_control_ui_button.cpp$(ObjectSuffix): ../src/control/ui_button.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_button.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_button.cpp$(DependSuffix) -MM ../src/control/ui_button.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_button.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_button.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_button.cpp$(PreprocessSuffix): ../src/control/ui_button.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_button.cpp$(PreprocessSuffix) ../src/control/ui_button.cpp

$(IntermediateDirectory)/up_src_control_ui_box.cpp$(ObjectSuffix): ../src/control/ui_box.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_box.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_box.cpp$(DependSuffix) -MM ../src/control/ui_box.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_box.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_box.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_box.cpp$(PreprocessSuffix): ../src/control/ui_box.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_box.cpp$(PreprocessSuffix) ../src/control/ui_box.cpp

$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(ObjectSuffix): ../src/control/ui_caption.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(DependSuffix) -MM ../src/control/ui_caption.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_caption.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_caption.cpp$(PreprocessSuffix): ../src/control/ui_caption.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_caption.cpp$(PreprocessSuffix) ../src/control/ui_caption.cpp

$(IntermediateDirectory)/up_src_control_ui_control.cpp$(ObjectSuffix): ../src/control/ui_control.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_control.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_control.cpp$(DependSuffix) -MM ../src/control/ui_control.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_control.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_control.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_control.cpp$(PreprocessSuffix): ../src/control/ui_control.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_control.cpp$(PreprocessSuffix) ../src/control/ui_control.cpp

$(IntermediateDirectory)/up_src_control_ui_image.cpp$(ObjectSuffix): ../src/control/ui_image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_image.cpp$(DependSuffix) -MM ../src/control/ui_image.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_image.cpp$(PreprocessSuffix): ../src/control/ui_image.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_image.cpp$(PreprocessSuffix) ../src/control/ui_image.cpp

$(IntermediateDirectory)/up_src_control_ui_label.cpp$(ObjectSuffix): ../src/control/ui_label.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_label.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_label.cpp$(DependSuffix) -MM ../src/control/ui_label.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_label.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_label.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_label.cpp$(PreprocessSuffix): ../src/control/ui_label.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_label.cpp$(PreprocessSuffix) ../src/control/ui_label.cpp

$(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(ObjectSuffix): ../src/control/ui_menubar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(DependSuffix) -MM ../src/control/ui_menubar.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_menubar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(PreprocessSuffix): ../src/control/ui_menubar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_menubar.cpp$(PreprocessSuffix) ../src/control/ui_menubar.cpp

$(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(ObjectSuffix): ../src/control/ui_menulist.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(DependSuffix) -MM ../src/control/ui_menulist.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_menulist.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(PreprocessSuffix): ../src/control/ui_menulist.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_menulist.cpp$(PreprocessSuffix) ../src/control/ui_menulist.cpp

$(IntermediateDirectory)/up_src_control_ui_progress.cpp$(ObjectSuffix): ../src/control/ui_progress.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_progress.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_progress.cpp$(DependSuffix) -MM ../src/control/ui_progress.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_progress.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_progress.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_progress.cpp$(PreprocessSuffix): ../src/control/ui_progress.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_progress.cpp$(PreprocessSuffix) ../src/control/ui_progress.cpp

$(IntermediateDirectory)/up_src_control_ui_radio.cpp$(ObjectSuffix): ../src/control/ui_radio.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_radio.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_radio.cpp$(DependSuffix) -MM ../src/control/ui_radio.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_radio.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_radio.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_radio.cpp$(PreprocessSuffix): ../src/control/ui_radio.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_radio.cpp$(PreprocessSuffix) ../src/control/ui_radio.cpp

$(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(ObjectSuffix): ../src/cfunciton/ui_easing.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(DependSuffix) -MM ../src/cfunciton/ui_easing.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/ui_easing.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(PreprocessSuffix): ../src/cfunciton/ui_easing.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_ui_easing.c$(PreprocessSuffix) ../src/cfunciton/ui_easing.c

$(IntermediateDirectory)/up_src_control_ui_scale.cpp$(ObjectSuffix): ../src/control/ui_scale.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_scale.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_scale.cpp$(DependSuffix) -MM ../src/control/ui_scale.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_scale.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_scale.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_scale.cpp$(PreprocessSuffix): ../src/control/ui_scale.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_scale.cpp$(PreprocessSuffix) ../src/control/ui_scale.cpp

$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(ObjectSuffix): ../src/control/ui_scrollarea.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(DependSuffix) -MM ../src/control/ui_scrollarea.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_scrollarea.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(PreprocessSuffix): ../src/control/ui_scrollarea.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_scrollarea.cpp$(PreprocessSuffix) ../src/control/ui_scrollarea.cpp

$(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(ObjectSuffix): ../src/control/ui_spacer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(DependSuffix) -MM ../src/control/ui_spacer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_spacer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(PreprocessSuffix): ../src/control/ui_spacer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_spacer.cpp$(PreprocessSuffix) ../src/control/ui_spacer.cpp

$(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(ObjectSuffix): ../src/cfunciton/ui_stringcolor.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(DependSuffix) -MM ../src/cfunciton/ui_stringcolor.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/ui_stringcolor.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(PreprocessSuffix): ../src/cfunciton/ui_stringcolor.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_ui_stringcolor.c$(PreprocessSuffix) ../src/cfunciton/ui_stringcolor.c

$(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(ObjectSuffix): ../src/control/ui_splitter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(DependSuffix) -MM ../src/control/ui_splitter.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_splitter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(PreprocessSuffix): ../src/control/ui_splitter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_splitter.cpp$(PreprocessSuffix) ../src/control/ui_splitter.cpp

$(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(ObjectSuffix): ../src/control/ui_checkbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(DependSuffix) -MM ../src/control/ui_checkbox.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_checkbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(PreprocessSuffix): ../src/control/ui_checkbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_checkbox.cpp$(PreprocessSuffix) ../src/control/ui_checkbox.cpp

$(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(ObjectSuffix): ../src/control/ui_statusbar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(DependSuffix) -MM ../src/control/ui_statusbar.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_statusbar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(PreprocessSuffix): ../src/control/ui_statusbar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_statusbar.cpp$(PreprocessSuffix) ../src/control/ui_statusbar.cpp

$(IntermediateDirectory)/up_src_control_ui_tab.cpp$(ObjectSuffix): ../src/control/ui_tab.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_tab.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_tab.cpp$(DependSuffix) -MM ../src/control/ui_tab.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_tab.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_tab.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_tab.cpp$(PreprocessSuffix): ../src/control/ui_tab.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_tab.cpp$(PreprocessSuffix) ../src/control/ui_tab.cpp

$(IntermediateDirectory)/up_src_control_ui_test.cpp$(ObjectSuffix): ../src/control/ui_test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_test.cpp$(DependSuffix) -MM ../src/control/ui_test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_test.cpp$(PreprocessSuffix): ../src/control/ui_test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_test.cpp$(PreprocessSuffix) ../src/control/ui_test.cpp

$(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(ObjectSuffix): ../src/control/ui_toolbar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(DependSuffix) -MM ../src/control/ui_toolbar.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_toolbar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(PreprocessSuffix): ../src/control/ui_toolbar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_toolbar.cpp$(PreprocessSuffix) ../src/control/ui_toolbar.cpp

$(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(ObjectSuffix): ../src/control/ui_richlist.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(DependSuffix) -MM ../src/control/ui_richlist.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_richlist.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(PreprocessSuffix): ../src/control/ui_richlist.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_richlist.cpp$(PreprocessSuffix) ../src/control/ui_richlist.cpp

$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(ObjectSuffix): ../src/control/ui_tooltip.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(DependSuffix) -MM ../src/control/ui_tooltip.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_tooltip.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(PreprocessSuffix): ../src/control/ui_tooltip.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_tooltip.cpp$(PreprocessSuffix) ../src/control/ui_tooltip.cpp

$(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(ObjectSuffix): ../src/style/ui_ssvalue.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(DependSuffix) -MM ../src/style/ui_ssvalue.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_ssvalue.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(PreprocessSuffix): ../src/style/ui_ssvalue.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_ssvalue.cpp$(PreprocessSuffix) ../src/style/ui_ssvalue.cpp

$(IntermediateDirectory)/up_src_container_pod_vector.cpp$(ObjectSuffix): ../src/container/pod_vector.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_container_pod_vector.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_container_pod_vector.cpp$(DependSuffix) -MM ../src/container/pod_vector.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/container/pod_vector.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_container_pod_vector.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_container_pod_vector.cpp$(PreprocessSuffix): ../src/container/pod_vector.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_container_pod_vector.cpp$(PreprocessSuffix) ../src/container/pod_vector.cpp

$(IntermediateDirectory)/up_src_style_ui_win10.cpp$(ObjectSuffix): ../src/style/ui_win10.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_win10.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_win10.cpp$(DependSuffix) -MM ../src/style/ui_win10.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_win10.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_win10.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_win10.cpp$(PreprocessSuffix): ../src/style/ui_win10.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_win10.cpp$(PreprocessSuffix) ../src/style/ui_win10.cpp

$(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(ObjectSuffix): ../src/effect/ui_effect_backimage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(DependSuffix) -MM ../src/effect/ui_effect_backimage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/effect/ui_effect_backimage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(PreprocessSuffix): ../src/effect/ui_effect_backimage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_effect_ui_effect_backimage.cpp$(PreprocessSuffix) ../src/effect/ui_effect_backimage.cpp

$(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(ObjectSuffix): ../src/style/ui_style_value.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(DependSuffix) -MM ../src/style/ui_style_value.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_style_value.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(PreprocessSuffix): ../src/style/ui_style_value.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_style_value.cpp$(PreprocessSuffix) ../src/style/ui_style_value.cpp

$(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(ObjectSuffix): ../src/text/ui_text_layout.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(DependSuffix) -MM ../src/text/ui_text_layout.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/text/ui_text_layout.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(PreprocessSuffix): ../src/text/ui_text_layout.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_text_ui_text_layout.cpp$(PreprocessSuffix) ../src/text/ui_text_layout.cpp

$(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(ObjectSuffix): ../src/core/ui_time_capsule.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(DependSuffix) -MM ../src/core/ui_time_capsule.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_time_capsule.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(PreprocessSuffix): ../src/core/ui_time_capsule.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_time_capsule.cpp$(PreprocessSuffix) ../src/core/ui_time_capsule.cpp

$(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(ObjectSuffix): ../src/text/ui_text_outline.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(DependSuffix) -MM ../src/text/ui_text_outline.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/text/ui_text_outline.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(PreprocessSuffix): ../src/text/ui_text_outline.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_text_ui_text_outline.cpp$(PreprocessSuffix) ../src/text/ui_text_outline.cpp

$(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(ObjectSuffix): ../src/xul/SimpAX.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(DependSuffix) -MM ../src/xul/SimpAX.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/xul/SimpAX.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(PreprocessSuffix): ../src/xul/SimpAX.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_xul_SimpAX.cpp$(PreprocessSuffix) ../src/xul/SimpAX.cpp

$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(ObjectSuffix): ../src/filesystem/ui_file.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(DependSuffix) -MM ../src/filesystem/ui_file.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/filesystem/ui_file.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(PreprocessSuffix): ../src/filesystem/ui_file.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_filesystem_ui_file.cpp$(PreprocessSuffix) ../src/filesystem/ui_file.cpp

$(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(ObjectSuffix): ../src/util/ui_named_control.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(DependSuffix) -MM ../src/util/ui_named_control.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_named_control.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(PreprocessSuffix): ../src/util/ui_named_control.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_named_control.cpp$(PreprocessSuffix) ../src/util/ui_named_control.cpp

$(IntermediateDirectory)/up_src_util_ui_function.cpp$(ObjectSuffix): ../src/util/ui_function.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_function.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_function.cpp$(DependSuffix) -MM ../src/util/ui_function.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_function.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_function.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_function.cpp$(PreprocessSuffix): ../src/util/ui_function.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_function.cpp$(PreprocessSuffix) ../src/util/ui_function.cpp

$(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(ObjectSuffix): ../src/xul/SimpAC.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(DependSuffix) -MM ../src/xul/SimpAC.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/xul/SimpAC.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(PreprocessSuffix): ../src/xul/SimpAC.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_xul_SimpAC.cpp$(PreprocessSuffix) ../src/xul/SimpAC.cpp

$(IntermediateDirectory)/up_src_core_ui_manager.cpp$(ObjectSuffix): ../src/core/ui_manager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_manager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_manager.cpp$(DependSuffix) -MM ../src/core/ui_manager.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_manager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_manager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_manager.cpp$(PreprocessSuffix): ../src/core/ui_manager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_manager.cpp$(PreprocessSuffix) ../src/core/ui_manager.cpp

$(IntermediateDirectory)/up_src_util_ui_animation.cpp$(ObjectSuffix): ../src/util/ui_animation.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_animation.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_animation.cpp$(DependSuffix) -MM ../src/util/ui_animation.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_animation.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_animation.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_animation.cpp$(PreprocessSuffix): ../src/util/ui_animation.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_animation.cpp$(PreprocessSuffix) ../src/util/ui_animation.cpp

$(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(ObjectSuffix): ../src/control/gapi/ui_control_g.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(DependSuffix) -MM ../src/control/gapi/ui_control_g.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/gapi/ui_control_g.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(PreprocessSuffix): ../src/control/gapi/ui_control_g.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_gapi_ui_control_g.cpp$(PreprocessSuffix) ../src/control/gapi/ui_control_g.cpp

$(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(ObjectSuffix): ../src/thread/ui_rwlocker.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(DependSuffix) -MM ../src/thread/ui_rwlocker.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/thread/ui_rwlocker.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(PreprocessSuffix): ../src/thread/ui_rwlocker.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_thread_ui_rwlocker.cpp$(PreprocessSuffix) ../src/thread/ui_rwlocker.cpp

$(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(ObjectSuffix): ../src/core/ui_basic_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(DependSuffix) -MM ../src/core/ui_basic_type.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_basic_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(PreprocessSuffix): ../src/core/ui_basic_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_basic_type.cpp$(PreprocessSuffix) ../src/core/ui_basic_type.cpp

$(IntermediateDirectory)/up_src_container_ui_string.cpp$(ObjectSuffix): ../src/container/ui_string.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_container_ui_string.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_container_ui_string.cpp$(DependSuffix) -MM ../src/container/ui_string.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/container/ui_string.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_container_ui_string.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_container_ui_string.cpp$(PreprocessSuffix): ../src/container/ui_string.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_container_ui_string.cpp$(PreprocessSuffix) ../src/container/ui_string.cpp

$(IntermediateDirectory)/up_src_core_ui_color.cpp$(ObjectSuffix): ../src/core/ui_color.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_color.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_color.cpp$(DependSuffix) -MM ../src/core/ui_color.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_color.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_color.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_color.cpp$(PreprocessSuffix): ../src/core/ui_color.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_color.cpp$(PreprocessSuffix) ../src/core/ui_color.cpp

$(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(ObjectSuffix): ../src/graphics/ui_cursor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(DependSuffix) -MM ../src/graphics/ui_cursor.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_cursor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(PreprocessSuffix): ../src/graphics/ui_cursor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_cursor.cpp$(PreprocessSuffix) ../src/graphics/ui_cursor.cpp

$(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(ObjectSuffix): ../src/cfunciton/ui_colorsystem.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(DependSuffix) -MM ../src/cfunciton/ui_colorsystem.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/ui_colorsystem.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(PreprocessSuffix): ../src/cfunciton/ui_colorsystem.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_ui_colorsystem.c$(PreprocessSuffix) ../src/cfunciton/ui_colorsystem.c

$(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(ObjectSuffix): ../RichED/ed_txtdoc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(DependSuffix) -MM ../RichED/ed_txtdoc.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/RichED/ed_txtdoc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(PreprocessSuffix): ../RichED/ed_txtdoc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_RichED_ed_txtdoc.cpp$(PreprocessSuffix) ../RichED/ed_txtdoc.cpp

$(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(ObjectSuffix): ../src/resource/ui_res_image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(DependSuffix) -MM ../src/resource/ui_res_image.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/resource/ui_res_image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(PreprocessSuffix): ../src/resource/ui_res_image.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_resource_ui_res_image.cpp$(PreprocessSuffix) ../src/resource/ui_res_image.cpp

$(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(ObjectSuffix): ../src/util/ui_unicode.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(DependSuffix) -MM ../src/util/ui_unicode.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_unicode.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(PreprocessSuffix): ../src/util/ui_unicode.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_unicode.cpp$(PreprocessSuffix) ../src/util/ui_unicode.cpp

$(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(ObjectSuffix): ../src/graphics/ui_matrix.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(DependSuffix) -MM ../src/graphics/ui_matrix.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_matrix.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(PreprocessSuffix): ../src/graphics/ui_matrix.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_matrix.cpp$(PreprocessSuffix) ../src/graphics/ui_matrix.cpp

$(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(ObjectSuffix): ../src/control/ui_stackdeck.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(DependSuffix) -MM ../src/control/ui_stackdeck.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_stackdeck.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(PreprocessSuffix): ../src/control/ui_stackdeck.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_stackdeck.cpp$(PreprocessSuffix) ../src/control/ui_stackdeck.cpp

$(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(ObjectSuffix): ../src/control/ui_viewport.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(DependSuffix) -MM ../src/control/ui_viewport.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_viewport.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(PreprocessSuffix): ../src/control/ui_viewport.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_viewport.cpp$(PreprocessSuffix) ../src/control/ui_viewport.cpp

$(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(ObjectSuffix): ../src/event/ui_event_host.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(DependSuffix) -MM ../src/event/ui_event_host.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/event/ui_event_host.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(PreprocessSuffix): ../src/event/ui_event_host.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_event_ui_event_host.cpp$(PreprocessSuffix) ../src/event/ui_event_host.cpp

$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(ObjectSuffix): ../RichED/ed_txtcell.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(DependSuffix) -MM ../RichED/ed_txtcell.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/RichED/ed_txtcell.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(PreprocessSuffix): ../RichED/ed_txtcell.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_RichED_ed_txtcell.cpp$(PreprocessSuffix) ../RichED/ed_txtcell.cpp

$(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(ObjectSuffix): ../src/thread/ui_fiber.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(DependSuffix) -MM ../src/thread/ui_fiber.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/thread/ui_fiber.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(PreprocessSuffix): ../src/thread/ui_fiber.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_thread_ui_fiber.cpp$(PreprocessSuffix) ../src/thread/ui_fiber.cpp

$(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(ObjectSuffix): ../src/graphics/ui_bd_renderer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(DependSuffix) -MM ../src/graphics/ui_bd_renderer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_bd_renderer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(PreprocessSuffix): ../src/graphics/ui_bd_renderer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_bd_renderer.cpp$(PreprocessSuffix) ../src/graphics/ui_bd_renderer.cpp

$(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(ObjectSuffix): ../src/control/ui_groupbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(DependSuffix) -MM ../src/control/ui_groupbox.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_groupbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(PreprocessSuffix): ../src/control/ui_groupbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_groupbox.cpp$(PreprocessSuffix) ../src/control/ui_groupbox.cpp

$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM main.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/CodeLite/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) main.cpp

$(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(ObjectSuffix): ../src/interface/ui_default_config.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(DependSuffix) -MM ../src/interface/ui_default_config.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/interface/ui_default_config.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(PreprocessSuffix): ../src/interface/ui_default_config.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_interface_ui_default_config.cpp$(PreprocessSuffix) ../src/interface/ui_default_config.cpp

$(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(ObjectSuffix): ../src/util/ui_time_clipboard.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(DependSuffix) -MM ../src/util/ui_time_clipboard.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_time_clipboard.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(PreprocessSuffix): ../src/util/ui_time_clipboard.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_time_clipboard.cpp$(PreprocessSuffix) ../src/util/ui_time_clipboard.cpp

$(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(ObjectSuffix): ../src/core/ui_window_manager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(DependSuffix) -MM ../src/core/ui_window_manager.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_window_manager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(PreprocessSuffix): ../src/core/ui_window_manager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_window_manager.cpp$(PreprocessSuffix) ../src/core/ui_window_manager.cpp

$(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(ObjectSuffix): ../RichED/ed_txtbuf.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(DependSuffix) -MM ../RichED/ed_txtbuf.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/RichED/ed_txtbuf.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(PreprocessSuffix): ../RichED/ed_txtbuf.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_RichED_ed_txtbuf.cpp$(PreprocessSuffix) ../RichED/ed_txtbuf.cpp

$(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(ObjectSuffix): ../src/thread/ui_waiter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(DependSuffix) -MM ../src/thread/ui_waiter.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/thread/ui_waiter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(PreprocessSuffix): ../src/thread/ui_waiter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_thread_ui_waiter.cpp$(PreprocessSuffix) ../src/thread/ui_waiter.cpp

$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(ObjectSuffix): ../src/private/ui_private_control.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(DependSuffix) -MM ../src/private/ui_private_control.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/private/ui_private_control.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(PreprocessSuffix): ../src/private/ui_private_control.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_private_ui_private_control.cpp$(PreprocessSuffix) ../src/private/ui_private_control.cpp

$(IntermediateDirectory)/up_src_core_ui_window.cpp$(ObjectSuffix): ../src/core/ui_window.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_window.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_window.cpp$(DependSuffix) -MM ../src/core/ui_window.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_window.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_window.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_window.cpp$(PreprocessSuffix): ../src/core/ui_window.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_window.cpp$(PreprocessSuffix) ../src/core/ui_window.cpp

$(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(ObjectSuffix): ../src/control/ui_scrollbar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(DependSuffix) -MM ../src/control/ui_scrollbar.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_scrollbar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(PreprocessSuffix): ../src/control/ui_scrollbar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_scrollbar.cpp$(PreprocessSuffix) ../src/control/ui_scrollbar.cpp

$(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(ObjectSuffix): ../src/graphics/ui_dcomp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(DependSuffix) -MM ../src/graphics/ui_dcomp.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_dcomp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(PreprocessSuffix): ../src/graphics/ui_dcomp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_dcomp.cpp$(PreprocessSuffix) ../src/graphics/ui_dcomp.cpp

$(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(ObjectSuffix): ../src/graphics/ui_graphics_util.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(DependSuffix) -MM ../src/graphics/ui_graphics_util.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_graphics_util.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(PreprocessSuffix): ../src/graphics/ui_graphics_util.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_graphics_util.cpp$(PreprocessSuffix) ../src/graphics/ui_graphics_util.cpp

$(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(ObjectSuffix): ../src/resource/ui_resource_id.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(DependSuffix) -MM ../src/resource/ui_resource_id.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/resource/ui_resource_id.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(PreprocessSuffix): ../src/resource/ui_resource_id.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_resource_ui_resource_id.cpp$(PreprocessSuffix) ../src/resource/ui_resource_id.cpp

$(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(ObjectSuffix): ../src/resource/ui_bitmap_bank.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(DependSuffix) -MM ../src/resource/ui_bitmap_bank.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/resource/ui_bitmap_bank.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(PreprocessSuffix): ../src/resource/ui_bitmap_bank.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_resource_ui_bitmap_bank.cpp$(PreprocessSuffix) ../src/resource/ui_bitmap_bank.cpp

$(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(ObjectSuffix): ../src/input/ui_kminput.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(DependSuffix) -MM ../src/input/ui_kminput.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/input/ui_kminput.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(PreprocessSuffix): ../src/input/ui_kminput.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_input_ui_kminput.cpp$(PreprocessSuffix) ../src/input/ui_kminput.cpp

$(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(ObjectSuffix): ../src/graphics/ui_mesh.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(DependSuffix) -MM ../src/graphics/ui_mesh.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_mesh.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(PreprocessSuffix): ../src/graphics/ui_mesh.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_mesh.cpp$(PreprocessSuffix) ../src/graphics/ui_mesh.cpp

$(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(ObjectSuffix): ../src/util/ui_double_click.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(DependSuffix) -MM ../src/util/ui_double_click.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_double_click.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(PreprocessSuffix): ../src/util/ui_double_click.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_double_click.cpp$(PreprocessSuffix) ../src/util/ui_double_click.cpp

$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(ObjectSuffix): ../src/graphics/ui_bg_renderer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(DependSuffix) -MM ../src/graphics/ui_bg_renderer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_bg_renderer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(PreprocessSuffix): ../src/graphics/ui_bg_renderer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_bg_renderer.cpp$(PreprocessSuffix) ../src/graphics/ui_bg_renderer.cpp

$(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(ObjectSuffix): ../src/core/ui_control_state.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(DependSuffix) -MM ../src/core/ui_control_state.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_control_state.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(PreprocessSuffix): ../src/core/ui_control_state.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_control_state.cpp$(PreprocessSuffix) ../src/core/ui_control_state.cpp

$(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(ObjectSuffix): ../src/util/ui_color_sysem.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(DependSuffix) -MM ../src/util/ui_color_sysem.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/util/ui_color_sysem.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(PreprocessSuffix): ../src/util/ui_color_sysem.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_util_ui_color_sysem.cpp$(PreprocessSuffix) ../src/util/ui_color_sysem.cpp

$(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(ObjectSuffix): ../src/accessible/ui_accessible_win.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(DependSuffix) -MM ../src/accessible/ui_accessible_win.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/accessible/ui_accessible_win.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(PreprocessSuffix): ../src/accessible/ui_accessible_win.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_accessible_ui_accessible_win.cpp$(PreprocessSuffix) ../src/accessible/ui_accessible_win.cpp

$(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(ObjectSuffix): ../src/filesystem/ui_findfile.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(DependSuffix) -MM ../src/filesystem/ui_findfile.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/filesystem/ui_findfile.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(PreprocessSuffix): ../src/filesystem/ui_findfile.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_filesystem_ui_findfile.cpp$(PreprocessSuffix) ../src/filesystem/ui_findfile.cpp

$(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(ObjectSuffix): ../src/cfunciton/view_atoi.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(DependSuffix) -MM ../src/cfunciton/view_atoi.c
	$(CC) $(SourceSwitch) "D:/github/LongUI/src/cfunciton/view_atoi.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(PreprocessSuffix): ../src/cfunciton/view_atoi.c
	$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_cfunciton_view_atoi.c$(PreprocessSuffix) ../src/cfunciton/view_atoi.c

$(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(ObjectSuffix): ../src/filesystem/ui_fileop.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(DependSuffix) -MM ../src/filesystem/ui_fileop.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/filesystem/ui_fileop.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(PreprocessSuffix): ../src/filesystem/ui_fileop.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_filesystem_ui_fileop.cpp$(PreprocessSuffix) ../src/filesystem/ui_fileop.cpp

$(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(ObjectSuffix): ../src/control/gapi/ui_label_g.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(DependSuffix) -MM ../src/control/gapi/ui_label_g.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/gapi/ui_label_g.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(PreprocessSuffix): ../src/control/gapi/ui_label_g.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_gapi_ui_label_g.cpp$(PreprocessSuffix) ../src/control/gapi/ui_label_g.cpp

$(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(ObjectSuffix): ../src/effect/ui_shader_borderimage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(DependSuffix) -MM ../src/effect/ui_shader_borderimage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/effect/ui_shader_borderimage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(PreprocessSuffix): ../src/effect/ui_shader_borderimage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_effect_ui_shader_borderimage.cpp$(PreprocessSuffix) ../src/effect/ui_shader_borderimage.cpp

$(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(ObjectSuffix): ../src/private/ui_private_effect.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(DependSuffix) -MM ../src/private/ui_private_effect.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/private/ui_private_effect.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(PreprocessSuffix): ../src/private/ui_private_effect.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_private_ui_private_effect.cpp$(PreprocessSuffix) ../src/private/ui_private_effect.cpp

$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(ObjectSuffix): ../src/graphics/ui_geometry.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(DependSuffix) -MM ../src/graphics/ui_geometry.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/graphics/ui_geometry.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(PreprocessSuffix): ../src/graphics/ui_geometry.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_graphics_ui_geometry.cpp$(PreprocessSuffix) ../src/graphics/ui_geometry.cpp

$(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(ObjectSuffix): ../src/debugger/ui_debug_window.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(DependSuffix) -MM ../src/debugger/ui_debug_window.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/debugger/ui_debug_window.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(PreprocessSuffix): ../src/debugger/ui_debug_window.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_debugger_ui_debug_window.cpp$(PreprocessSuffix) ../src/debugger/ui_debug_window.cpp

$(IntermediateDirectory)/up_src_control_ui_list.cpp$(ObjectSuffix): ../src/control/ui_list.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_list.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_list.cpp$(DependSuffix) -MM ../src/control/ui_list.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_list.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_list.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_list.cpp$(PreprocessSuffix): ../src/control/ui_list.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_list.cpp$(PreprocessSuffix) ../src/control/ui_list.cpp

$(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(ObjectSuffix): ../src/effect/ui_shader_backimage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(DependSuffix) -MM ../src/effect/ui_shader_backimage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/effect/ui_shader_backimage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(PreprocessSuffix): ../src/effect/ui_shader_backimage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_effect_ui_shader_backimage.cpp$(PreprocessSuffix) ../src/effect/ui_shader_backimage.cpp

$(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(ObjectSuffix): ../src/effect/ui_effect_borderimage.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(DependSuffix) -MM ../src/effect/ui_effect_borderimage.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/effect/ui_effect_borderimage.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(PreprocessSuffix): ../src/effect/ui_effect_borderimage.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_effect_ui_effect_borderimage.cpp$(PreprocessSuffix) ../src/effect/ui_effect_borderimage.cpp

$(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(ObjectSuffix): ../src/core/ui_resource_manager.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(DependSuffix) -MM ../src/core/ui_resource_manager.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_resource_manager.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(PreprocessSuffix): ../src/core/ui_resource_manager.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_resource_manager.cpp$(PreprocessSuffix) ../src/core/ui_resource_manager.cpp

$(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(ObjectSuffix): ../src/debugger/ui_tracer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(DependSuffix) -MM ../src/debugger/ui_tracer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/debugger/ui_tracer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(PreprocessSuffix): ../src/debugger/ui_tracer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_debugger_ui_tracer.cpp$(PreprocessSuffix) ../src/debugger/ui_tracer.cpp

$(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(ObjectSuffix): ../src/control/ui_textbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(DependSuffix) -MM ../src/control/ui_textbox.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_textbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(PreprocessSuffix): ../src/control/ui_textbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_textbox.cpp$(PreprocessSuffix) ../src/control/ui_textbox.cpp

$(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(ObjectSuffix): ../src/debugger/ui_debug.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(DependSuffix) -MM ../src/debugger/ui_debug.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/debugger/ui_debug.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(PreprocessSuffix): ../src/debugger/ui_debug.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_debugger_ui_debug.cpp$(PreprocessSuffix) ../src/debugger/ui_debug.cpp

$(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(ObjectSuffix): ../src/core/ui_popup_window.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(DependSuffix) -MM ../src/core/ui_popup_window.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_popup_window.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(PreprocessSuffix): ../src/core/ui_popup_window.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_popup_window.cpp$(PreprocessSuffix) ../src/core/ui_popup_window.cpp

$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(ObjectSuffix): ../src/style/ui_ssvalue_list.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(DependSuffix) -MM ../src/style/ui_ssvalue_list.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_ssvalue_list.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(PreprocessSuffix): ../src/style/ui_ssvalue_list.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_ssvalue_list.cpp$(PreprocessSuffix) ../src/style/ui_ssvalue_list.cpp

$(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(ObjectSuffix): ../src/core/ui_malloc.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(DependSuffix) -MM ../src/core/ui_malloc.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_malloc.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(PreprocessSuffix): ../src/core/ui_malloc.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_malloc.cpp$(PreprocessSuffix) ../src/core/ui_malloc.cpp

$(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(ObjectSuffix): ../src/core/ui_hidpi.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(DependSuffix) -MM ../src/core/ui_hidpi.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_hidpi.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(PreprocessSuffix): ../src/core/ui_hidpi.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_hidpi.cpp$(PreprocessSuffix) ../src/core/ui_hidpi.cpp

$(IntermediateDirectory)/up_src_style_ui_style.cpp$(ObjectSuffix): ../src/style/ui_style.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_style.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_style.cpp$(DependSuffix) -MM ../src/style/ui_style.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_style.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_style.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_style.cpp$(PreprocessSuffix): ../src/style/ui_style.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_style.cpp$(PreprocessSuffix) ../src/style/ui_style.cpp

$(IntermediateDirectory)/up_src_core_ui_node.cpp$(ObjectSuffix): ../src/core/ui_node.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_node.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_node.cpp$(DependSuffix) -MM ../src/core/ui_node.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_node.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_node.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_node.cpp$(PreprocessSuffix): ../src/core/ui_node.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_node.cpp$(PreprocessSuffix) ../src/core/ui_node.cpp

$(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(ObjectSuffix): ../src/core/ui_control_control.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(DependSuffix) -MM ../src/core/ui_control_control.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_control_control.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(PreprocessSuffix): ../src/core/ui_control_control.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_control_control.cpp$(PreprocessSuffix) ../src/core/ui_control_control.cpp

$(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(ObjectSuffix): ../src/thread/ui_locker.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(DependSuffix) -MM ../src/thread/ui_locker.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/thread/ui_locker.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(PreprocessSuffix): ../src/thread/ui_locker.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_thread_ui_locker.cpp$(PreprocessSuffix) ../src/thread/ui_locker.cpp

$(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(ObjectSuffix): ../src/core/ui_const_sstring.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(DependSuffix) -MM ../src/core/ui_const_sstring.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_const_sstring.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(PreprocessSuffix): ../src/core/ui_const_sstring.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_const_sstring.cpp$(PreprocessSuffix) ../src/core/ui_const_sstring.cpp

$(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(ObjectSuffix): ../src/core/ui_all_control_info.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(DependSuffix) -MM ../src/core/ui_all_control_info.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/core/ui_all_control_info.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(PreprocessSuffix): ../src/core/ui_all_control_info.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_core_ui_all_control_info.cpp$(PreprocessSuffix) ../src/core/ui_all_control_info.cpp

$(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(ObjectSuffix): ../RichED/ed_undoredo.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(DependSuffix) -MM ../RichED/ed_undoredo.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/RichED/ed_undoredo.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(PreprocessSuffix): ../RichED/ed_undoredo.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_RichED_ed_undoredo.cpp$(PreprocessSuffix) ../RichED/ed_undoredo.cpp

$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(ObjectSuffix): ../src/style/ui_native_style.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(DependSuffix) -MM ../src/style/ui_native_style.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/style/ui_native_style.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(PreprocessSuffix): ../src/style/ui_native_style.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_style_ui_native_style.cpp$(PreprocessSuffix) ../src/style/ui_native_style.cpp

$(IntermediateDirectory)/up_src_control_ui_tree.cpp$(ObjectSuffix): ../src/control/ui_tree.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/up_src_control_ui_tree.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/up_src_control_ui_tree.cpp$(DependSuffix) -MM ../src/control/ui_tree.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "D:/github/LongUI/src/control/ui_tree.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/up_src_control_ui_tree.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/up_src_control_ui_tree.cpp$(PreprocessSuffix): ../src/control/ui_tree.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/up_src_control_ui_tree.cpp$(PreprocessSuffix) ../src/control/ui_tree.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


