using UnrealBuildTool;
using System.IO;

public class InfinityTable : ModuleRules
{
    public InfinityTable(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core","CoreUObject","Engine","InputCore",
            "UMG","Slate","SlateCore",
            "NetCore","OnlineSubsystem","OnlineSubsystemUtils",
            "PhysicsCore","Chaos",
            "MetasoundEngine","AudioMixer",
            "Niagara","Json","JsonUtilities","HTTP"
        });
        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "OnlineSubsystemNull","OnlineSubsystemEOS","glTFRuntime"
        });
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/lua54/include");
        PublicIncludePaths.Add("$(ProjectDir)/ThirdParty/sol2/include");
        PublicAdditionalLibraries.Add("$(ProjectDir)/ThirdParty/lua54/lib/lua54.lib");

        // SQLite3 ships as a single-file amalgamation (sqlite3.c / sqlite3.h)
        // rather than a prebuilt lib, so it's compiled directly into this
        // module instead of being linked as a static library.
        string SQLitePath = Path.Combine(ModuleDirectory, "..", "..", "..", "ThirdParty", "sqlite3");
        PublicIncludePaths.Add(SQLitePath);
        string SQLiteSource = Path.Combine(SQLitePath, "sqlite3.c");
        if (File.Exists(SQLiteSource))
        {
            // UBT only compiles .cpp by default; expose the amalgamation as a
            // valid translation unit via a thin wrapper compiled as part of
            // this module's own source tree.
            string WrapperPath = Path.Combine(ModuleDirectory, "ThirdParty", "sqlite3_amalgamation.cpp");
            Directory.CreateDirectory(Path.GetDirectoryName(WrapperPath));
            if (!File.Exists(WrapperPath))
            {
                File.WriteAllText(WrapperPath,
                    "// Auto-generated: compiles the SQLite3 amalgamation as part of this module.\n" +
                    "// See ThirdParty/install.bat / install.sh to fetch sqlite3.c/.h.\n" +
                    "#include \"sqlite3.c\"\n");
            }
        }
        bEnableUndefinedIdentifierWarnings = false;
        PublicDefinitions.Add("SQLITE_THREADSAFE=1");
        PublicDefinitions.Add("SQLITE_OMIT_LOAD_EXTENSION=1");
    }
}
