#pragma once

#include <string>
#include <vector>
#include "Common.h"
#include <map>
#include <set>
#include "Build/BuildSettings.h"
#include "Json/json.hpp"

typedef nlohmann::json Json;

struct WindowsTargetLibs
{
	std::vector<std::string> MSVCLibs;
	std::vector<std::string> MinGWLibs;
};

struct LinuxTargetLibs
{
	std::vector<std::string> GCCLibs;
};

struct PlatformThirdParty
{
	std::vector<std::string> Includes;

	LinuxTargetLibs Linux64Libs;
	LinuxTargetLibs Linux32Libs;
	WindowsTargetLibs Win64Libs;
	WindowsTargetLibs Win32Libs;
};

struct PlatformLibDef
{
	LinuxTargetLibs Linux64Libs;
	LinuxTargetLibs Linux32Libs;
	WindowsTargetLibs Win64Libs;
	WindowsTargetLibs Win32Libs;
};

enum ModuleType
{
	EXECUTABLE, LIBRARY
};

class ExternDependency
{
public:
	std::string Name;

	std::string GetIncludePath() const;

	std::string GetPlatformLibraryPath(const BuildSettings& Settings) const;
	std::string GetPlatformBinaryPath(const BuildSettings& Settings) const;

	void GetPlatformLibs(const BuildSettings& Settings, std::vector<std::string>& OutLibs) const;
	void GetPlatformBins(const BuildSettings& Settings, std::vector<std::string>& OutBins) const;
	void GetPlatformBinPaths(const BuildSettings& Settings, std::vector<std::string>& OutBins) const;

};

class Module
{
public:

	std::string Name;
	ModuleType Type;
	std::string RootDir;
	std::vector<std::string> ModuleDependencies;
	std::vector<std::string> MacroDefinitions;
	std::vector<std::string> OSXFrameworks;
	std::vector<ExternDependency> ExternDependencies;

	std::vector<std::string> PythonIncludes;
	std::vector<std::string> PythonLibraryPaths;
	std::vector<std::string> PythonLibraries;

	std::string ModuleFilePath;
	
	// Whether this module built successfully without errors
	bool bBuiltSuccessfully = true;

	bool bNeededFullRebuild = false;

	// Flag used when iterating through models
	bool bVisisted = false;

	// Whether we've attempted to build this module. Note this doesn't mean the build was successful.
	bool bAttemptedBuild = false;

	bool bEngineModule = false;

	bool HasCorrespondingObject(std::string ObjectDir, std::string ObjectExt, std::string SourceFile)
	{
		Filesystem::path SourceFilePath = Filesystem::path(SourceFile);
		
		std::string SourceFileName = SourceFilePath.filename().string();
		SourceFileName = SourceFileName.substr(0, SourceFileName.find(SourceFilePath.extension().string())); // Strip off the file extension

		Filesystem::path ObjFilePath = (Filesystem::path(ObjectDir) / (SourceFileName + ObjectExt)).string();

		return Filesystem::exists(ObjFilePath);
	}
	
	void DiscoverHeaders(std::vector<std::string>& OutHeaders, bool bThirdParty = false) const
	{
		if(Filesystem::exists(GetIncludeDir()))
		{
			Filesystem::recursive_directory_iterator DirectoryItr(GetIncludeDir());

			for (Filesystem::path File : DirectoryItr)
			{
				// Detect if this file is a source file
				if (File.extension() == ".h")
				{
					OutHeaders.push_back(Filesystem::absolute(File).string());
				}
			}
		}

		// Optionally scan through third party directory for source
		if (bThirdParty && Filesystem::exists(GetThirdPartyDir()))
		{
			Filesystem::recursive_directory_iterator DirectoryItrThirdParty(GetThirdPartyDir());

			for (Filesystem::path File : DirectoryItrThirdParty)
			{
				// Detect if this file is a source file
				if (File.extension() == ".h")
				{
					OutHeaders.push_back(Filesystem::absolute(File).string());
				}
			}
		}
	}

