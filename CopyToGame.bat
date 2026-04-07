@echo off
set "SOURCE=compilado\bin\GTAVImpulseTriggers.asi"
set "DEST=E:\SteamLibrary\steamapps\common\Grand Theft Auto V Enhanced\GTAVImpulseTriggers.asi"

if exist "%SOURCE%" (
    copy /Y "%SOURCE%" "%DEST%"
    echo Mod copiado com sucesso para: %DEST%
) else (
    echo ERRO: Arquivo compilado nao encontrado em %SOURCE%
    pause
)
