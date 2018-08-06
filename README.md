# Uptime Faker
Generic Windows library designed to help detecting issues related to high PC uptime.

## Reason for creating this plugin
The most accurate timers in Windows all count time from the time Windows has started.
Historically, these values often were relatively small, as people usually shut down their PC at night,
and therefore uptime stayed low. However, with the introduction of Fast Startup in Windows 8,
uptime stopped resetting after shutting down the PC (since it's now effectively a partial hibernation).
This led to uptimes inflating noticeably for most people, as with Fast Startup enabled it resets **only** on a full PC reboot.

Turns out, older software often cannot handle high uptimes.
Much to my surprise, Application Verifier did not have any options to help detecting such issues by faking high uptime,
I decided to create this plugin.

## Usage
This plugin is a Detours plugin. Therefore, it can be injected into the process with
[DetourCreateProcessWithDlls](https://github.com/Microsoft/Detours/wiki/DetourCreateProcessWithDlls)
or by any other means.
It is also possible to inject UptimeFaker by using [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases),
but file extension needs to be renamed to ASI first.

Hooked WinAPI functions and the amount of uptime added can be configured with a provided `UptimeFaker.ini` file.

## Supported functions
* From **kernel32.dll**:
  * QueryPerformanceCounter
  * GetTickCount
  * GetTickCount64
* From **winmm.dll**:
  * timeGetTime
  * timeGetSystemTime

## Third party dependencies
This project uses [Detours](https://github.com/Microsoft/Detours) to hook into WinAPI time functions.
