WireGuard VPN Proxy

A comprehensive WireGuard VPN relay and TCP port forwarding solution for ESP32 with web-based configuration interface. This project transforms your T-Display S3 into a portable VPN gateway with real-time status monitoring.

## Features

- **WireGuard VPN Integration** - Full WireGuard client implementation for secure VPN connectivity
- **TCP Port Forwarding** - Forward local ports to remote hosts through the VPN tunnel
- **Web Configuration Portal** - Modern, responsive web UI for easy setup and management
- **Password Protection** - Optional authentication for the settings page
- **Real-time Status Display** - AMOLED display showing live VPN, network, and data transfer statistics
- **Dual Mode WiFi** - Access Point mode for configuration + Station mode for actual operation
- **Session Management** - Secure cookie-based session handling with 1-hour timeout
- **Automatic Service Management** - Services automatically start/stop based on WiFi connection state

## Hardware Requirements

- **ESP32-S3** based T-Display AMOLED board (LilyGo T-Display S3)
- USB-C cable for programming and power
- WiFi network for internet connectivity

## Software Requirements

- Arduino IDE 2.0 or later
- ESP32 board package (2.0.0 or later)
- Required Libraries:
  - `WiFi` (built-in)
  - `Preferences` (built-in)
  - `WiFiUdp` (built-in)
  - `WireGuard-ESP32` - Install via Library Manager
  - `ESP Async WebServer` (by me-no-dev) - Install via Library Manager
  - `LilyGo AMOLED` - Install via Library Manager
  - `LV_Helper` - Install via Library Manager
  - `TFT_eSPI` - Install via Library Manager

## Installation & Setup

### 1. Install Required Libraries

In Arduino IDE:
- Go to **Sketch** → **Include Library** → **Manage Libraries**
- Search for and install:
  - `WireGuard-ESP32`
  - `ESP Async WebServer`
  - `LilyGo AMOLED`
  - `LV_Helper`
  - `TFT_eSPI`

### 2. Configure TFT_eSPI

After installing `TFT_eSPI`, you need to configure it for the T-Display S3:
- Navigate to the library folder: `Arduino/libraries/TFT_eSPI/`
- Open `User_Setup_Select.h`
- Comment out the default setup and uncomment the LilyGo T-Display S3 setup (usually around line 20-25)
- Alternatively, create a file `User_Setup.h` in the TFT_eSPI folder with appropriate pin definitions

### 3. Upload the Sketch

1. Connect your T-Display S3 via USB-C
2. Select Board: **Tools** → **Board** → **ESP32-S3 Dev Module** (or similar)
3. Select Port: **Tools** → **Port** → Select your USB port
4. Upload: **Sketch** → **Upload** or press Ctrl+U

## Initial Configuration

### First Boot - Access Point Mode

When the device first boots:
1. It creates a WiFi Access Point named: `WG-VPN-Proxy_[HEX_ID]]`
2. Default AP Password: `configureme`
3. Open a web browser and connect to `http://192.168.4.1`

### Configuration Steps

#### 1. Set WiFi Credentials
- **Network Name (SSID)**: Your home/office WiFi network
- **Password**: Your WiFi password
- Click **Connect & Save**

The device will restart and attempt to connect to the specified network.

#### 2. Configure WireGuard VPN

Get your WireGuard configuration details:
- **Local IP Address (CIDR)**: Format like `10.0.0.2/32` (provided by your VPN provider)
- **DNS IP Address**: e.g., `8.8.8.8` or your VPN provider's DNS
- **Private Key**: Your WireGuard private key
- **Peer Public Key**: Your VPN server's public key
- **Endpoint (host:port)**: VPN server address and port, e.g., `vpn.example.com:51820`
- **Allowed IPs**: Routes to tunnel through VPN, e.g., `0.0.0.0/0` (all traffic)

Once configured, click **Save Configuration**, then click **WireGuard on** to activate.

#### 3. Configure Port Forwarding (Optional)

Forward local ports to remote hosts through the VPN:
- **Local Port**: Port to listen on locally, e.g., `8080`
- **Remote Host**: Target server address (resolvable via VPN DNS), e.g., `internal.server`
- **Remote Port**: Target port, e.g., `80`

Click **Save Forwarding Rules** to apply.

#### 4. Set Security Password (Optional)

- Enter a password in the **Settings Password** field
- Leave empty to disable authentication
- Click **Save Password**

Once a password is set, you'll need to authenticate before accessing the configuration page.

## Usage

### Access the Web Interface

**With WiFi Connected:**
- Find the device's IP address from your router or display
- Open `http://<device-ip>` in your browser
- Login if you've set a password
- Monitor and adjust settings as needed

**From Access Point:**
- Connect to `WG-VPN-Proxy_[HEX_ID]` network
- Open `http://192.168.4.1`
- Configure settings

### Real-time Status Display

