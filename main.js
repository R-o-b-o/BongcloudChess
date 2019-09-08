const electron = require('electron')

function createWindow () {
  const { width, height } = electron.screen.getPrimaryDisplay().workAreaSize;
  let win = new electron.BrowserWindow({
    width: height,
    height: height*0.95,
    icon: __dirname + '/favicon.ico'
  })
  win.setMenu(null);
  win.loadFile('index.html')
}

electron.app.on('ready', createWindow)