import requests
from PIL import Image

# --- Configuration ---
ESP_IP = "192.168.1.108" 
PORT = 80
IMAGE_PATH = "send.png" 
TEXT_MESSAGE = "Status: High"

def convert_to_rgb565(r, g, b):
    """Convert RGB888 to RGB565 format"""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def send_image_and_text(image_path, text):
    """Send both image and text to ESP32 display"""
    img = Image.open(image_path).convert("RGB")
    img = img.resize((80, 80), Image.Resampling.LANCZOS)
    
    img_payload = bytearray()
    for y in range(80):
        for x in range(80):
            r, g, b = img.getpixel((x, y))
            rgb565 = convert_to_rgb565(r, g, b)
            img_payload.append((rgb565 >> 8) & 0xFF)
            img_payload.append(rgb565 & 0xFF)
    
    url = f"http://{ESP_IP}:{PORT}/upload"
    files = {'image': ('image.bin', bytes(img_payload), 'application/octet-stream')}
    data = {'text': text}
    
    response = requests.post(url, files=files, data=data, timeout=30)
    return response.status_code == 200

def send_text_only(text):
    """Send only text to ESP32 display (keeps existing image)"""
    url = f"http://{ESP_IP}:{PORT}/text"
    data = {'text': text}
    
    response = requests.post(url, data=data, timeout=10)
    return response.status_code == 200

if __name__ == "__main__":
    # Example 1: Send image with text
    if send_image_and_text(IMAGE_PATH, TEXT_MESSAGE):
        print("✓ Image and text sent")
    else:
        print("✗ Failed to send image and text")
    
    # Example 2: Update only text (after a delay)
    import time
    time.sleep(2)
    
    if send_text_only("Updated: OK"):
        print("✓ Text updated")
    else:
        print("✗ Failed to update text")
