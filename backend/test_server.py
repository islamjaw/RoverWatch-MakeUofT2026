import requests

# This URL points to your running Flask server
url = 'http://192.168.123.78:5000/classify'
image_path = 'test_pixelated.jpg' # Make sure this file exists!

print(f"Attempting to send {image_path} to {url}...")

try:
    # Open the image in binary mode
    with open(image_path, 'rb') as img_file:
        # Create the payload
        files = {'image': img_file}
        
        # Send POST request
        response = requests.post(url, files=files)
        
        # Print results
        print("\n--- SERVER RESPONSE ---")
        print(f"Status Code: {response.status_code}")
        print(f"Raw Output: {response.text}")

except FileNotFoundError:
    print(f"ERROR: Could not find '{image_path}'. Did you download a fruit image?")
except Exception as e:
    print(f"ERROR: Connection failed. Is the server running? Details: {e}")