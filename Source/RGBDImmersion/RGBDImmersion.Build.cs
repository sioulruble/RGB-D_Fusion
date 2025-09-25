// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RGBDImmersion : ModuleRules
{
	public RGBDImmersion(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17;


        	PublicIncludePaths.Add("/usr/include/eigen3");
        	PublicIncludePaths.Add("/usr/include");
	      	PublicIncludePaths.Add("/usr/lib/x86_64-linux-gnu/libk4a.so");
	      	
	      	PublicAdditionalLibraries.Add("/usr/lib/x86_64-linux-gnu/libk4a.so");
	      	
	      	        	PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "rclUE", "UMG", "RHI", "RenderCore", "zlib", "LibJpegTurbo", "LibTiff", "OpenCV", "OpenCVHelper", "GeometryCore",  "GeometryFramework", "MeshDescription", "OpenVR" });
        	

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
