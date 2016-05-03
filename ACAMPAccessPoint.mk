##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=ACAMPAccessPoint
ConfigurationName      :=Debug
WorkspacePath          :=/home/ouyc/Documents/Project/ACAMPAccessPoint
ProjectPath            :=/home/ouyc/Documents/Project/ACAMPAccessPoint
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=ouyc
Date                   :=03/05/16
CodeLitePath           :=/home/ouyc/.codelite
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="ACAMPAccessPoint.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) $(IntermediateDirectory)/AcampNetwork.cpp$(ObjectSuffix) $(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) $(IntermediateDirectory)/AcampRegisterState.cpp$(ObjectSuffix) $(IntermediateDirectory)/APMain.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix): AcampAP.cpp $(IntermediateDirectory)/AcampAP.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampAP.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampAP.cpp$(DependSuffix): AcampAP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampAP.cpp$(DependSuffix) -MM "AcampAP.cpp"

$(IntermediateDirectory)/AcampAP.cpp$(PreprocessSuffix): AcampAP.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampAP.cpp$(PreprocessSuffix) "AcampAP.cpp"

$(IntermediateDirectory)/AcampNetwork.cpp$(ObjectSuffix): AcampNetwork.cpp $(IntermediateDirectory)/AcampNetwork.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampNetwork.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampNetwork.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampNetwork.cpp$(DependSuffix): AcampNetwork.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampNetwork.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampNetwork.cpp$(DependSuffix) -MM "AcampNetwork.cpp"

$(IntermediateDirectory)/AcampNetwork.cpp$(PreprocessSuffix): AcampNetwork.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampNetwork.cpp$(PreprocessSuffix) "AcampNetwork.cpp"

$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix): AcampProtocol.cpp $(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampProtocol.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix): AcampProtocol.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix) -MM "AcampProtocol.cpp"

$(IntermediateDirectory)/AcampProtocol.cpp$(PreprocessSuffix): AcampProtocol.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampProtocol.cpp$(PreprocessSuffix) "AcampProtocol.cpp"

$(IntermediateDirectory)/AcampRegisterState.cpp$(ObjectSuffix): AcampRegisterState.cpp $(IntermediateDirectory)/AcampRegisterState.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampRegisterState.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampRegisterState.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampRegisterState.cpp$(DependSuffix): AcampRegisterState.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampRegisterState.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampRegisterState.cpp$(DependSuffix) -MM "AcampRegisterState.cpp"

$(IntermediateDirectory)/AcampRegisterState.cpp$(PreprocessSuffix): AcampRegisterState.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampRegisterState.cpp$(PreprocessSuffix) "AcampRegisterState.cpp"

$(IntermediateDirectory)/APMain.cpp$(ObjectSuffix): APMain.cpp $(IntermediateDirectory)/APMain.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/APMain.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/APMain.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/APMain.cpp$(DependSuffix): APMain.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/APMain.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/APMain.cpp$(DependSuffix) -MM "APMain.cpp"

$(IntermediateDirectory)/APMain.cpp$(PreprocessSuffix): APMain.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/APMain.cpp$(PreprocessSuffix) "APMain.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


