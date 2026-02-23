#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Colores ANSI
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

void obtener_timestamp(char *buffer, size_t size)
{
    time_t ahora = time(NULL);
    struct tm *t = localtime(&ahora);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void registrar_log(const char *mensaje)
{
    FILE *log = fopen("monitor_log.txt", "a");
    if (log != NULL)
    {
        char timestamp[64];
        obtener_timestamp(timestamp, sizeof(timestamp));
        fprintf(log, "[%s] %s\n", timestamp, mensaje);
        fclose(log);
    }
}

void mostrar_banner()
{
    printf("%s\n", CYAN);
    printf(" ____  _   _ ____ _____ _____ ____  \n");
    printf("|  _ \\| | | / ___|_   _| ____|  _ \\ \n");
    printf("| |_) | | | \\___ \\ | | |  _| | |_) |\n");
    printf("|  __/| |_| |___) || | | |___|  _ < \n");
    printf("|_|    \\___/|____/ |_| |_____|_| \\_\\\n");
    printf("       %sLinux Monitoring Tool 2.0%s\n", MAGENTA, RESET);
    printf("        %sOrami Infosec 2025%s\n\n", YELLOW, RESET);
}

void pausar()
{
    printf("\nPresiona Enter para continuar...");
    getchar();
}

void mostrar_procesos()
{
    printf("%s--- Procesos activos ---%s\n", GREEN, RESET);
    system("ps aux --sort=-%cpu | head -n 10");
    registrar_log("[INFO] Procesos activos mostrados");
}

void mostrar_disco()
{
    printf("%s--- Uso de disco ---%s\n", GREEN, RESET);
    system("df -h | grep '^/dev'");
    registrar_log("[INFO] Uso de disco mostrado");
}

void detectar_conexiones_remotas()
{
    printf("%s--- Detección de conexiones remotas --- %s\n", YELLOW, RESET);

    FILE *fp = popen("ss -tunp | grep ESTAB | grep -v 127.0.0.1 | grep -v '::1'", "r");
    if (fp == NULL)
    {
        printf("%sError al ejecutar comando de red.%s\n", RED, RESET);
        registrar_log("[ERROR] No se pudo ejecutar ss");
        return;
    }

    char buffer[1024];
    int alerta = 0;
    int contador = 0;

    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        alerta = 1;
        contador++;
        printf("%s[ALERTA] Conexión externa detectada:%s %s", RED, RESET, buffer);
        registrar_log("[ALERTA] Conexión externa detectada");
        registrar_log(buffer);
    }

    if (!alerta)
    {
        printf("%sNo se detectaron conexiones externas activas.%s\n", GREEN, RESET);
        registrar_log("[INFO] No se detectaron conexiones externas activas");
    }
    else
    {
        char resumen[128];
        snprintf(resumen, sizeof(resumen),
                 "[INFO] Total de conexiones externas detectadas: %d", contador);
        registrar_log(resumen);
    }

    pclose(fp);
}

void ver_log()
{
    printf("%s--- Contenido del Log --- %s\n", BLUE, RESET);
    system("cat monitor_log.txt");
}

int main()
{
    system("clear");
    int opcion;
    char c;

    do
    {
        mostrar_banner();

        printf("%sMenú:%s\n", MAGENTA, RESET);
        printf("1. Ver procesos activos\n");
        printf("2. Ver uso de disco\n");
        printf("3. Detectar conexiones remotas\n");
        printf("4. Ver log\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");

        if (scanf("%d", &opcion) != 1)
        {
            printf("%sEntrada inválida.%s\n", RED, RESET);
            registrar_log("[WARNING] Entrada inválida detectada");
            while ((c = getchar()) != '\n' && c != EOF);
            pausar();
            continue;
        }

        while ((c = getchar()) != '\n' && c != EOF);

        switch (opcion)
        {
        case 1:
            mostrar_procesos();
            break;
        case 2:
            mostrar_disco();
            break;
        case 3:
            detectar_conexiones_remotas();
            break;
        case 4:
            ver_log();
            break;
        case 5:
            printf("%sSaliendo del monitor... ¡Hasta luego!%s\n", GREEN, RESET);
            registrar_log("[INFO] Usuario salió del monitor");
            break;
        default:
            printf("%sOpción inválida.%s\n", RED, RESET);
            registrar_log("[WARNING] Opción inválida seleccionada");
            break;
        }

        pausar();

    } while (opcion != 5);

    return 0;
}
