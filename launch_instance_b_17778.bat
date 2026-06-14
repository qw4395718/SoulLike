@echo off
REM ============================================
REM Launch Packaged Instance B (Summoner/Host)
REM Listen server on port 17778
REM ============================================

start "" "D:\Code\SoulLike_Public\WindowsNoEditor\SoulLikeDemo.exe" ThirdPersonExampleMap?listen -game -Port=17778 -GamePort=17778 -log -ResX=1280 -ResY=720
