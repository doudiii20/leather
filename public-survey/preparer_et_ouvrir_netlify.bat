@echo off
chcp 65001 >nul
set "HERE=%~dp0"
set "SRC=%HERE%..\Resources\satisfaction_avis_client.html"
set "DST=%HERE%satisfaction_avis_client.html"

if not exist "%SRC%" (
  echo ERREUR: fichier introuvable :
  echo   %SRC%
  echo Verifiez que ce .bat est dans leather\public-survey\
  pause
  exit /b 1
)

copy /Y "%SRC%" "%DST%" >nul
if errorlevel 1 (
  echo Copie impossible.
  pause
  exit /b 1
)

echo Fichier copie : %DST%
echo.
echo Etapes Netlify :
echo   1 - Si vous voyez "Deploy your first project", finissez l'inscription / connexion.
echo   2 - Ouvrez la page Drop : https://app.netlify.com/drop
echo       (ou Sites - Add new site - Deploy manually)
echo   3 - Glissez tout le dossier public-survey dans la zone prevue.
echo.
echo Ouverture du dossier + page Drop...
start "" "%HERE%"
start "" "https://app.netlify.com/drop"
exit /b 0
