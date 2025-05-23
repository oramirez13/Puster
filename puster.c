#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Colores ANSI
#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

void mostrar_banner()
{
    printf("%s\n", CYAN);
    printf(" ____  _   _ ____ _____ _____ ____  \n");
    printf("|  _ \\| | | / ___|_   _| ____|  _ \\ \n");
    printf("| |_) | | | \\___ \\ | | |  _| | |_) |\n");
    printf("|  __/| |_| |___) || | | |___|  _ < \n");
    printf("|_|    \\___/|____/ |_| |_____|_| \\_\\\n");
    printf("       %sLinux Monitoring Tool%s\n", MAGENTA, RESET);
    printf("        %sOrami Cybersec 2025%s\n\n", YELLOW, RESET);
}

void pausar()
{
    printf("\nPresiona Enter para continuar...");
    getchar();
}

void registrar_log(const char *mensaje)
{
    FILE *log = fopen("monitor_log.txt", "a");
    if (log != NULL)
    {
        fprintf(log, "%s\n", mensaje);
        fclose(log);
    }
}

void mostrar_procesos()
{
    printf("%s--- Procesos activos ---%s\n", GREEN, RESET);
    system("ps aux | head -n 10");
    registrar_log("[INFO] Mostrando procesos activos");
}

void mostrar_disco()
{
    printf("%s--- Uso de disco ---%s\n", GREEN, RESET);
    system("df -h | grep '^/dev'");
    registrar_log("[INFO] Mostrando uso de disco");
}

void detectar_remoto()
{
    printf("%s--- Detección de ejecución remota --- %s\n", YELLOW, RESET);
    system("who");
    FILE *fp = popen("netstat -tunp | grep -v 127.0.0.1 | grep -v '::1'", "r");
    if (fp == NULL)
        return;

    char buffer[1024];
    int alerta = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        alerta = 1;
        printf("%s[ALERTA] Conexión externa detectada:%s %s", RED, RESET, buffer);
        registrar_log("[ALERTA] Conexión externa detectada");
        registrar_log(buffer);
    }
    if (!alerta)
    {
        printf("%sNo se detectaron conexiones externas sospechosas.%s\n", GREEN, RESET);
        registrar_log("[INFO] No se detectaron conexiones externas sospechosas");
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
    system("clear"); // ← limpia pantalla antes de iniciar
    int opcion;
    char c;
    do
    {
        mostrar_banner();
        printf("%sMen\u00fa:%s\n", MAGENTA, RESET);
        printf("1. Ver procesos activos\n");
        printf("2. Ver uso de disco\n");
        printf("3. Detección de ejecución remota\n");
        printf("4. Ver log\n");
        printf("5. Salir\n");
        printf("Seleccione una opci\u00f3n: ");
        scanf("%d", &opcion);
        while ((c = getchar()) != '\n' && c != EOF)
            ; // limpiar buffer

        switch (opcion)
        {
        case 1:
            mostrar_procesos();
            break;
        case 2:
            mostrar_disco();
            break;
        case 3:
            detectar_remoto();
            break;
        case 4:
            ver_log();
            break;
        case 5:
            printf("%sSaliendo del monitor... \u00a1Hasta luego!%s\n", GREEN, RESET);
            registrar_log("[INFO] Usuario sali\u00f3 del monitor");
            break;
        default:
            printf("%sOpci\u00f3n inv\u00e1lida.%s\n", RED, RESET);
            break;
        }
        pausar();
    } while (opcion != 5);

    return 0;
}
