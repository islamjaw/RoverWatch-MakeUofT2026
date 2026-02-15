import google.generativeai as genai
import os

os.environ["GOOGLE_API_KEY"] = "AIzaSyBhz1aV6KGsInXzxjMDHtIjm5HSeXDcWro"
genai.configure(api_key=os.environ["GOOGLE_API_KEY"])

print("Listing available models...")
for m in genai.list_models():
    if 'generateContent' in m.supported_generation_methods:
        print(m.name)