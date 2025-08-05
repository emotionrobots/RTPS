import socket
import json

HOST = '127.0.0.1'  # localhost
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen(1)
        print(f"Server listening on {HOST}:{PORT}...")
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")
            data = b""
            while True:
                part = conn.recv(1024)
                if not part:
                    break
                data += part
            json_str = data.decode('utf-8')
            print(f"Received JSON string: {json_str}")

            # Convert JSON string back to Python object
            obj = json.loads(json_str)
            print(f"Converted to Python object: {obj}")

if __name__ == "__main__":
    main()
