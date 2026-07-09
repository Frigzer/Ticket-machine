# Ticket Machine

A C++23 client-server ticket machine simulation. The project models ticket availability, reservations, purchases, customer data, coin-based payment and change calculation. It contains a console client, a TCP server and reusable domain logic packaged as a CMake library.

## Table of Contents

* [Overview](#overview)
* [Features](#features)
* [Tech Stack](#tech-stack)
* [Project Structure](#project-structure)
* [Requirements](#requirements)
* [Build](#build)
* [Run](#run)
* [Client Commands](#client-commands)
* [Data Files](#data-files)
* [Configuration](#configuration)
* [Troubleshooting](#troubleshooting)

## Overview

Ticket Machine is split into two executables:

* `ticket-machine-server` — loads ticket and cashbox data from JSON, starts a local TCP server and handles ticket operations.
* `ticket-machine` — interactive console client used to list tickets, reserve a ticket, finalize a purchase or cancel a reservation.

The server keeps track of ticket states and active reservations. Purchases are finalized only when the reservation is valid, the inserted coins cover the ticket price and the machine can return the required change from the available cashbox.

Money values are represented as integer grosz values. For example, `350` means `3.50 PLN`.

## Features

* Interactive console client.
* TCP-based client-server communication.
* JSON request/response protocol.
* Ticket listing grouped by type and price.
* Ticket reservation with timeout handling.
* Purchase finalization with customer data.
* Coin inventory management.
* Minimal coin change calculation with limited coin availability.
* Automatic refund on failed purchases.
* Seed data loaded from JSON files.
* CMake + Conan build setup.

## Tech Stack

* C++23
* CMake 3.25+
* Conan 2
* Asio
* nlohmann/json
* Ninja

## Project Structure

```text
.
├── app/
│   ├── main.cpp                # Client entry point
│   └── server_main.cpp         # Server entry point
├── data/
│   ├── server_seed.json        # Default server data
│   ├── 01_happy_path.json      # Standard purchase scenario
│   ├── 02_no_change_possible.json
│   ├── 03_last_ticket_race.json
│   └── 04_exact_payment_focus.json
├── include/
│   ├── change_calculator.hpp
│   ├── coin_inventory.hpp
│   ├── console_client_app.hpp
│   ├── models.hpp
│   ├── network_protocol.hpp
│   ├── paths.hpp
│   ├── server_console_app.hpp
│   ├── server_seed_data.hpp
│   ├── ticket_machine_client.hpp
│   ├── ticket_server.hpp
│   └── ticket_server_host.hpp
├── src/
│   ├── change_calculator.cpp
│   ├── coin_inventory.cpp
│   ├── console_client_app.cpp
│   ├── network_protocol.cpp
│   ├── server_console_app.cpp
│   ├── server_seed_data.cpp
│   ├── ticket_machine_client.cpp
│   ├── ticket_server.cpp
│   └── ticket_server_host.cpp
├── tests/
├── CMakeLists.txt
├── conanfile.py
└── README.md
```

## Requirements

Install the following tools before building the project:

* Conan 2
* CMake 3.25 or newer
* Ninja
* A C++23-capable compiler

On first Conan usage, create or refresh your local Conan profile:

```bash
conan profile detect --force
```

## Build

Install dependencies and generate CMake presets:

```bash
conan install . -s build_type=Debug -s compiler.cppstd=23 -c tools.cmake.cmaketoolchain:generator=Ninja --build=missing
```

Configure the project:

```bash
cmake --preset conan-debug
```

Build the project:

```bash
cmake --build --preset conan-debug
```

For a release build, use `Release` as the build type:

```bash
conan install . -s build_type=Release -s compiler.cppstd=23 -c tools.cmake.cmaketoolchain:generator=Ninja --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
```

## Run

Start the server in one terminal:

```bash
./build/Debug/ticket-machine-server --port 5555 --data server_seed.json
```

On Windows, the executable will usually have the `.exe` extension:

```powershell
.\build\Debug\ticket-machine-server.exe --port 5555 --data server_seed.json
```

Start the client in another terminal:

```bash
./build/Debug/ticket-machine --host 127.0.0.1 --port 5555
```

On Windows:

```powershell
.\build\Debug\ticket-machine.exe --host 127.0.0.1 --port 5555
```

Then use the interactive prompt:

```text
list
reserve normal
status
buy
cancel
quit
```

## Client Commands

| Command          | Description                                                                            |
| ---------------- | -------------------------------------------------------------------------------------- |
| `help`           | Show available client commands.                                                        |
| `list`           | Show available ticket types, prices and counts.                                        |
| `reserve <type>` | Reserve one ticket of the selected type, for example `reserve normal`.                 |
| `status`         | Show the currently active reservation.                                                 |
| `buy`            | Finalize the active reservation. The client asks for customer data and inserted coins. |
| `cancel`         | Cancel the active reservation.                                                         |
| `quit` / `exit`  | Close the client.                                                                      |

Supported coin denominations are:

```text
1, 2, 5, 10, 20, 50, 100, 200, 500
```

All denominations are given in grosz.

## Data Files

The server loads initial tickets and cashbox state from JSON files located in the `data/` directory.

Default file:

```text
data/server_seed.json
```

Available sample scenarios:

| File                          | Purpose                                                           |
| ----------------------------- | ----------------------------------------------------------------- |
| `server_seed.json`            | Default server state with normal and reduced tickets.             |
| `01_happy_path.json`          | Basic successful purchase scenario.                               |
| `02_no_change_possible.json`  | Scenario where the cashbox may not be able to return change.      |
| `03_last_ticket_race.json`    | Scenario focused on a single remaining ticket.                    |
| `04_exact_payment_focus.json` | Scenario with an empty cashbox, useful for exact payment testing. |

Example:

```bash
./build/Debug/ticket-machine-server --data 02_no_change_possible.json
```

Relative paths passed to `--data` are resolved inside the `data/` directory. Absolute paths can also be used.

A seed file has the following shape:

```json
{
  "tickets": [
    { "id": 1, "price": 350, "type": "normal", "status": "available" },
    { "id": 2, "price": 170, "type": "reduced", "status": "available" }
  ],
  "cashbox": [
    { "denomination": 200, "count": 1 },
    { "denomination": 100, "count": 5 }
  ]
}
```

Ticket statuses supported by the seed loader:

```text
available, reserved, sold
```

## Configuration

The server accepts:

```bash
ticket-machine-server [--port PORT] [--data FILE_OR_PATH]
```

The client accepts:

```bash
ticket-machine [--host HOST] [--port PORT]
```

Default values:

| Setting          | Default                 |
| ---------------- | ----------------------- |
| Server port      | `5555`                  |
| Client host      | `127.0.0.1`             |
| Client port      | `5555`                  |
| Server data file | `data/server_seed.json` |

The CMake project exposes these options:

| Option                        | Default | Description                                   |
| ----------------------------- | ------- | --------------------------------------------- |
| `TICKET_MACHINE_BUILD_CLIENT` | `ON`    | Build the `ticket-machine` executable.        |
| `TICKET_MACHINE_BUILD_SERVER` | `ON`    | Build the `ticket-machine-server` executable. |

Example: build only the server:

```bash
cmake --preset conan-debug -DTICKET_MACHINE_BUILD_CLIENT=OFF
cmake --build --preset conan-debug
```

## Troubleshooting

### `Preset "conan-debug" not found`

Run `conan install` first. Conan generates the CMake presets used by the build commands.

### `Ninja` is not found

Install Ninja or change the CMake generator passed to Conan:

```bash
-c tools.cmake.cmaketoolchain:generator=Ninja
```

### The project is built with the wrong C++ standard

Make sure the Conan install command includes:

```bash
-s compiler.cppstd=23
```

The project requires a C++23-capable compiler.

### The client cannot connect to the server

Check that the server is running and that both programs use the same port:

```bash
ticket-machine-server --port 5555
ticket-machine --host 127.0.0.1 --port 5555
```

### The server cannot open the data file

Use a filename from the `data/` directory, for example:

```bash
ticket-machine-server --data server_seed.json
```

or pass an absolute path to a custom JSON file.
