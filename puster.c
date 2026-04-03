#include <stdio.h>    // Librería estándar de entrada y salida
#include <stdlib.h>   // Para funciones de sistema y gestión de memoria
#include <string.h>   // Para manipulación de cadenas de texto
#include <unistd.h>   // Para funciones de API de Linux (como sleep o acceso a archivos)

// Definición de Colores ANSI
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

// Función para mostrar el banner del proyecto
void mostrar_banner() {
    printf("%s\n", CYAN);
    printf(" ____  _   _ ____ _____ _____ ____  \n");
    printf("|  _ \\| | | / ___|_   _| ____|  _ \\ \n");
    printf("| |_) | | | \\___ \\ | | |  _| | |_) |\n");
    printf("|  __/| |_| |___) || | | |___|  _ < \n");
    printf("|_|    \\___/|____/ |_| |_____|_| \\_\\\n");
    printf("       %sLinux Monitoring Tool%s\n", MAGENTA, RESET);
    printf("        %sOrami InfoSec 2026%s\n\n", YELLOW, RESET);
}

// Función para pausar la ejecución y permitir que el usuario lea la salida
void pausar() {
    printf("\n%sPresiona Enter para continuar...%s", YELLOW, RESET);
    getchar(); // Captura el carácter sobrante del buffer
}

// Función para registrar eventos en un archivo de texto externo
void registrar_log(const char *mensaje) {
    // Se abre en modo "a" (append) para añadir al final sin borrar lo anterior
    FILE *log = fopen("monitor_log.txt", "a");
    if (log == NULL) {
        perror("Error al abrir el archivo de log"); // Muestra el error del sistema
        return;
    }
    fprintf(log, "%s\n", mensaje); // Escribe el mensaje en el archivo
    fclose(log); // Siempre cerrar el flujo para evitar fugas de memoria
}

// Ejecuta el comando 'ps' para listar procesos, limitado a los 10 principales
void mostrar_procesos() {
    printf("%s--- Procesos activos (Top 10) ---%s\n", GREEN, RESET);
    // Ordenado por uso de CPU, limitando cada línea a 100 caracteres
    system("ps aux --sort=-%cpu | head -n 11 | cut -c 1-100");  
    registrar_log("[INFO] Consulta de procesos realizada.");
}

// Muestra el espacio disponible en las particiones de disco físicas
void mostrar_disco() {
    printf("%s--- Uso de disco ---%s\n", GREEN, RESET);
    system("df -h -x tmpfs -x devtmpfs"); // Filtra sistemas de archivos virtuales
    registrar_log("[INFO] Consulta de disco realizada.");
}

// Función crítica de seguridad: detecta conexiones de red activas
void detectar_remoto() {
    printf("%s--- Detección de conexiones externas (ss) --- %s\n", YELLOW, RESET);
    
    // Se usa 'ss' en lugar de 'netstat' por ser el estándar moderno en Linux
    // popen abre un proceso y nos permite leer su salida como un archivo
    FILE *fp = popen("ss -tunp | grep -v '127.0.0.1' | grep -v '::1'", "r");
    if (fp == NULL) {
        printf("%sError al ejecutar ss.%s\n", RED, RESET);
        return;
    }

    char buffer[1024];
    int alerta = 0;
    // Se lee la salida del comando línea por línea
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        alerta = 1;
        printf("%s[ALERTA] Conexión detectada:%s %s", RED, RESET, buffer);
        registrar_log("[ALERTA] Conexión detectada:");
        registrar_log(buffer);
    }

    if (!alerta) {
        printf("%sNo hay conexiones externas sospechosas.%s\n", GREEN, RESET);
        registrar_log("[INFO] Escaneo de red limpio.");
    }
    pclose(fp); // Se cierra el proceso abierto por popen
}

// Función para visualizar el historial de eventos guardados
void ver_log() {
    printf("%s--- Historial del Sistema (monitor_log.txt) --- %s\n", BLUE, RESET);
    // Se verifica si el archivo existe antes de intentar leerlo
    if (access("monitor_log.txt", F_OK) != -1) {
        system("tail -n 20 monitor_log.txt"); // Muestra solo las últimas 20 líneas
    } else {
        printf("El archivo de log aún no existe.\n");
    }
}

int main() {
    char entrada[10]; // Buffer para leer la opción del menú
    int opcion = 0;

    do {
        system("clear"); // Limpia la terminal en cada ciclo del menú
        mostrar_banner();
        printf("%sMenú Principal:%s\n", MAGENTA, RESET);
        printf("1. Ver procesos activos\n");
        printf("2. Ver uso de disco\n");
        printf("3. Detección de red (Ciberseguridad)\n");
        printf("4. Ver historial (Log)\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");

        // Forma más segura de leer entrada para evitar crashes por caracteres no numéricos
        if (fgets(entrada, sizeof(entrada), stdin)) {
            opcion = atoi(entrada); // Convierte la cadena a entero
        }

        switch (opcion) {
            case 1: mostrar_procesos(); break;
            case 2: mostrar_disco(); break;
            case 3: detectar_remoto(); break;
            case 4: ver_log(); break;
            case 5: 
                printf("%sSaliendo... Hasta luego Hacker!%s\n", GREEN, RESET);
                registrar_log("[SISTEMA] Monitor finalizado por el usuario.");
                break;
            default: 
                printf("%sOpción no válida.%s\n", RED, RESET); 
                break;
        }

        if (opcion != 5) pausar();

    } while (opcion != 5);

    return 0;
}
