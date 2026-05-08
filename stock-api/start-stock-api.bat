@echo off
setlocal
cd /d "%~dp0"
if not exist "node_modules\" (
  echo [stock-api] Installation des dependances npm...
  call npm.cmd install
  if errorlevel 1 exit /b 1
)
echo [stock-api] Demarrage ^(port dans .env, defaut 3000^)...
call npm.cmd start
