#==================================================================================
#
#  @fn      rtps_client.py
#
#  @brief   RTPS Python client example
#  
#==================================================================================
import socket
import json
import time


HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 65432        # The port used by the server

#---------------------------------------------------------------------------------
# Example Python object to send
#---------------------------------------------------------------------------------
data_object = {
    "title": "larrylisky",
    "x_label": "t (sec)",
    "y_label": "v (V)",
    "width": 800,
    "height": 400,
    "y_count": 3,
    "max_point": 1000,
    "x_step": 0.1,
    "x_range": 10.0,
    "y_min": -2.0,
    "y_max":  2.0,
    "x_grid_step": 1.0,
    "y_grid_step": 0.5,
    "y_color": [ {"r":255, "b":  0, "g":  0, "a":255}, 
                 {"r":  0, "b":255, "g":  0, "a":255},
                 {"r":  0, "b":  0, "g":255, "a":255} ]
}


#---------------------------------------------------------------------------------
#  Main program
#---------------------------------------------------------------------------------
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
