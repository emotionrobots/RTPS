import socket
import json
import time

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

# Example Python object to send
data_object = {
    "username": "larrylisky",
    "score": 100,
    "active": True,
    "tags": ["python", "json", "network"]
}

def main():
    # Convert to JSON string
    json_str = json.dumps(data_object)
    print(f"Sending JSON string: {json_str}")

    # Give the server a moment to start (optional, for demo)
    time.sleep(1)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        s.sendall(json_str.encode('utf-8'))
        print("Data sent to server.")

if __name__ == "__main__":
    main()

