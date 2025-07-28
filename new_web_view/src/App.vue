<template>
  <div class="min-h-screen bg-neutral-900 text-white p-4 space-y-6">
    <!-- 顶部栏 -->
    <div class="flex justify-between items-center mt-1">
      <h1 class="text-xl font-bold">Wi-Fi コントロールパネル</h1>
      <button @click="toggleSettings" class="hover:text-white text-neutral-400">
        <svg class="w-6 h-6" fill="none" stroke="currentColor"
             stroke-width="2" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round"
                d="M12 4v1m0 14v1m8.66-8.66l-.707.707M4.34 4.34l-.707.707M4 12H3m18 0h1M4.34 19.66l.707-.707M19.66 4.34l.707-.707" />
        </svg>
      </button>
    </div>

    <!-- 坐标输入与定位按钮 -->
    <div class="flex flex-wrap items-center gap-3 mt-15">
      <input v-model="ra" placeholder="RA 赤经"
             class="bg-neutral-800 rounded px-3 py-2 w-40 placeholder:text-neutral-400" />
      <input v-model="dec" placeholder="DEC 赤纬"
             class="bg-neutral-800 rounded px-3 py-2 w-40 placeholder:text-neutral-400" />
      <button @click="locate"
              class="bg-blue-600 hover:bg-blue-700 text-white rounded px-4 py-2">
        定位
      </button>
    </div>

    <!-- Reset button -->
     <div class="flex ">
      <button @click="ra = ''; dec = ''"
              class="bg-red-600 hover:bg-red-700 text-white rounded px-4 py-2">
        重置  
      </button>
    </div>

    <!-- Tracking button -->
    <div>
      <button @click="track"
              class="bg-green-600 hover:bg-green-700 text-white rounded px-6 py-2">
        追踪
      </button>
    </div>

    <!-- 设置面板 -->
    <div v-if="showSettings" class="bg-neutral-800 rounded p-4 space-y-1">
      <h2 class="text-lg font-semibold">设备信息</h2>
      <div>RA 齿轮比：{{ deviceInfo.ra_ratio }}</div>
      <div>DEC 齿轮比：{{ deviceInfo.dec_ratio }}</div>
      <div>WiFi 名称：{{ deviceInfo.ssid }}</div>
      <div>WiFi 密码：{{ deviceInfo.password }}</div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import axios from 'axios'

const ra = ref('')
const dec = ref('')
const showSettings = ref(false)

const deviceInfo = ref({
  ra_ratio: 'Loading...',
  dec_ratio: 'Loading...',
  ssid: '-',
  password: '-'
})

const toggleSettings = () => {
  showSettings.value = !showSettings.value
}

const locate = async () => {
  try {
    await fetch('/api/locate', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ra: ra.value, dec: dec.value })
    })
  } catch (err) {
    console.error('定位失败', err)
  }
}

const track = async () => {
  try {
    await fetch('/api/start_tracking', { method: 'POST' })
  } catch (err) {
    console.error('追踪失败', err)
  }
}

const loadDeviceInfo = async () => {
 try {
    const [apRes, ratioRes] = await Promise.all([
      axios.get('/api/get_ap_config'),
      axios.get('/api/get_ratio_config')
    ])

   deviceInfo.ra_ratio = apRes.data.ratio_RA || '未知'
   deviceInfo.dec_ratio = apRes.data.ratio_DEC || '未知'
   deviceInfo.ssid = apRes.data.ap_ssid || '未知'
   deviceInfo.password = apRes.data.ap_password || '未知'
   
} catch (err) {
    console.error('加载设备信息失败', err)
      apConfig.value = { ssid: 'mock-ssid', password: 'mock-password' }
      ratioConfig.value = { ratio_RA: 'mock-ratio', ratio_DEC: 'mock-ratio' }
  }
}

onMounted(loadDeviceInfo)
</script>