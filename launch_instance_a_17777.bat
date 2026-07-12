@echo off
REM ============================================
REM Launch Packaged Instance A (Placer/Soul)
REM Listen server on port 17777
REM ============================================

start "" "D:\Code\SoulLike_Public\Windows\SoulLikeDemo.exe" ThirdPersonExampleMap?listen -game -Port=17777 -GamePort=17777 -log -ResX=1280 -ResY=720
