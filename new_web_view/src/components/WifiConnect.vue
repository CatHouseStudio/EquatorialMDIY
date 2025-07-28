<template>
  <div class="wifi-page">
    <h1>设备连接向导</h1>
    <p>请连接 WiFi：<strong>ESP32-AP</strong></p>
    <p>然后点击下面的按钮测试连接设备</p>
    <button @click="checkDevice">测试连接</button>
    <p v-if="connected">✅ 已连接设备！</p>
    <p v-else-if="checked">❌ 无法连接，请确认 WiFi 是否连接。</p>
  </div>
</template>

<script setup>
import { ref } from 'vue'

const connected = ref(false)
const checked = ref(false)

const checkDevice = async () => {
  try {
    const res = await fetch('http://192.168.4.1/status')
    connected.value = res.ok
  } catch {
    connected.value = false
  } finally {
    checked.value = true
  }
}
</script>