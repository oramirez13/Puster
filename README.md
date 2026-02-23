Linux Monitoring Tool - orami Cybersec 2025

Puster es una herramienta de monitoreo para sistemas Linux desarrollada en C que permite visualizar procesos activos, uso de disco, detectar conexiones remotas y registrar alertas en un archivo de log. Ideal para fines educativos, laboratorios de ciberseguridad y prácticas de programación en bajo nivel.

Características

* Visualización de procesos activos del sistema.
* Monitoreo del uso de disco.
* Detección de ejecución remota mediante conexiones de red activas.
* Registro automático de eventos en un archivo de log (`monitor_log.txt`).
* Interfaz en terminal con colores ANSI.
* Alerta de conexiones externas establecidas o sospechosas.

Requisitos

* Sistema operativo Linux
* Compilador GCC

Compilación

```bash
gcc puster.c -o puster
```

