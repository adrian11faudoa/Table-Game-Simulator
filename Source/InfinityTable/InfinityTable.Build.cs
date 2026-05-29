using UnrealBuildTool;

public class InfinityTable : ModuleRules
{
    public InfinityTable(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore",
            "UMG", "Slate", "SlateCore",
            "NetCore", "OnlineSubsystem", "OnlineSubsystemUtils",
            "PhysicsCore", "Chaos",
            "MetasoundEngine", "AudioMixer",
            "Niagara",
            "Json", "JsonUtilities",
            "HTTP"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemSteam",
            "OnlineSubsystemEOS",
            "glTFRuntime"
        });

        // Lua + sol2
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/lua54/include");
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/sol2/include");
        PublicAdditionalLibraries.Add("$(ProjectDir)/ThirdParty/lua54/lib/lua54.lib");

        // SQLite
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/sqlite3");
    }
}
