# 019_OLED_News_Ticker

This project displays the latest news headlines on an OLED screen using the New York Times API. Follow the steps below to configure and run the project.

---

## Prerequisites

1. An OLED display (compatible with the project setup).
2. ESP wifi module.
3. Access to [The New York Times Developer Portal](https://developer.nytimes.com/).

---

## Setup Instructions

### Step 1: Sign Up/Login to NYT Developer Portal
1. Go to [The New York Times Developer Portal](https://developer.nytimes.com/).
2. Sign up or log in to your account.

### Step 2: Create a New App
1. Navigate to the **Apps** section.
2. Click on **New App**.
3. Provide a name for your app (e.g., `OLED_News_Ticker`) and enable the required APIs.

### Step 3: Obtain the API Key
1. After creating the app, copy the API key provided.
2. Save this key securely as you will need it for the project.

### Step 4: Select and Configure APIs
1. In the API section, select the required API(s) based on the data you want to display.
2. Copy the endpoint(s) for the selected APIs.

### Step 5: Configure the Code
1. Open the `main.h` file in the project codebase.
2. Enter your Wi-Fi SSID and password:
   ```c
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   #define API_KEY "YourNYTApiKey"
   
   
##After powering on the device and establishing the Wi-Fi connection:

	Single Click: Scroll down the menu to browse through topics.
	Long Press  : Select the highlighted topic to display headlines related to it.
   
