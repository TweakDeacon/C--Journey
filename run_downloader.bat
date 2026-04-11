@echo off
echo Installing dependencies...
pip install cloudscraper beautifulsoup4 --quiet

echo.
echo Downloading Creepypasta wiki...
echo.
python fandom_downloader.py creepypasta

echo.
echo ----------------------------------------
echo Done! Press any key to close.
echo ----------------------------------------
pause
