/*
 * Puster - Linux System Monitoring Tool
 *
 * A console-based monitoring tool for Linux that displays active processes,
 * disk usage, and remote network connections with logging support.
 *
 * Author: Orami
 * Version: 3.0
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>

/* ANSI color codes for terminal output */
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

/* Log file path */
#define LOG_FILE "monitor_log.txt"
#define JSON_FILE "monitor_data.json"

/*
 * Get current timestamp in "YYYY-MM-DD HH:MM:SS" format.
 * Uses localtime_r() for thread safety.
 */
void get_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_buf;
    localtime_r(&now, &tm_buf);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_buf);
}

/*
 * Append a message to the log file with timestamp.
 */
void log_message(const char *message)
{
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL)
        return;

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));
    fprintf(log, "[%s] %s\n", timestamp, message);
    fclose(log);
}

/*
 * Print a command output using popen() and log each line.
 * Returns 0 on success, -1 on failure.
 */
int run_command(const char *cmd, const char *header, const char *log_prefix)
{
    printf("%s--- %s ---%s\n", GREEN, header, RESET);

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("%sError executing command.%s\n", RED, RESET);
        log_message("[ERROR] Failed to execute command");
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    int count = 0;

    while (getline(&line, &len, fp) != -1) {
        printf("%s", line);
        count++;
    }

    free(line);
    pclose(fp);

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "[INFO] %s shown (%d lines)", log_prefix, count);
    log_message(log_msg);

    return 0;
}

/*
 * Display the application banner.
 */
void print_banner(void)
{
    printf("%s\n", CYAN);
    printf(" ____  _   _ ____ _____ _____ ____  \n");
    printf("|  _ \\| | | / ___|_   _| ____|  _ \\ \n");
    printf("| |_) | | | \\___ \\ | | |  _| | |_) |\n");
    printf("|  __/| |_| |___) || | | |___|  _ < \n");
    printf("|_|    \\___/|____/ |_| |_____|_| \\_\\\n");
    printf("       %sLinux Monitoring Tool 3.0%s\n", MAGENTA, RESET);
    printf("        %sOrami Infosec 2025%s\n\n", YELLOW, RESET);
}

/*
 * Wait for user to press Enter.
 */
void pause_screen(void)
{
    printf("\nPress Enter to continue...");
    getchar();
}

/*
 * Display active processes sorted by CPU usage.
 * Optional filter: if filter is not NULL, only show lines containing it.
 */
void show_processes(const char *filter)
{
    char cmd[256];
    if (filter && filter[0] != '\0')
        snprintf(cmd, sizeof(cmd), "ps aux --sort=-%%cpu | head -n 15 | grep -i '%s'", filter);
    else
        snprintf(cmd, sizeof(cmd), "ps aux --sort=-%%cpu | head -n 15");

    run_command(cmd, "Active Processes", "Active processes shown");
}

/*
 * Display disk usage for mounted partitions.
 */
void show_disk(void)
{
    run_command("df -h | grep '^/dev'", "Disk Usage", "Disk usage shown");
}

/*
 * Detect and display active remote network connections.
 * Excludes localhost (127.0.0.1 and ::1).
 */
void detect_remote_connections(void)
{
    printf("%s--- Remote Connection Detection ---%s\n", YELLOW, RESET);

    const char *cmd = "ss -tunp | grep ESTAB | grep -v 127.0.0.1 | grep -v '::1'";
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("%sError executing network command.%s\n", RED, RESET);
        log_message("[ERROR] Failed to execute ss");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    int alert_count = 0;

    while (getline(&line, &len, fp) != -1) {
        alert_count++;
        printf("%s[ALERT] External connection detected:%s %s", RED, RESET, line);
        log_message("[ALERT] External connection detected");
    }

    free(line);
    pclose(fp);

    if (alert_count == 0) {
        printf("%sNo external active connections detected.%s\n", GREEN, RESET);
        log_message("[INFO] No external active connections detected");
    } else {
        char summary[128];
        snprintf(summary, sizeof(summary),
                 "[INFO] Total external connections detected: %d", alert_count);
        log_message(summary);
    }
}

/*
 * Export current system state to a JSON file.
 */
