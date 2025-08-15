# bang-server

**Repository:** [bangsalvoserver/bang-server](https://github.com/bangsalvoserver/bang-server)  
**Language:** C++23  
**Visibility:** Public

## Overview

This is the backend server for [bang.salvoserver.it](https://bang.salvoserver.it), an online implementation of the game **Bang!** with all expansions. The server is written in modern C++ (C++23) with a focus on minimal dependencies and high performance.

## Features

- Online multiplayer game server for **Bang!** with all expansions
- Written in modern C++23
- Minimal external dependencies:
  - [libuv](https://github.com/libuv/libuv)
  - [uWebSockets](https://github.com/uNetworking/uWebSockets)
  - [cxxopts](https://github.com/jarro2783/cxxopts)
  - [nlohmann-json](https://github.com/nlohmann/json)
  - [libpng](http://www.libpng.org/pub/png/libpng.html)
- Some build steps require Python 3 with additional packages
- Designed for performance and scalability

## Getting Started

### Prerequisites

- Modern C++ compiler with C++23 support
- Dependencies:
  - libuv
  - uWebSockets
  - cxxopts
  - nlohmann-json
  - libpng
- Python 3 (for build/installation scripts)
  - [PyYAML](https://pyyaml.org/)
  - [Pillow](https://python-pillow.org/)
- [CMake](https://cmake.org/) (for building the project)

### Installation

1. Clone this repository:
    ```sh
    git clone https://github.com/bangsalvoserver/bang-server.git
    cd bang-server
    ```

2. Install the required C++ dependencies (refer to each library's documentation for installation).

    - **Note:** [uWebSockets](https://github.com/uNetworking/uWebSockets) requires [libuv](https://github.com/libuv/libuv) to be installed on your system.

3. Ensure Python 3 is installed, then install Python dependencies:
    ```sh
    pip install pyyaml pillow
    ```

4. Build the server using CMake:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

   The resulting binary (e.g. `bang-server`) will be placed in the `build` directory.

### Usage

After building, you can run the server with the following command-line options:

```text
Usage: bang-server [PORT] [OPTIONS]

Positional arguments:
  PORT                      Set the server port (default: 47654)

Options:
  --cheats                  Enable cheats
  -l, --logging <level>     Set logging verbosity (e.g., info, debug, warn, error)
  -r, --reuse-addr          Allow reusing the address/port (useful for quick restarts)
  -t, --tracking-db <file>  Path to the tracking database (SQLite file).  
                            This database tracks player count and lobby count over time.
                            The database will be created automatically if missing.
  -h, --help                Print this help message and exit

TLS/SSL options (if built with SSL support):
  -s, --secure              Enable TLS/SSL mode
  --cert <file>             Path to the SSL certificate file
  --key <file>              Path to the SSL private key file
```

**Examples:**
```sh
# Start the server with default options
./bang-server

# Start on a custom port with increased logging
./bang-server 12345 --logging debug

# Start with tracking database and cheats enabled
./bang-server --tracking-db tracking.sqlite --cheats

# Start with TLS/SSL enabled (if supported)
./bang-server 12345 --secure --cert server.crt --key server.key
```

## Tracking Database

If you specify a tracking database (via `--tracking-db <file>`), it must be a SQLite file.  
The server will record player count and lobby count over time for monitoring and analytics purposes.  
If the database file does not exist, it will be created automatically.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any proposed changes.

## Community

For questions or support, please open an issue in this repository or join our [Discord server](https://discord.gg/jhQS6ATkgU).

## Disclaimer

This project is an independent, fan-made implementation of the game **Bang!**  
All rights to the original game, its rules, names, and artwork are property of **dV Giochi**.  
We do **not** claim any rights to the game or its intellectual property.
