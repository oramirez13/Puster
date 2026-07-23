# Puster

Linux system monitoring tool written in C. A console-based application that displays active processes, disk usage, and remote network connections with logging support.

## Features

- Display top processes sorted by CPU usage
- Monitor disk usage for mounted partitions
- Detect active remote network connections
- Log all activity to file with timestamps
- Export system state to JSON
- Continuous monitoring mode with configurable interval
- Interactive menu with colored output

## Requirements

- GCC compiler
- Linux system with `ps`, `df`, and `ss` commands

## Installation

```bash
# Clone the repository
git clone <repository-url>
cd puster

# Build
make

# Optional: install to /usr/local/bin
sudo make install
```

### If make fails

You can compile manually:
```bash
gcc -o puster puster.c
```

## Usage

```bash
# Interactive menu mode
./puster

# Continuous monitoring (every 5 seconds)
./puster --watch 5

# Show help
./puster --help
```

## Menu Options

| Option | Description |
|--------|-------------|
| 1 | Show active processes (top 15 by CPU) |
| 2 | Show disk usage |
| 3 | Detect remote connections |
| 4 | View log file |
| 5 | Export data to JSON |
| 6 | Exit |

## Output Files

- `monitor_log.txt` - Activity log with timestamps
- `monitor_data.json` - System state export (created on demand)

## Project Structure

```
puster/
├── puster.c          # Source code
├── Makefile          # Build configuration
├── README.md         # Documentation
└── .gitignore
```
