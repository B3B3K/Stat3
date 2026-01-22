import psutil
import serial
import serial.tools.list_ports
import time
import struct
import platform
import subprocess
import sys

# Configuration
BAUD_RATE = 115200
UPDATE_INTERVAL = 0.1  # seconds

class SystemMonitor:
    def __init__(self):
        self.os_type = platform.system()
        self.serial_port = None
        self.last_net_io = psutil.net_io_counters()
        self.last_disk_io = psutil.disk_io_counters()
        self.last_time = time.time()
        
        print(f"Detected OS: {self.os_type}")
        
    def find_arduino_port(self):
        """Automatically find Arduino port"""
        ports = serial.tools.list_ports.comports()
        
        print("\nAvailable serial ports:")
        for i, port in enumerate(ports):
            print(f"{i+1}. {port.device} - {port.description}")
        
        if not ports:
            print("No serial ports found!")
            return None
        
        # Try to auto-detect Arduino
        for port in ports:
            if any(keyword in port.description.lower() for keyword in ['ch340', 'ch341', 'cp210', 'usb', 'serial', 'arduino']):
                print(f"\nAuto-detected Arduino at: {port.device}")
                return port.device
        
        # Manual selection
        while True:
            try:
                choice = input(f"\nSelect port (1-{len(ports)}) or 'q' to quit: ").strip()
                if choice.lower() == 'q':
                    return None
                choice = int(choice)
                if 1 <= choice <= len(ports):
                    return ports[choice-1].device
            except (ValueError, KeyboardInterrupt):
                pass
        
    def connect_serial(self):
        """Connect to Arduino via serial"""
        port = self.find_arduino_port()
        if not port:
            return False
        
        try:
            self.serial_port = serial.Serial(port, BAUD_RATE, timeout=1)
            print(f"Connected to {port} at {BAUD_RATE} baud")
            time.sleep(2)  # Wait for Arduino reset
            return True
        except Exception as e:
            print(f"Failed to connect to {port}: {e}")
            return False
    
    def get_cpu_temp(self):
        """Get CPU temperature (cross-platform)"""
        try:
            if self.os_type == "Linux":
                # Try multiple methods for Linux
                temps = psutil.sensors_temperatures()
                
                # Try coretemp (Intel)
                if 'coretemp' in temps and temps['coretemp']:
                    return temps['coretemp'][0].current
                
                # Try k10temp (AMD)
                if 'k10temp' in temps and temps['k10temp']:
                    return temps['k10temp'][0].current
                
                # Try zenpower (AMD Ryzen)
                if 'zenpower' in temps and temps['zenpower']:
                    return temps['zenpower'][0].current
                
                # Try any available sensor
                for name, entries in temps.items():
                    if entries and entries[0].current:
                        return entries[0].current
                
            elif self.os_type == "Windows":
                # Try OpenHardwareMonitor/LibreHardwareMonitor
                try:
                    import wmi
                    w = wmi.WMI(namespace="root\\OpenHardwareMonitor")
                    sensors = w.Sensor()
                    for sensor in sensors:
                        if sensor.SensorType == 'Temperature' and 'CPU' in sensor.Name:
                            return float(sensor.Value)
                except:
                    pass
                
                # Try LibreHardwareMonitor
                try:
                    import wmi
                    w = wmi.WMI(namespace="root\\LibreHardwareMonitor")
                    sensors = w.Sensor()
                    for sensor in sensors:
                        if sensor.SensorType == 'Temperature' and 'CPU' in sensor.Name:
                            return float(sensor.Value)
                except:
                    pass
        
        except Exception as e:
            pass
        
        return 50.0  # Default fallback
    
    def get_gpu_info(self):
        """Get GPU temperature and VRAM usage (cross-platform)"""
        temp = 50.0
        vram_usage = 0.0
        
        try:
            # Try NVIDIA first
            result = subprocess.run(
                ['nvidia-smi', '--query-gpu=temperature.gpu,memory.used,memory.total', 
                 '--format=csv,noheader,nounits'],
                capture_output=True, text=True, timeout=2
            )
            
            if result.returncode == 0:
                values = result.stdout.strip().split(',')
                temp = float(values[0])
                used = float(values[1])
                total = float(values[2])
                vram_usage = (used / total) * 100 if total > 0 else 0
                return temp, vram_usage
        except:
            pass
        
        try:
            # Try AMD (radeontop - Linux only)
            if self.os_type == "Linux":
                result = subprocess.run(
                    ['radeontop', '-d', '-', '-l', '1'],
                    capture_output=True, text=True, timeout=2
                )
                if result.returncode == 0:
                    # Parse radeontop output
                    for line in result.stdout.split('\n'):
                        if 'vram' in line.lower():
                            # Extract VRAM percentage
                            pass
        except:
            pass
        
        return temp, vram_usage
    
    def temp_to_color(self, temp, min_temp=30, max_temp=90):
        """Convert temperature to RGB565 color"""
        temp = max(min_temp, min(max_temp, temp))
        ratio = (temp - min_temp) / (max_temp - min_temp)
        
        if ratio < 0.33:  # Blue to Green
            r = 0
            g = int(255 * (ratio / 0.33))
            b = int(255 * (1 - ratio / 0.33))
        elif ratio < 0.66:  # Green to Yellow
            r = int(255 * ((ratio - 0.33) / 0.33))
            g = 255
            b = 0
        else:  # Yellow to Red
            r = 255
            g = int(255 * (1 - (ratio - 0.66) / 0.34))
            b = 0
        
        # Convert to RGB565
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    
    def get_system_data(self):
        """Collect all system data"""
        # CPU
        cpu_usage = psutil.cpu_percent(interval=0.1)
        cpu_temp = self.get_cpu_temp()
        cpu_color = self.temp_to_color(cpu_temp)
        
        # RAM
        ram = psutil.virtual_memory()
        ram_usage = ram.percent
        
        # GPU and VRAM
        gpu_temp, vram_usage = self.get_gpu_info()
        gpu_color = self.temp_to_color(gpu_temp)
        vram_color = gpu_color  # Same as GPU
        
        # Estimate GPU usage from VRAM (rough approximation)
        gpu_usage = vram_usage
        
        # Disk I/O
        current_disk_io = psutil.disk_io_counters()
        current_time = time.time()
        time_delta = current_time - self.last_time
        
        if time_delta > 0:
            disk_read_speed = (current_disk_io.read_bytes - self.last_disk_io.read_bytes) / time_delta / (1024 * 1024)
            disk_write_speed = (current_disk_io.write_bytes - self.last_disk_io.write_bytes) / time_delta / (1024 * 1024)
        else:
            disk_read_speed = 0.0
            disk_write_speed = 0.0
        
        self.last_disk_io = current_disk_io
        
        # Network I/O
        current_net_io = psutil.net_io_counters()
        
        if time_delta > 0:
            net_up_speed = (current_net_io.bytes_sent - self.last_net_io.bytes_sent) / time_delta / (1024 * 1024)
            net_down_speed = (current_net_io.bytes_recv - self.last_net_io.bytes_recv) / time_delta / (1024 * 1024)
        else:
            net_up_speed = 0.0
            net_down_speed = 0.0
        
        self.last_net_io = current_net_io
        self.last_time = current_time
        
        return {
            'cpu_usage': cpu_usage,
            'cpu_temp': cpu_temp,
            'cpu_color': cpu_color,
            'ram_usage': ram_usage,
            'gpu_usage': gpu_usage,
            'gpu_temp': gpu_temp,
            'gpu_color': gpu_color,
            'vram_usage': vram_usage,
            'vram_color': vram_color,
            'disk_read': disk_read_speed,
            'disk_write': disk_write_speed,
            'net_up': net_up_speed,
            'net_down': net_down_speed
        }
    
    def send_data(self, data):
        """Send data to Arduino"""
        try:
            # Convert all values to uint8 (0-99 range, clamped)
            cpu_usage = min(99, int(data['cpu_usage']))
            ram_usage = min(99, int(data['ram_usage']))
            gpu_usage = min(99, int(data['gpu_usage']))
            vram_usage = min(99, int(data['vram_usage']))
            disk_read = min(99, int(data['disk_read']))
            disk_write = min(99, int(data['disk_write']))
            net_up = min(99, int(data['net_up']))
            net_down = min(99, int(data['net_down']))
            
            # Pack as uint8 values with colors
            packet = struct.pack('<B H B B H B H B B B B',
                                cpu_usage, data['cpu_color'],
                                ram_usage,
                                gpu_usage, data['gpu_color'],
                                vram_usage, data['vram_color'],
                                disk_read, disk_write,
                                net_up, net_down)
            
            self.serial_port.write(b'\xFF\xFF')  # Start marker
            self.serial_port.write(packet)
            return True
        except Exception as e:
            print(f"Error sending data: {e}")
            return False
    
    def print_data(self, data):
        """Print data to console"""
        print(f"\rCPU: {data['cpu_usage']:5.1f}% ({data['cpu_temp']:4.1f}°C) | "
              f"RAM: {data['ram_usage']:5.1f}% | "
              f"GPU: {data['gpu_usage']:5.1f}% ({data['gpu_temp']:4.1f}°C) | "
              f"VRAM: {data['vram_usage']:5.1f}% | "
              f"Disk: R:{data['disk_read']:6.2f} W:{data['disk_write']:6.2f} MB/s | "
              f"Net: ↑{data['net_up']:6.2f} ↓{data['net_down']:6.2f} MB/s", end='')
        sys.stdout.flush()
    
    def run(self):
        """Main loop"""
        print("=" * 80)
        print("System Monitor - Starting...")
        print("=" * 80)
        
        if not self.connect_serial():
            print("\nFailed to connect to Arduino. Exiting...")
            return
        
        print("\nMonitoring started. Press Ctrl+C to stop.\n")
        
        try:
            while True:
                data = self.get_system_data()
                
                if self.send_data(data):
                    self.print_data(data)
                else:
                    print("\nConnection lost. Attempting to reconnect...")
                    self.serial_port.close()
                    time.sleep(2)
                    if not self.connect_serial():
                        print("Reconnection failed. Exiting...")
                        break
                
                time.sleep(UPDATE_INTERVAL)
                
        except KeyboardInterrupt:
            print("\n\nStopping monitor...")
        except Exception as e:
            print(f"\n\nError: {e}")
        finally:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
            print("Serial connection closed. Goodbye!")

def check_dependencies():
    """Check and install required packages"""
    required = ['psutil', 'pyserial']
    missing = []
    
    for package in required:
        try:
            __import__(package)
        except ImportError:
            missing.append(package)
    
    if missing:
        print(f"Missing required packages: {', '.join(missing)}")
        print(f"Install with: pip install {' '.join(missing)}")
        return False
    
    return True

if __name__ == "__main__":
    
    if not check_dependencies():
        sys.exit(1)
    
    monitor = SystemMonitor()
    monitor.run()
