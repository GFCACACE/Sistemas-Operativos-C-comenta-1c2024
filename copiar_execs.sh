#!/bin/bash

# Definir los archivos a copiar
archivos_a_copiar=("exec" "hexec" "vexec" "tests" "makefile" "settings.mk")
archivos_vscode=("launch.json" "tasks.json")

# Definir las carpetas de destino
carpetas_destino=("memoria" "entradasalida" "kernel")

# Bucle para copiar los archivos a cada carpeta de destino
for archivo in "${archivos_a_copiar[@]}"; do
    for carpeta in "${carpetas_destino[@]}"; do
        cp "cpu/$archivo" "$carpeta/"
    done
done

# Bucle para copiar los archivos .vscode a cada carpeta de destino
for archivo_vscode in "${archivos_vscode[@]}"; do
    for carpeta in "${carpetas_destino[@]}"; do
        cp "memoria/.vscode/$archivo_vscode" "$carpeta/.vscode/"
    done
done

echo "Archivos copiados exitosamente a las carpetas destino."