void export_to_json(void)
{
    FILE *json = fopen(JSON_FILE, "w");
    if (json == NULL) {
        printf("%sError creating JSON file.%s\n", RED, RESET);
        return;
    }

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    fprintf(json, "{\n");
    fprintf(json, "  \"timestamp\": \"%s\",\n", timestamp);

    /* Export processes */
    fprintf(json, "  \"processes\": [\n");
    FILE *fp = popen("ps aux --sort=-%cpu | head -n 11 | tail -n +2", "r");
    if (fp != NULL) {
        char *line = NULL;
        size_t len = 0;
        int first = 1;
        while (getline(&line, &len, fp) != -1) {
            /* Remove trailing newline */
            size_t slen = strlen(line);
            if (slen > 0 && line[slen - 1] == '\n')
                line[slen - 1] = '\0';

            if (!first)
                fprintf(json, ",\n");
            fprintf(json, "    \"%s\"", line);
            first = 0;
        }
        free(line);
        pclose(fp);
    }
    fprintf(json, "\n  ],\n");

    /* Export connections */
    fprintf(json, "  \"connections\": [\n");
    fp = popen("ss -tunp | grep ESTAB | grep -v 127.0.0.1 | grep -v '::1'", "r");
    if (fp != NULL) {
        char *line = NULL;
        size_t len = 0;
        int first = 1;
        while (getline(&line, &len, fp) != -1) {
            size_t slen = strlen(line);
            if (slen > 0 && line[slen - 1] == '\n')
                line[slen - 1] = '\0';

            if (!first)
                fprintf(json, ",\n");
            fprintf(json, "    \"%s\"", line);
            first = 0;
        }
        free(line);
        pclose(fp);
    }
    fprintf(json, "\n  ]\n");
    fprintf(json, "}\n");

    fclose(json);

    char log_msg[128];
    snprintf(log_msg, sizeof(log_msg), "[INFO] Data exported to %s", JSON_FILE);
    log_message(log_msg);
    printf("%s[+] Data exported to %s%s\n", GREEN, JSON_FILE, RESET);
}

/*
 * Display the contents of the log file.
 */
void show_log(void)
{
    run_command("cat " LOG_FILE, "Log Contents", "Log contents shown");
}

/*
 * Display the main menu.
 */
void print_menu(void)
{
    printf("%sMenu:%s\n", MAGENTA, RESET);
    printf("1. Show active processes\n");
    printf("2. Show disk usage\n");
    printf("3. Detect remote connections\n");
    printf("4. View log\n");
    printf("5. Export data to JSON\n");
    printf("6. Exit\n");
    printf("Select an option: ");
}

/*
 * Main entry point.
 * Supports --watch <seconds> for continuous monitoring.
 */
int main(int argc, char *argv[])
{
    int watch_mode = 0;
    int watch_interval = 5;
    int opt;

    static struct option long_options[] = {
        {"watch",   required_argument, 0, 'w'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "w:h", long_options, NULL)) != -1) {
        switch (opt) {
        case 'w':
            watch_mode = 1;
            watch_interval = atoi(optarg);
            if (watch_interval < 1)
                watch_interval = 5;
            break;
        case 'h':
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("  -w, --watch <seconds>  Continuous monitoring mode\n");
            printf("  -h, --help             Show this help\n");
            return 0;
        default:
            fprintf(stderr, "Usage: %s [-w seconds] [-h]\n", argv[0]);
            return 1;
        }
    }

    /* Continuous monitoring mode */
    if (watch_mode) {
        printf("Monitoring every %d seconds (Ctrl+C to stop)...\n\n", watch_interval);
        while (1) {
            print_banner();
            show_processes(NULL);
            show_disk();
            detect_remote_connections();
            printf("\n--- Next scan in %d seconds ---\n\n", watch_interval);
            log_message("[INFO] Watch scan completed");
            sleep(watch_interval);
        }
        return 0;
    }

    /* Interactive menu mode */
    system("clear");
    int option;

    do {
        print_banner();
        print_menu();

        if (scanf("%d", &option) != 1) {
            printf("%sInvalid input.%s\n", RED, RESET);
            log_message("[WARNING] Invalid input detected");
            while (getchar() != '\n');
            pause_screen();
            continue;
        }

        while (getchar() != '\n');

        switch (option) {
        case 1:
            show_processes(NULL);
            break;
        case 2:
            show_disk();
            break;
        case 3:
            detect_remote_connections();
            break;
        case 4:
            show_log();
            break;
        case 5:
            export_to_json();
            break;
        case 6:
            printf("%sExiting monitor... Goodbye!%s\n", GREEN, RESET);
            log_message("[INFO] User exited monitor");
            break;
        default:
            printf("%sInvalid option.%s\n", RED, RESET);
            log_message("[WARNING] Invalid option selected");
            break;
        }

        pause_screen();

    } while (option != 6);

    return 0;
}
