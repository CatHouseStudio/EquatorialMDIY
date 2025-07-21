import { http, HttpResponse } from 'msw';

export const handlers = [
  http.get('/api/get_ap_config', (req, res, ctx) => {
    return res(
      ctx.status(200),
      ctx.json({
        status: 'success',
        data: {
            "ap_ssid":"ESP32-Access-Point",     // 设备WiFi-AP模式下的SSID
            "ap_password":"123456789",          // 设备WiFi-AP模式下的Password
},
      })
    );
  }),
    http.get('/api/get_ratio_config', (req, res, ctx) => {
    return res(
      ctx.status(200),
      ctx.json({
        status: 'success',
        data: {
         
            "ratio_RA":50,     // RA轴减速箱减速比
            "ratio_DEC":50    // DEC轴减速箱减速比  
        
        },
      })
    );
  }),
];