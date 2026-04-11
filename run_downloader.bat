@echo off
echo Installing dependencies...
pip install requests beautifulsoup4 --quiet

echo.
echo Downloading Analog Horror wiki...
python fandom_downloader.py analog-horror-0

echo.
pause
