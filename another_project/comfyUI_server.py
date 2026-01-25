import threading
import time
import json
import uuid
import urllib.request
import urllib.parse
from llama_cpp import Llama
import subprocess
import os

# --- CONFIGURATION ---
LLM_MODEL_PATH = "path/to/your/model.gguf" # Update this
COMFY_SERVER = "127.0.0.1:8188"
CLIENT_ID = str(uuid.uuid4())

def run_comfyui():
    """Starts ComfyUI in the background with 6GB VRAM optimization."""
    print("[ComfyUI] Starting background server...")
    # --lowvram or --normalvram is recommended for 6GB. 
    # --preview-method none saves a bit of VRAM and CPU.
    subprocess.Popen([
        "python", "main.py", 
        "--listen", "127.0.0.1", 
        "--port", "8188", 
        "--lowvram", 
        "--preview-method", "none",
        "--disable-auto-launch"
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def generate_text(prompt):
    """Generates text using CPU only."""
    print("[Llama] Loading model on CPU...")
    # n_gpu_layers=0 ensures it stays on CPU
    # n_threads=8 is a good balance for your 12-core CPU
    llm = Llama(model_path=LLM_MODEL_PATH, n_gpu_layers=0, n_threads=8)
    print(f"[Llama] Generating text for: {prompt}")
    output = llm(f"Q: {prompt} A: ", max_tokens=64, stop=["Q:", "\n"], echo=True)
    print("\n--- TEXT GENERATED ---")
    print(output['choices'][0]['text'])
    return output['choices'][0]['text']

def queue_image_generation(prompt_text):
    """Sends a prompt to the ComfyUI API."""
    # Load your workflow (Export API JSON from ComfyUI first)
    # Default simple workflow structure for example:
    workflow = {
        "3": {
            "inputs": {
                "seed": 42, "steps": 20, "cfg": 8, "sampler_name": "euler",
                "scheduler": "normal", "denoise": 1,
                "model": ["4", 0], "positive": ["6", 0], "negative": ["7", 0],
                "latent_image": ["5", 0]
            },
            "class_type": "KSampler"
        },
        "4": {"inputs": {"ckpt_name": "v1-5-pruned-emaonly.safetensors"}, "class_type": "CheckpointLoaderSimple"},
        "6": {"inputs": {"text": prompt_text, "clip": ["4", 1]}, "class_type": "CLIPTextEncode"},
        "7": {"inputs": {"text": "bad quality, blurry", "clip": ["4", 1]}, "class_type": "CLIPTextEncode"},
        "5": {"inputs": {"width": 512, "height": 512, "batch_size": 1}, "class_type": "EmptyLatentImage"},
        "8": {"inputs": {"samples": ["3", 0], "vae": ["4", 2]}, "class_type": "VAEDecode"},
        "9": {"inputs": {"filename_prefix": "ScriptGen", "images": ["8", 0]}, "class_type": "SaveImage"}
    }

    p = {"prompt": workflow, "client_id": CLIENT_ID}
    data = json.dumps(p).encode('utf-8')
    req = urllib.request.Request(f"http://{COMFY_SERVER}/prompt", data=data)
    print("[ComfyUI] Image queued.")
    urllib.request.urlopen(req)

if __name__ == "__main__":
    # 1. Start ComfyUI Server
    comfy_thread = threading.Thread(target=run_comfyui, daemon=True)
    comfy_thread.start()
    
    # Give the server a moment to warm up
    time.sleep(10) 

    # 2. Run both concurrently
    text_prompt = "Describe a futuristic city in one sentence."
    image_prompt = "A futuristic city with neon lights, high resolution"

    # We run the text gen in the main thread and queue the image
    # ComfyUI's internal queue will handle the GPU while Llama uses the CPU
    t1 = threading.Thread(target=generate_text, args=(text_prompt,))
    t2 = threading.Thread(target=queue_image_generation, args=(image_prompt,))

    t1.start()
    t2.start()

    t1.join()
    t2.join()
    
    print("\nTasks complete. Check ComfyUI/output for the image.")