The AMOLED display shows:
- **Current Time** - Synchronized via NTP
- **WiFi Signal** - Signal strength indicator
- **IP Address** - Current local IP
- **DNS Server** - Active DNS IP
- **Port Forwarding** - Target host and ports
- **WireGuard Status** - Connection state (CONNECTED/OFFLINE)
- **Data Transfer** - Real-time RX/TX bytes and throughput
- **Port Forwarding Status** - ACTIVE/IDLE

### Status Dashboard

Navigate to **View Status Dashboard** to see detailed information:
- Local IP address
- DNS configuration
- WireGuard connection status
- Data transfer statistics
- Port forwarding configuration

## Configuration Storage

All settings are stored in ESP32's NVS (Non-Volatile Storage) and persist across reboots:
- WiFi credentials
- WireGuard configuration
- Port forwarding rules
- Security password

## Advanced Features

### Session Management

- Secure HTTP-only cookies with 1-hour expiration
- Session tokens are randomly generated 32-character hex strings
- Automatic logout after 1 hour of inactivity
- Logout button available in the footer when password is set

### Automatic Service Management

The device intelligently manages services based on WiFi connection:
- **WiFi Connects**: Time sync, WireGuard, and port forwarder automatically start
- **WiFi Disconnects**: All services gracefully stop, DNS restored
- **AP Mode**: Services remain inactive until WiFi connects

### Device Reboot

A reboot button is available in the footer with a confirmation dialog. Rebooting allows:
- Recovery from software issues
- Restarting services after configuration changes
- Safe shutdown before power-off

## Troubleshooting

### Can't Connect to Access Point
- Ensure you're using the correct SSID: `WG-VPN-Proxy_[HEX_ID]]`
- Default password is `configureme`
- The AP is always active at `192.168.4.1`

### WiFi Connection Fails
- Verify SSID and password are correct
- Check that your WiFi network is 2.4GHz (ESP32 typically doesn't support 5GHz)
- Check router signal strength at device location

### WireGuard Won't Connect
- Verify all configuration fields are filled correctly
- Ensure endpoint is reachable from your network
- Check that DNS resolution works for the endpoint hostname
- Verify private key, public key, and endpoint format
- Check the Serial Monitor for error messages

### Port Forwarding Not Working
- Ensure remote host is resolvable via the VPN DNS
- Verify remote host is reachable through the VPN tunnel
- Check that local port isn't already in use
- Monitor the status display for forwarding status (ACTIVE/IDLE)

### Display Shows "AP only"
- Device couldn't connect to configured WiFi
- Check WiFi credentials and network availability
- Reconfigure WiFi through the AP at `192.168.4.1`

### Lost Access to Device
- If password is forgotten, you can still access via AP mode at `192.168.4.1`
- Change the password from the AP configuration page
- To reset everything, clear NVS storage via Arduino IDE (Tools → Erase Flash)

## Serial Monitor

Connect via USB and open Serial Monitor (115200 baud) to see:
- WiFi connection status
- WireGuard initialization and connection attempts
- DNS configuration
- Port forwarding activity
- Error messages and diagnostics

## Security Considerations

- **Password Protection**: Optional, disabled by default. Set a strong password if enabled.
- **HTTPS**: Not implemented (HTTP only). Use on trusted networks only.
- **Default AP Password**: Change immediately after first configuration.
- **Session Timeout**: 1 hour of inactivity. Logout button available for manual logout.
- **Stored Credentials**: WiFi and WireGuard keys are stored in NVS. Consider the security implications.

## Performance Notes

- ESP32 can handle sustained throughput of ~50-100 Mbps depending on VPN encryption
- TCP port forwarding performance depends on available RAM and CPU resources
- Multiple concurrent port forwarding connections share device resources
- Real-time display updates don't impact network performance

## Project Structure

```
TDisplayWGProject/
├── TDisplayWGProject.ino    (Main sketch)
├── config.h / config.cpp    (Configuration management)
├── webui.h / webui.cpp      (Web UI HTML/CSS)
├── webserver.h / webserver.cpp  (Web server handlers)
├── forwarder.h / forwarder.cpp  (Port forwarding logic)
├── WGProject.h              (Project definitions)
└── README.md                (This file)
```

## License

This project is provided as-is for personal use.

## Support & Sponsoring

### Getting Help

For issues, questions, or suggestions:
1. Check the Serial Monitor output for error messages
2. Review the troubleshooting section above
3. Verify hardware connections and power supply
4. Try a full reset by erasing Flash and re-uploading

### Support This Project

If you find this project useful, consider supporting its development:

[![Buy Me A Coffee](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-Support-yellow?style=flat-square)](https://buymeacoffee.com/airfox90)

[Buy Me A Coffee - airfox90](https://buymeacoffee.com/airfox90)

## Version History

- **v1.0** - Initial release with WireGuard, port forwarding, and web UI
- **v1.1** - Added password protection and session management
- **v1.2** - Added ESPAsyncWebServer for improved performance
- **v1.3** - Added automatic service management based on WiFi connection state
- **v1.4** - Added device reboot functionality
