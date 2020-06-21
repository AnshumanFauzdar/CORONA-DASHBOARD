# CORONA DASHBOARD - HackJaipur Submission

![Dashboard](https://raw.githubusercontent.com/AnshumanFauzdar/CORONA-DASHBOARD/master/Dashboard.png)
![Hardware](https://raw.githubusercontent.com/AnshumanFauzdar/CORONA-DASHBOARD/master/hardware.JPG)
# Hardware

- ESP8266 based microcontroller
  - Wemos D1 mini 
- BMP280 Sensor Breakout
- LED
- HC-SR04 Ultrasonic Sensor

More things can be added but during this lockdown scenario only these items are avaialable!

# Software

- Arduino IDE

Change your wifi credentials

![](https://raw.githubusercontent.com/AnshumanFauzdar/CORONA-DASHBOARD/master/wifi-creds.png)

Change your covid data API, for full information click [here](https://github.com/AnshumanFauzdar/COVID19-India-Counter)

![](https://raw.githubusercontent.com/AnshumanFauzdar/CORONA-DASHBOARD/master/covid-api.png)

You can change login credentials to the webpage!

```
// Web Server HTTP Authentication credentials
const char* http_username = "admin";
const char* http_password = "admin";
```

BMP is connected to ```D1 and D2```

LED is connected to pin ```D3```

Ultrasonic Sensor is connected to ```D6 and D7```

# How this can help you?

- Are you in a meeting and your family folks bump into you?
  - You can set indicator LED and tell them to look for it!
- Do you want to assure ambient temperature in your room?
  - This will assure that you maintain temperature which helps to avoid corona
- Do you repeatedly check total cases by visiting websites?
  - You can rapidly look all data in one place!
- You want to maintain social distancing?
  - Ultrasonic sensor helps to make sure that you are maintaing right distance!
  
### HackJaipur inspired us to make this jugaad! and this is our first hackathon!

![](https://raw.githubusercontent.com/AnshumanFauzdar/CORONA-DASHBOARD/master/India-logos.png)
