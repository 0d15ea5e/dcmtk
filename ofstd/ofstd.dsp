# Microsoft Developer Studio Project File - Name="ofstd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ofstd - Win32 Debug
!MESSAGE Dies ist kein g�ltiges Makefile. Zum Erstellen dieses Projekts mit\
 NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und f�hren Sie den\
 Befehl
!MESSAGE 
!MESSAGE NMAKE /f "ofstd.mak".
!MESSAGE 
!MESSAGE Sie k�nnen beim Ausf�hren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "ofstd.mak" CFG="ofstd - Win32 Debug"
!MESSAGE 
!MESSAGE F�r die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "ofstd - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "ofstd - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "ofstd - Win32 Release OpenSSL" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "ofstd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "..\config\include" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_REENTRANT" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ofstd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "include" /I "..\config\include" /D "_WINDOWS" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_REENTRANT" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ofstd - Win32 Release OpenSSL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OpenSSL"
# PROP BASE Intermediate_Dir "OpenSSL"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OpenSSL"
# PROP Intermediate_Dir "OpenSSL"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "include" /I "..\config\include" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_REENTRANT" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /I "..\config\include" /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "_REENTRANT" /D "WITH_OPENSSL" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ofstd - Win32 Release"
# Name "ofstd - Win32 Debug"
# Name "ofstd - Win32 Release OpenSSL"
# Begin Source File

SOURCE=.\libsrc\ofcmdln.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofconapp.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofcond.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofconsol.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofcrc32.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofdate.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofdatime.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\offname.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\oflist.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\oflogfil.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofstd.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofstring.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\ofthread.cxx
# End Source File
# Begin Source File

SOURCE=.\libsrc\oftime.cxx
# End Source File
# End Target
# End Project
