from flask import Flask, render_template, jsonify, request, Response
from flask_socketio import SocketIO
import os
import re 
from collections import defaultdict

app = Flask(__name__)
socketio = SocketIO(app)

# Path to the firmware folder, file, and version file
FIRMWARE_FOLDER = 'uploads/'
FIRMWARE_FILE = os.path.join(FIRMWARE_FOLDER, 'firmware.bin')
VERSION_FILE = os.path.join(FIRMWARE_FOLDER, 'version.txt')

# Initialize version file if it doesn't exist
if not os.path.exists(FIRMWARE_FOLDER):
    os.makedirs(FIRMWARE_FOLDER)
if not os.path.exists(VERSION_FILE):
    with open(VERSION_FILE, 'w') as vf:
        vf.write('1.0.0')  # Starting version

# Dictionary to track connected devices and download progress
connected_boards = defaultdict(lambda: {'connected': False, 'progress': 0})

# Function to read the firmware version
def read_firmware_version():
    with open(VERSION_FILE, 'r') as vf:
        return vf.read().strip()

# Function to update the firmware version
def update_firmware_version():
    version = read_firmware_version()
    major, minor, patch = map(int, version.split('.'))
    patch += 1  # Increment patch version
    new_version = f"{major}.{minor}.{patch}"
    with open(VERSION_FILE, 'w') as vf:
        vf.write(new_version)
    return new_version

# Serve the dashboard HTML page
@app.route('/')
def index():
    return render_template('dashboard.html')  # Dashboard HTML

# API to fetch firmware details like version, program ID, etc.
@app.route('/api/firmware_info', methods=['GET'])
def get_firmware_info():
    firmware_info = {
        'firmware_id': read_firmware_version(),  # Fetch version from file
        'program_id': '12345',  # Example
        'size': os.path.getsize(FIRMWARE_FILE) if os.path.exists(FIRMWARE_FILE) else 0
    }
    return jsonify(firmware_info)

def increment_version():
    # Read current version
    if os.path.exists(VERSION_FILE):
        with open(VERSION_FILE, 'r') as vf:
            current_version = vf.read().strip()
    else:
        current_version = "1.0.0"  # Starting version if no version file exists

    # Split version into major, minor, and patch parts
    major, minor, patch = map(int, current_version.split('.'))
    
    # Increment the version, e.g., updating the patch number
    patch += 1
    if patch >= 10:  # Example: reset patch and increment minor version
        patch = 0
        minor += 1
        if minor >= 10:  # Example: reset minor and increment major version
            minor = 0
            major += 1

    # Construct new version string
    new_version = f"{major}.{minor}.{patch}"
    
    # Write updated version back to the version file
    with open(VERSION_FILE, 'w') as vf:
        vf.write(new_version)
    
    return new_version

# API to upload new firmware to the server
@app.route('/api/upload', methods=['POST'])
def upload_firmware():
    if 'file' not in request.files:
        return jsonify({'error': 'No file part'}), 400

    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No selected file'}), 400

    if not os.path.exists(FIRMWARE_FOLDER):
        os.makedirs(FIRMWARE_FOLDER)

    file.save(os.path.join(FIRMWARE_FOLDER, 'firmware.bin'))
    
    # Update firmware version
    new_version = increment_version()
    
    return jsonify({'success': 'Firmware uploaded successfully', 'new_version': new_version})

@app.route('/OTA/firmware', methods=['GET'])
def serve_firmware():
    board_ip = request.remote_addr
    connected_boards[board_ip]['connected'] = True
    update_board_count()

    if not os.path.exists(FIRMWARE_FILE):
        return jsonify({'error': 'Firmware file not found'}), 404

    file_size = os.path.getsize(FIRMWARE_FILE)
    range_header = request.headers.get('Range', None)

    if range_header:
        range_match = re.search(r'bytes=(\d+)-(\d+)?', range_header)
        if range_match:
            start = int(range_match.group(1))
            end = range_match.group(2)

            if end:
                end = int(end)
            else:
                end = start + 1024 - 1  # Default chunk size

            end = min(end, file_size - 1)

            if start > end or start >= file_size:
                return Response('Range Not Satisfiable', status=416)

            with open(FIRMWARE_FILE, 'rb') as firmware:
                firmware.seek(start)
                chunk_data = firmware.read(end - start + 1)

            response = Response(chunk_data, 206, content_type='application/octet-stream')
            response.headers['Content-Range'] = f'bytes {start}-{end}/{file_size}'
            response.headers['Accept-Ranges'] = 'bytes'
            response.headers['Content-Length'] = str(end - start + 1)

            # Calculate and emit the download percentage
            download_percentage = int((end + 1) / file_size * 100)
            connected_boards[board_ip]['progress'] = download_percentage
            socketio.emit('download_progress', {'ip': board_ip, 'progress': download_percentage})

            return response

    with open(FIRMWARE_FILE, 'rb') as firmware:
        firmware_data = firmware.read()
    return Response(firmware_data, content_type='application/octet-stream')

@app.route('/notify_board_connected', methods=['GET'])
def handle_board_connect():
    board_ip = request.remote_addr
    connected_boards[board_ip]['connected'] = True
    update_board_count()
    return jsonify({'status': 'Board connected successfully'})

@socketio.on('connect')
def handle_dashboard_connect():
    print("Dashboard connected")

@socketio.on('disconnect')
def handle_dashboard_disconnect():
    print("Dashboard disconnected")

def update_board_count():
    connected_count = sum(1 for board in connected_boards.values() if board['connected'])
    socketio.emit('status', {'message': f'{connected_count} boards connected.', 'connected_boards': connected_count})

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
