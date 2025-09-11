; NSIS installer template for ACX
OutFile "acx-setup.exe"
InstallDir "$PROGRAMFILES\ACX"
Section
  SetOutPath "$INSTDIR"
  File /oname=acx.exe "build\acx.exe"
  CreateShortCut "$DESKTOP\ACX.lnk" "$INSTDIR\acx.exe"
SectionEnd
