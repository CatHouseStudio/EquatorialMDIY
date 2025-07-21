<template>
  <div>
    <h2>控制设备</h2>
    <button @click="turnOn">开启</button>
    <button @click="turnOff">关闭</button>
  </div>
</template>

<script setup>
const turnOn = () => fetch('http://192.168.4.1/control?power=on')
const turnOff = () => fetch('http://192.168.4.1/control?power=off')
</script>