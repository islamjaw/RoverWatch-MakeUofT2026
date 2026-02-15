import requests
import os
import time

# CONFIGURATION
# We talk to Flask (Port 5000), NOT React (Port 5173)
URL = 'http://192.168.123.78:5000/classify' 
IMAGE_FILE = 'bear.jpg' 

# In simulate.py
import time

def send_loop():
    while True:
        send_image()
        print("Waiting 5 seconds to respect API quota...")
        time.sleep(5) # This prevents the 429 error

def send_image():
    # 1. Check if the test image exists
    if not os.path.exists(IMAGE_FILE):
        print(f"❌ ERROR: Could not find '{IMAGE_FILE}' in the backend folder.")
        print("   -> Go to Google Images, download a fruit/plant, and save it as 'test.jpg' here.")
        return

    print(f"--- 📡 SIMULATING HEADBAND TRANSMISSION ---")
    print(f"Target: {URL}")
    print(f"Payload: {IMAGE_FILE}")

    try:
        # 2. Open the image in binary mode
        with open(IMAGE_FILE, 'rb') as img:
            # 3. Create the payload (mimics a form upload)
            files = {'image': img}
            
            # 4. Send the POST request
            print("Sending... ", end="", flush=True)
            response = requests.post(URL, files=files)
            
            # 5. Print the result
            if response.status_code == 200:
                print("✅ SUCCESS!")
                print(f"Gemini Analysis: {response.json().get('analysis')}")
                print("Check your React Website now - it should have updated!")
            else:
                print(f"❌ FAILED (Status {response.status_code})")
                print(response.text)

    except Exception as e:
        print(f"\n❌ CONNECTION ERROR: {e}")
        print("   -> Is 'app.py' running in another terminal?")

if __name__ == "__main__":
    send_image()