	bool SourceCheck() const
	{
		Filesystem::recursive_directory_iterator DirectoryItr(GetCppDir());

		for (Filesystem::path File : DirectoryItr)
		{
			// Detect if this file is a source file
			if (File.extension() == ".cpp" || File.extension() == ".hpp" || File.extension() == ".c")
			{
				return true;
			}
		}

		return false;
	}
	
	void DiscoverSource(std::vector<std::string>& OutSource, bool bThirdParty = false) const
	{
		if(Filesystem::exists(GetCppDir()))
		{
			Filesystem::recursive_directory_iterator DirectoryItr(GetCppDir());

			for (Filesystem::path File : DirectoryItr)
			{
				// Detect if this file is a source file
				if (File.extension() == ".cpp" || File.extension() == ".hpp" || File.extension() == ".c")
				{
					OutSource.push_back(Filesystem::absolute(File).string());
				}
			}
		}

		// Optionally scan through third party directory for source
		if(bThirdParty && Filesystem::exists(GetThirdPartyDir()))
		{
			Filesystem::recursive_directory_iterator DirectoryItrThirdParty (GetThirdPartyDir());

			for (Filesystem::path File : DirectoryItrThirdParty)
			{
				// Detect if this file is a source file
				if (File.extension() == ".cpp" || File.extension() == ".hpp" || File.extension() == ".c")
				{
					OutSource.push_back(Filesystem::absolute(File).string());
				}
			}
		}
	}

	void DiscoverObjectFiles(std::string ObjectDir, std::string ObjectExt, std::vector<std::string>& OutFoundObjectFiles, std::vector<std::string>& OutMissingObjectFiles)
	{
		std::vector<std::string> DiscoveredSource;
		DiscoverSource(DiscoveredSource);

		for (const std::string& SourceFile : DiscoveredSource)
		{
			Filesystem::path SourceFilePath = Filesystem::path(SourceFile);

			std::string SourceFileName = SourceFilePath.filename().string();
			SourceFileName = SourceFileName.substr(0, SourceFileName.find(SourceFilePath.extension().string())); // Strip off the file extension

			Filesystem::path ObjFilePath = (Filesystem::path(ObjectDir) / (SourceFileName + ObjectExt)).string();

			if (Filesystem::exists(ObjFilePath))
			{
				OutFoundObjectFiles.push_back(ObjFilePath.string());
			}
			else
			{
				OutMissingObjectFiles.push_back(ObjFilePath.string());
			}

		}
	}

	void GatherIncludes(const std::map<std::string, Module*>& AllModules, std::vector<std::string>& OutIncludes)
	{
		//ModuleIncludes.push_back(PathRelativeTo(Filesystem::absolute("."), TheModule.GetIncludeDir()).string());
		OutIncludes.push_back(GetIncludeDir());
		OutIncludes.push_back(GetGeneratedDir());

		for (ExternDependency& ExternDep : ExternDependencies)
		{
			//ModuleIncludes.push_back(PathRelativeTo(Filesystem::absolute("."), ExternDep.GetIncludePath()).string());
			OutIncludes.push_back(ExternDep.GetIncludePath());
		}
		
		for (std::string& ModuleDep : ModuleDependencies)
		{
			Module* ModDep = AllModules.at(ModuleDep);

			if (ModDep)
			{
				//ModuleIncludes.push_back(PathRelativeTo(Filesystem::absolute("."), ModDep->GetIncludeDir()).string());
				OutIncludes.push_back(ModDep->GetIncludeDir());
				OutIncludes.push_back(ModDep->GetGeneratedDir());
			}
		}
	}

	std::string GetRootDir() const
	{
		return Filesystem::absolute(RootDir).string() + (char) Filesystem::path::preferred_separator;
	}

	std::string GetPythonBuildScriptPath() const
	{
		return (Filesystem::path(RootDir) / (Name + std::string(".build.py"))).string();
	}

	std::string GetCppDir() const
	{
		return (Filesystem::path(RootDir) / (std::string("Source") + (char)Filesystem::path::preferred_separator)).string();
	}

