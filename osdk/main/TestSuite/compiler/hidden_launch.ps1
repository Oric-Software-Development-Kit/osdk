# hidden_launch.ps1 - dot-sourced by the runners.
#
# Start-EmulatorProcess launches Oricutron either normally or, with
# -Hidden, on a separate hidden Windows desktop: SDL ignores the
# minimized-window startup hint and its window always pops up center
# screen and steals keyboard focus. A window created on another desktop
# is never shown and never takes focus, while the emulation (and the
# printer_out.txt output the runners read) works normally.

if (-not ('OsdkHiddenLauncher' -as [type])) {
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;

public static class OsdkHiddenLauncher
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    struct STARTUPINFO
    {
        public int cb;
        public string lpReserved;
        public string lpDesktop;
        public string lpTitle;
        public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars;
        public int dwFillAttribute, dwFlags;
        public short wShowWindow, cbReserved2;
        public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct PROCESS_INFORMATION
    {
        public IntPtr hProcess, hThread;
        public int dwProcessId, dwThreadId;
    }

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern IntPtr CreateDesktop(string desktop, IntPtr device, IntPtr devmode,
        int flags, uint access, IntPtr attrs);

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    static extern bool CreateProcess(string app, string cmdline, IntPtr pattr,
        IntPtr tattr, bool inherit, int flags, IntPtr env, string cwd,
        ref STARTUPINFO si, out PROCESS_INFORMATION pi);

    const uint GENERIC_ALL = 0x10000000;

    public static int Start(string exe, string args, string cwd)
    {
        // create (or open) the hidden desktop; the handle is deliberately
        // kept for the lifetime of the calling process
        IntPtr desk = CreateDesktop("OsdkTestSuiteDesktop", IntPtr.Zero, IntPtr.Zero,
            0, GENERIC_ALL, IntPtr.Zero);
        if (desk == IntPtr.Zero)
            throw new System.ComponentModel.Win32Exception();

        STARTUPINFO si = new STARTUPINFO();
        si.cb = Marshal.SizeOf(typeof(STARTUPINFO));
        si.lpDesktop = "OsdkTestSuiteDesktop";
        PROCESS_INFORMATION pi;
        if (!CreateProcess(null, "\"" + exe + "\" " + args, IntPtr.Zero, IntPtr.Zero,
                false, 0, IntPtr.Zero, cwd, ref si, out pi))
            throw new System.ComponentModel.Win32Exception();
        return pi.dwProcessId;
    }
}
'@
}

function Start-EmulatorProcess([string]$Exe, [string]$Arguments, [string]$WorkDir, [switch]$Hidden)
{
    if ($Hidden) {
        $procId = [OsdkHiddenLauncher]::Start($Exe, $Arguments, $WorkDir)
        return (Get-Process -Id $procId)
    }
    return (Start-Process $Exe -ArgumentList $Arguments -WorkingDirectory $WorkDir -PassThru)
}
