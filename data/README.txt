SmartClock Web UI — modular frontend

Files:
  index.html
  css/base.css
  css/components.css
  css/main.css
  css/alarms.css
  js/app.js
  js/state.js
  js/ui.js
  js/api.js
  js/params.js
  js/sd.js
  js/audio.js
  js/tabs.js
  js/alarms/alarms.js
  js/alarms/alarm-editor.js
  js/alarms/alarm-phases.js
  js/alarms/alarm-utils.js

REST API paths are unchanged. Existing inline onclick handlers are kept compatible because app.js exposes the public functions on window.

ESP32/LittleFS requirement:
The WebServerManager must serve these static files:
  /index.html       -> text/html
  /css/*.css        -> text/css
  /js/*.js          -> application/javascript

The API endpoints remain /api/params, /api/param, /api/sd, /api/audio/* and /api/alarms/*.
