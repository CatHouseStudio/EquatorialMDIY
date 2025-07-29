import { createApp } from 'vue'
import './main.css'
import App from './App.vue'
import {http, HttpResponse} from 'msw'

const handlers = [
  http.get('/api/get_ap_config', () => {
    return HttpResponse.json({
        status: 'success',
        data: {
            "ap_ssid": "ESP32-Access-Point",     // 设备WiFi-AP模式下的SSID
            "ap_password": "123456789",          // 设备WiFi-AP模式下的Password
    }
     
},
    { status: 200 })
  }),
    http.get('/api/get_ratio_config',  () => {
    return HttpResponse.json({
      status: 'success',
      data: {
        ratio_RA: 50,
        ratio_DEC: 50
      }
    }, { status: 200 })
  }),
];

if (process.env.NODE_ENV === 'development') {
  import('msw/browser').then(({ setupWorker }) => {
    const worker = setupWorker(...handlers);
    worker.start({
      onUnhandledRequest: 'bypass', // Ignore unhandled requests
    });
  });
}

createApp(App).mount('#app')

