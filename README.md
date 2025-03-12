# Sound Windows Agent

Sound Agent detects and visualizes plug-and-play audio endpoint devices under Windows. It integrates with audio control interfaces and handles audio notifications and device changes.

The Sound Agent Service collects audio device information and sends it to a remote server.

## Executables Generated
- **SoundAgentCli**: Command-line interface for audio control.
- **SoundAgentService**: Windows Service collects audio device information and sends it to a remote server.

## Technologies Used
- **C++**: Core logic implementation.
- **Packages**: Poco and Cppservice vcpkg packages in order to utilize HTTP and Windows Server Manager

In order to build:

1. Check out the current repository and the repository commonLibsCpp;
2. Add to NuGet sources the local path to "commonLibsCpp\OutputArtifacts"
3. Set NuGet environment variable to the path of the NuGet executable.
4. Build the solution
	- "%nuget%" restore SoundAgent.sln
	- "c:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe" SoundWinAgent.sln /p:Configuration=Release /target:Rebuild -restore
5. Install / Uninstall / Start / Stop the SoundAgentService service
	- (elevated) SoundAgentService.exe /registerService [/startup=auto|manual|disabled]. 
	- (elevated) SoundAgentService.exe /unregisterService
	- net start SoundAgentService
	- net stop SoundAgentService

