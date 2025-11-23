
const char* PAGE_LOGIN = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wireguard VPN Proxy Login</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .login-container {
      background: white;
      border-radius: 12px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      width: 100%;
      max-width: 400px;
      padding: 40px;
    }
    .login-header {
      text-align: center;
      margin-bottom: 30px;
    }
    .login-header h1 {
      font-size: 28px;
      color: #333;
      margin-bottom: 5px;
    }
    .login-header p {
      font-size: 14px;
      color: #999;
    }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      font-size: 13px;
      font-weight: 600;
      color: #555;
      margin-bottom: 8px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    input[type="password"] {
      width: 100%;
      padding: 12px 15px;
      border: 2px solid #ddd;
      border-radius: 6px;
      font-size: 16px;
      font-family: inherit;
      transition: all 0.3s;
    }
    input[type="password"]:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    button {
      width: 100%;
      padding: 12px 20px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      border-radius: 6px;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    button:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3);
    }
    .error {
      background: #fee;
      color: #c33;
      padding: 12px;
      border-radius: 6px;
      margin-bottom: 20px;
      font-size: 13px;
      border-left: 4px solid #f5576c;
      display: none;
    }
  </style>
</head>
<body>
  <div class="login-container">
    <div class="login-header">
      <h1>Wireguard VPN Proxy</h1>
      <p>Configuration Portal</p>
    </div>
    
    <div id="error" class="error">%ERROR_MSG%</div>
    
    <form method="POST" action="/login">
      <div class="form-group">
        <label for="password">Password</label>
        <input type="password" id="password" name="password" placeholder="Enter access password" required autofocus>
      </div>
      <button type="submit">Login</button>
    </form>
  </div>
  
  <script>
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.get('error') === '1') {
      document.getElementById('error').textContent = 'Invalid password. Please try again.';
      document.getElementById('error').style.display = 'block';
    }
  </script>
</body>
</html>
)HTML";

