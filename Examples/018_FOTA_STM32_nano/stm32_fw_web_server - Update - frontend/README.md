# STM32 Firmware Server

This project provides a simple REST API and dashboard for managing firmware updates for STM32 boards. The server allows you to:
- Retrieve firmware and program IDs via HTTP requests.
- Upload and select `.bin` firmware files via a web dashboard.
- Track when a board connects or disconnects using WebSocket connections.

## Features

- **REST API**:
  - `/api/fw_id`: Get firmware ID.
  - `/api/program_id`: Get program ID.
  - `/api/firmware`: Get the current firmware `.bin`.

- **Dashboard**:
  - Upload `.bin` files.
  - View available firmware files.
  - Monitor connection status of STM32 boards.

## Installation

1. Clone the repository.
2. Install dependencies using `pip install -r requirements.txt`.
3. Run the application: `python app.py`.
4. Open `http://localhost:5000/dashboard` to view the web dashboard.

## API Endpoints

- **Firmware ID**: `GET /api/fw_id`
- **Program ID**: `GET /api/program_id`
- **Firmware Binary**: `GET /api/firmware`

## Dependencies

- Flask
- Flask-SocketIO
