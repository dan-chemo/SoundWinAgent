# Sound Windows Agent

Sound Windows Agent detects and visualizes plug-and-play audio endpoint devices under Windows. It integrates with audio control interfaces and provides mechanisms to handle audio notifications and device changes.

## Executables Generated
- **SoundAgentCli**: Command-line interface for audio control.
- **SoundAgent**: Single-device client application GUI.

## Technologies Used
- **C++**: Core logic implementation.
- **C#**: GUI and client application.

In order to build:

1. Check out the current repository and the repository commonLibsCpp;
2. Add to NuGet sources the local path to "commonLibsCpp\OutputArtifacts"
3. Set NuGet environment variable to the path of the NuGet executable.
4. Build the solution
	- "%nuget%" restore SoundAgent.sln
	- "c:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe" SoundWinAgent.sln /p:Configuration=Release /target:Rebuild -restore
	- dotnet publish \"Projects\\SoundAgent\\SoundAgent.csproj\" -c Release -p:PublishProfile=JustOutputPublishProfile"