	std::string GetThirdPartyBinDir() const
	{
		return (Filesystem::path(GetThirdPartyDir()) / (std::string("Binaries") + (char)Filesystem::path::preferred_separator)).string();
	}

	std::string GetThirdPartyDir() const
	{
		return (Filesystem::path(RootDir) / (std::string("ThirdParty") + (char)Filesystem::path::preferred_separator)).string();
	}

	std::string GetIncludeDir() const
	{
		return (Filesystem::path(RootDir) / (std::string("Include") + (char)Filesystem::path::preferred_separator)).string();
	}

	std::string GetGeneratedDir() const;
	std::string GetGeneratedSourcePath() const;

	std::string GetArtifactName() const
	{
		return std::string("RyRuntime-") + Name;
	}

	bool IsExecutable(BuildSettings& Settings) const
	{
		if(Settings.bDistribute && !bEngineModule && Type == ModuleType::EXECUTABLE)
		{
			return true;
		}

		if (!Settings.bDistribute && bEngineModule && Type == ModuleType::EXECUTABLE)
		{
			return true;
		}

		return false;
	}

	void GetTargetBins(const BuildSettings& Settings, std::vector<std::string>& OutBinaries)
	{
		Filesystem::path ThirdPartyBin = GetThirdPartyBinDir();

		if(Settings.TargetPlatform.Arch == ArchitectureType::X64)
		{
			ThirdPartyBin /= "x64";
		}
		else if(Settings.TargetPlatform.Arch == ArchitectureType::X86)
		{
			ThirdPartyBin /= "x86";
		}
		else if(Settings.TargetPlatform.Arch == ArchitectureType::ARM)
		{
			ThirdPartyBin /= "Arm";
		}
		else
		{
			return;
		}

		if (Settings.TargetPlatform.OS == OSType::WINDOWS)
		{
			ThirdPartyBin /= "Windows";
		}
		else if(Settings.TargetPlatform.OS == OSType::OSX)
		{
			ThirdPartyBin /= "OSX";
		}
		else if(Settings.TargetPlatform.OS == OSType::LINUX)
		{
			ThirdPartyBin /= "Linux";
		}
		else
		{
			return;
		}

		if (Settings.Toolset == ToolsetType::MSVC)
		{
			ThirdPartyBin /= "MSVC";
		}
		else if (Settings.Toolset == ToolsetType::GCC)
		{
			if(Settings.TargetPlatform.OS == OSType::WINDOWS)
			{
				ThirdPartyBin /= "MinGW";
			}
			else
			{
				ThirdPartyBin /= "GCC";
			}
		}
		else if (Settings.Toolset == ToolsetType::CLANG)
		{
			ThirdPartyBin /= "Clang";
		}
		else
		{
			return;
		}

		if(Filesystem::exists(ThirdPartyBin))
		{
			Filesystem::directory_iterator BinItr(ThirdPartyBin);

			for (auto& Bin : BinItr)
			{
				OutBinaries.push_back(Bin.path().string());
			}
		}

	}
	
};

Module* LoadModule(Filesystem::path Path, const BuildSettings* Settings);

void DiscoverModules(Filesystem::path RootDir, std::vector<Module*>& OutModules);
void LoadModules(Filesystem::path RootDir, std::vector<Module*>& OutModules, const BuildSettings* Settings = nullptr);
bool VerifyModules(std::vector<Module*>& OutModules);
bool IsModuleOutOfDate(const Module& Module, std::string BinaryDir, BuildSettings Settings);

void TopSort(const std::vector<std::string>& InModules, const std::map<std::string, Module*>& ModMap, std::vector<std::string>& OutModules);
void TopSort_Helper(std::vector<std::string>& OutModules, const std::map<std::string, Module*>& ModMap, std::set<std::string>& Visited, std::string RootModule);

void RecurseDependencies(const Module& Mod, const std::map<std::string, Module*>& ModMap, std::vector<std::string>& OutMods);