const char* PAGE_INDEX = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>VPN Proxy Config</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background: white;
      border-radius: 12px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      overflow: hidden;
    }
    .header {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 30px 20px;
      text-align: center;
    }
    .header h1 { font-size: 28px; margin-bottom: 5px; }
    .header p { font-size: 14px; opacity: 0.9; }
    .content { padding: 30px 20px; }
    .section {
      margin-bottom: 30px;
      padding-bottom: 30px;
      border-bottom: 1px solid #eee;
    }
    .section:last-child { border-bottom: none; padding-bottom: 0; }
    .section h3 {
      font-size: 18px;
      color: #333;
      margin-bottom: 15px;
      display: flex;
      align-items: center;
    }
    .section h3:before {
      content: '';
      width: 4px;
      height: 20px;
      background: #667eea;
      border-radius: 2px;
      margin-right: 10px;
    }
    .form-group {
      margin-bottom: 15px;
    }
    label {
      display: block;
      font-size: 13px;
      font-weight: 600;
      color: #555;
      margin-bottom: 6px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    input[type="text"],
    input[type="password"],
    textarea {
      width: 100%;
      padding: 10px 12px;
      border: 1px solid #ddd;
      border-radius: 6px;
      font-size: 14px;
      font-family: inherit;
      transition: border-color 0.3s;
    }
    input[type="text"]:focus,
    input[type="password"]:focus,
    textarea:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    textarea { resize: vertical; min-height: 60px; }
    .button-group {
      display: flex;
      gap: 10px;
      margin-top: 20px;
    }
    button, input[type="submit"] {
      flex: 1;
      padding: 12px 20px;
      border: none;
      border-radius: 6px;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3); }
    .btn-secondary {
      background: #f0f0f0;
      color: #333;
      flex: 1;
    }
    .btn-secondary:hover { background: #e0e0e0; }
    .btn-toggle {
      width: 100%;
      background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
      color: white;
    }
    .btn-toggle:hover { transform: translateY(-2px); box-shadow: 0 10px 20px rgba(245, 87, 108, 0.3); }
    .btn-logout {
      width: 100%;
      background: #999;
      color: white;
      text-decoration: none;
      display: inline-block;
      text-align: center;
      margin-top: 10px;
      padding: 10px 20px;
      border-radius: 6px;
      font-weight: 600;
    }
    .btn-logout:hover { background: #777; }
    .btn-reboot {
      width: 100%;
      background: #e74c3c;
      color: white;
      text-decoration: none;
      display: inline-block;
      text-align: center;
      margin-top: 10px;
      padding: 10px 20px;
      border: none;
      border-radius: 6px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s;
      font-size: 14px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    .btn-reboot:hover { background: #c0392b; transform: translateY(-2px); box-shadow: 0 10px 20px rgba(231, 76, 60, 0.3); }
    .footer {
      padding: 20px;
      text-align: center;
      background: #f9f9f9;
    }
    .footer a {
      color: #667eea;
      text-decoration: none;
      font-weight: 600;
      transition: color 0.3s;
    }
    .footer a:hover { color: #764ba2; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>VPN Proxy</h1>
      <p>WireGuard Configuration Portal</p>
    </div>
    
    <div class="content">
      <!-- WLAN Section -->
      <div class="section">
        <h3>WLAN Configuration</h3>
        <form method="POST" action="/save_wifi">
          <div class="form-group">
            <label for="ssid">Primary Network Name (SSID)</label>
            <input type="text" id="ssid" name="ssid" value="%SSID%" placeholder="Enter primary WiFi network name">
          </div>
          <div class="form-group">
            <label for="pass">Primary Password</label>
            <input type="password" id="pass" name="pass" value="%PASS%" placeholder="Enter primary WiFi password">
          </div>
          <div class="form-group">
            <label for="ssid2">Secondary Network Name (SSID) - Optional Fallback</label>
            <input type="text" id="ssid2" name="ssid2" value="%SSID2%" placeholder="Enter secondary WiFi network name (optional)">
          </div>
          <div class="form-group">
            <label for="pass2">Secondary Password</label>
            <input type="password" id="pass2" name="pass2" value="%PASS2%" placeholder="Enter secondary WiFi password (optional)">
          </div>
          <button type="submit" class="btn-primary">Connect & Save</button>
        </form>
      </div>

      <!-- WireGuard Section -->
      <div class="section">
        <h3>WireGuard Configuration</h3>
        <form method="POST" action="/save_wg">
          <div class="form-group">
            <label for="wg_local">Local IP Address (CIDR)</label>
            <input type="text" id="wg_local" name="wg_local" value="%WG_LOCAL%" placeholder="e.g., 10.0.0.2/32">
          </div>
          <div class="form-group">
            <label for="wg_dns">DNS IP Address</label>
            <input type="text" id="wg_dns" name="wg_dns" value="%WG_DNS%" placeholder="e.g., 8.8.8.8">
          </div>
          <div class="form-group">
            <label for="priv">Private Key</label>
            <textarea id="priv" name="priv" placeholder="Paste your WireGuard private key">%WG_PRIV%</textarea>
          </div>
          <div class="form-group">
            <label for="peer">Peer Public Key</label>
            <textarea id="peer" name="peer" placeholder="Paste the peer's public key">%WG_PEER%</textarea>
          </div>
          <div class="form-group">
            <label for="ep">Endpoint (host:port)</label>
            <input type="text" id="ep" name="ep" value="%WG_EP%" placeholder="e.g., vpn.example.com:51820">
          </div>
          <div class="form-group">
            <label for="ai">Allowed IPs</label>
            <input type="text" id="ai" name="ai" value="%WG_AI%" placeholder="e.g., 0.0.0.0/0">
          </div>
          <button type="submit" class="btn-primary">Save Configuration</button>
        </form>
        <form method="POST" action="/wg_toggle" style="margin-top: 15px;">
          <button type="submit" class="btn-toggle">%WG_TOGGLE%</button>
        </form>
      </div>

      <!-- Port Forwarding Section -->
      <div class="section">
        <h3>TCP Port Forwarding</h3>
        <form method="POST" action="/save_fwd">
          <div class="form-group">
            <label for="lport">Local Port</label>
            <input type="text" id="lport" name="lport" value="%FWD_LPORT%" placeholder="e.g., 8080">
          </div>
          <div class="form-group">
            <label for="rhost">Remote Host (VPN)</label>
            <input type="text" id="rhost" name="rhost" value="%FWD_RHOST%" placeholder="e.g., internal.server">
          </div>
          <div class="form-group">
            <label for="rport">Remote Port</label>
            <input type="text" id="rport" name="rport" value="%FWD_RPORT%" placeholder="e.g., 80">
          </div>
          <button type="submit" class="btn-primary">Save Forwarding Rules</button>
        </form>
      </div>

      <!-- MQTT & Sensor Section -->
      <div class="section">
        <h3>MQTT & Temperature/Humidity Sensor</h3>
        <form method="POST" action="/save_mqtt">
          <div class="form-group">
            <label for="mqtt_server">MQTT Server IP Address</label>
            <input type="text" id="mqtt_server" name="mqtt_server" value="%MQTT_SERVER%" placeholder="e.g., 192.168.1.100">
          </div>
          <div class="form-group">
            <label for="mqtt_port">MQTT Port</label>
            <input type="text" id="mqtt_port" name="mqtt_port" value="%MQTT_PORT%" placeholder="Default: 1883">
          </div>
          <div class="form-group">
            <label for="mqtt_user">MQTT Username (Optional)</label>
            <input type="text" id="mqtt_user" name="mqtt_user" value="%MQTT_USER%" placeholder="Leave empty if not required">
          </div>
          <div class="form-group">
            <label for="mqtt_pass">MQTT Password (Optional)</label>
            <input type="password" id="mqtt_pass" name="mqtt_pass" value="%MQTT_PASS%" placeholder="Leave empty if not required">
          </div>
          <div class="form-group">
            <label for="mqtt_prefix">MQTT Topic Prefix</label>
            <input type="text" id="mqtt_prefix" name="mqtt_prefix" value="%MQTT_PREFIX%" placeholder="e.g., esp32/sensor or home/livingroom">
          </div>
          <button type="submit" class="btn-primary">Save MQTT Configuration</button>
        </form>
        <form method="POST" action="/mqtt_toggle" style="margin-top: 15px;">
          <button type="submit" class="btn-toggle">%MQTT_TOGGLE%</button>
        </form>
      </div>

      <!-- Settings Section -->
      <div class="section">
        <h3>Security Settings</h3>
        <form method="POST" action="/save_settings">
          <div class="form-group">
            <label for="settings_pwd">Settings Password</label>
            <input type="password" id="settings_pwd" name="settings_pwd" value="%SETTINGS_PWD%" placeholder="Leave empty to disable authentication">
          </div>
          <button type="submit" class="btn-primary">Save Password</button>
        </form>
      </div>
    </div>

    <div class="footer">
      <a href="/status">→ View Status Dashboard</a>
      %LOGOUT_BTN%
      <form method="POST" action="/reboot" style="margin-top: 10px;">
        <button type="submit" class="btn-reboot" onclick="return confirm('Are you sure you want to reboot the device?');">⚠ Reboot Device</button>
      </form>
    </div>
  </div>
</body>
</html>
)HTML";

const char* PAGE_STATUS = R"HTML(
<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wireguard VPN Proxy Status</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background: white;
      border-radius: 12px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
      overflow: hidden;
    }
    .header {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      padding: 30px 20px;
      text-align: center;
    }
    .header h1 { font-size: 28px; margin-bottom: 5px; }
    .content { padding: 30px 20px; }
    .stat-card {
      background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
      padding: 15px;
      border-radius: 8px;
      text-align: center;
      border-left: 4px solid #667eea;
      margin-bottom: 15px;
    }
    .stat-label {
      font-size: 12px;
      color: #666;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      margin-bottom: 8px;
      font-weight: 600;
    }
    .stat-value {
      font-size: 22px;
      font-weight: 700;
      color: #333;
      word-break: break-all;
    }
    .throughput-section {
      margin-top: 25px;
      padding-top: 25px;
      border-top: 2px solid #eee;
    }
    .throughput-section h3 {
      font-size: 16px;
      color: #333;
      margin-bottom: 15px;
      display: flex;
      align-items: center;
    }
    .throughput-section h3:before {
      content: '';
      width: 4px;
      height: 18px;
      background: #667eea;
      border-radius: 2px;
      margin-right: 10px;
    }
    .byte-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid #eee;
      font-size: 14px;
    }
    .byte-row:last-child { border-bottom: none; }
    .byte-label { color: #666; font-weight: 500; }
    .byte-value { color: #333; font-weight: 700; font-family: 'Monaco', monospace; }
    .footer {
      padding: 20px;
      text-align: center;
      background: #f9f9f9;
    }
    .footer a {
      color: #667eea;
      text-decoration: none;
      font-weight: 600;
      transition: color 0.3s;
    }
    .footer a:hover { color: #764ba2; }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Status Dashboard</h1>
      <p>Real-time System Information</p>
    </div>
    
    <div class="content">
      <div class="stat-card">
        <div class="stat-label">Local IP</div>
        <div class="stat-value">%LOCAL_IP%</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">WiFi Network</div>
        <div class="stat-value" style="font-size: 18px;">%WIFI_SSID%</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Temperature & Humidity</div>
        <div class="stat-value">%TEMPERATURE%°C / %HUMIDITY%%</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">WireGuard Status</div>
        <div class="stat-value">%WG_STATE%</div>
      </div>
      <div class="stat-card">
        <div class="stat-label">Port Forwarding</div>
        <div class="stat-value" style="font-size: 16px;">%F_LPORT% → %F_RHOST%:%F_RPORT%</div>
      </div>

      <div class="throughput-section">
        <h3>Data Transfer</h3>
        <div class="byte-row">
          <span class="byte-label">Download (RX)</span>
          <span class="byte-value">%WG_RX%</span>
        </div>
        <div class="byte-row">
          <span class="byte-label">Upload (TX)</span>
          <span class="byte-value">%WG_TX%</span>
        </div>
        <div class="byte-row">
          <span class="byte-label">Throughput</span>
          <span class="byte-value">%THR% kb/s</span>
        </div>
      </div>
    </div>

    <div class="footer">
      <a href="/">← Back to Configuration</a>
    </div>
  </div>
</body>
</html>
)HTML";