##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=ACAMPAccessPoint
ConfigurationName      :=Debug
WorkspacePath          :=/home/ouyc/Documents/Project
ProjectPath            :=/home/ouyc/Documents/Project/ACAMPAccessPoint
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=ouyc
Date                   :=29/04/16
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
Objects0=$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) $(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/AcampTransmission.cpp$(ObjectSuffix) $(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) 



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
$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix): AcampProtocol.cpp $(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampProtocol.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix): AcampProtocol.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampProtocol.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampProtocol.cpp$(DependSuffix) -MM "AcampProtocol.cpp"

$(IntermediateDirectory)/AcampProtocol.cpp$(PreprocessSuffix): AcampProtocol.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampProtocol.cpp$(PreprocessSuffix) "AcampProtocol.cpp"

$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/AcampTransmission.cpp$(ObjectSuffix): AcampTransmission.cpp $(IntermediateDirectory)/AcampTransmission.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampTransmission.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampTransmission.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampTransmission.cpp$(DependSuffix): AcampTransmission.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampTransmission.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampTransmission.cpp$(DependSuffix) -MM "AcampTransmission.cpp"

$(IntermediateDirectory)/AcampTransmission.cpp$(PreprocessSuffix): AcampTransmission.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampTransmission.cpp$(PreprocessSuffix) "AcampTransmission.cpp"

$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix): AcampAP.cpp $(IntermediateDirectory)/AcampAP.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/ouyc/Documents/Project/ACAMPAccessPoint/AcampAP.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/AcampAP.cpp$(DependSuffix): AcampAP.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/AcampAP.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/AcampAP.cpp$(DependSuffix) -MM "AcampAP.cpp"

$(IntermediateDirectory)/AcampAP.cpp$(PreprocessSuffix): AcampAP.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/AcampAP.cpp$(PreprocessSuffix) "AcampAP.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


