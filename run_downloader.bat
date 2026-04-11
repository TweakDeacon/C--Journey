@echo off
echo Installing dependencies...
pip install requests beautifulsoup4 --quiet

echo.
echo ----------------------------------------
echo  Fandom Wiki Downloader
echo ----------------------------------------
echo.
echo If the wiki is public, just press Enter for username and password.
echo If it is private, enter your Fandom login details.
echo.
set /p FANDOM_USER=Fandom username (or press Enter to skip):
set /p FANDOM_PASS=Fandom password (or press Enter to skip):

echo.
echo Downloading Analog Horror wiki...
echo.

if "%FANDOM_USER%"=="" (
    python fandom_downloader.py analog-horror-0
) else (
    python fandom_downloader.py analog-horror-0 --username %FANDOM_USER% --password %FANDOM_PASS%
)

echo.
echo ----------------------------------------
echo Done! Press any key to close.
echo ----------------------------------------
pause
