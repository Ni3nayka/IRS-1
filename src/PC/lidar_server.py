from rplidar import RPLidar
import sys
import socket
import threading
import json
import time

class LidarServer:
    def __init__(self, lidar_port, host='localhost', port=9999):
        self.lidar = RPLidar(lidar_port)
        self.host = host
        self.port = port
        self.running = False
        self.server_socket = None
        self.clients = []
        self.clients_lock = threading.Lock()  # Lock for thread-safe client operations
        
    def start_server(self):
        """Start the TCP server"""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            print(f"LiDAR server listening on {self.host}:{self.port}")
            
            # Set running flag BEFORE starting threads
            self.running = True
            
            # Start accepting clients in a separate thread
            accept_thread = threading.Thread(target=self.accept_clients)
            accept_thread.daemon = True
            accept_thread.start()
            
            print("Client acceptance thread started. Waiting for connections...")
            
            # Start LiDAR scanning in the main thread
            self.start_lidar_scanning()
            
        except Exception as e:
            print(f"Failed to start server: {e}")
            self.cleanup()
    
    def accept_clients(self):
        """Accept incoming client connections"""
        print("Client acceptance thread is running...")
        while self.running:
            try:
                # Set timeout to allow checking self.running periodically
                self.server_socket.settimeout(1.0)
                client_socket, address = self.server_socket.accept()
                print(f"New client connected: {address}")
                
                # Add client to the list with thread safety
                with self.clients_lock:
                    self.clients.append(client_socket)
                print(f"Total clients connected: {len(self.clients)}")
                
                # Start a thread to handle client messages (if needed)
                client_thread = threading.Thread(
                    target=self.handle_client, 
                    args=(client_socket, address)
                )
                client_thread.daemon = True
                client_thread.start()
                
            except socket.timeout:
                # Timeout is expected, just continue the loop
                continue
            except Exception as e:
                if self.running:
                    print(f"Error accepting client: {e}")
    
    def handle_client(self, client_socket, address):
        """Handle individual client communication"""
        print(f"Started client handler for {address}")
        try:
            while self.running:
                # Check if client is still connected
                try:
                    # Try to receive a small amount of data to detect disconnection
                    client_socket.settimeout(1.0)
                    data = client_socket.recv(1)
                    if not data:  # Client disconnected
                        break
                    # You can process client commands here if needed
                except socket.timeout:
                    # No data received, just continue
                    continue
                except:
                    # Client disconnected or error occurred
                    break
        except Exception as e:
            print(f"Error handling client {address}: {e}")
        finally:
            self.remove_client(client_socket)
            print(f"Client disconnected: {address}")
    
    def remove_client(self, client_socket):
        """Remove a client from the list"""
        with self.clients_lock:
            if client_socket in self.clients:
                self.clients.remove(client_socket)
                print(f"Client removed. Total clients: {len(self.clients)}")
        try:
            client_socket.close()
        except:
            pass
    
    def start_lidar_scanning(self):
        """Start LiDAR scanning and broadcast to clients"""
        print("Starting LiDAR scanning...")
        
        try:
            for scan in self.lidar.iter_scans():
                if not self.running:
                    break
                    
                # Filter scan data (remove points with distance 0)
                filtered_scan = [(angle, distance) for _, angle, distance in scan if distance > 0]
                
                # Prepare data for transmission
                scan_data = {
                    'timestamp': time.time(),
                    'points': filtered_scan
                }
                
                # Convert to JSON and send to all connected clients
                self.broadcast_data(scan_data)
                
        except Exception as e:
            print(f"LiDAR scanning error: {e}")
        finally:
            self.cleanup()
    
    def broadcast_data(self, data):
        """Broadcast scan data to all connected clients"""
        json_data = json.dumps(data) + '\n'
        disconnected_clients = []
        
        # Use lock for thread-safe access to clients list
        with self.clients_lock:
            current_clients = self.clients.copy()
        
        for client in current_clients:
            try:
                client.sendall(json_data.encode('utf-8'))
            except:
                print("Client connection failed, marking for removal")
                disconnected_clients.append(client)
        
        # Remove disconnected clients
        if disconnected_clients:
            with self.clients_lock:
                for client in disconnected_clients:
                    if client in self.clients:
                        self.clients.remove(client)
            print(f"Removed {len(disconnected_clients)} disconnected clients")
    
    def stop(self):
        """Stop the server and cleanup"""
        print("Stopping LiDAR server...")
        self.running = False
        self.cleanup()
    
    def cleanup(self):
        """Cleanup resources"""
        print("Cleaning up resources...")
        
        # Close all client connections
        with self.clients_lock:
            for client in self.clients:
                try:
                    client.close()
                except:
                    pass
            self.clients.clear()
        
        # Close server socket
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass
        
        # Stop LiDAR
        try:
            self.lidar.stop()
            self.lidar.stop_motor()
            self.lidar.disconnect()
            print("LiDAR disconnected")
        except Exception as e:
            print(f"Error disconnecting LiDAR: {e}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python lidar_server.py <lidar_port> [tcp_port]")
        print("Example: python lidar_server.py COM3 9999")
        sys.exit(1)
    
    lidar_port = sys.argv[1]
    tcp_port = int(sys.argv[2]) if len(sys.argv) > 2 else 9999
    
    server = LidarServer(lidar_port, port=tcp_port)
    
    try:
        server.start_server()
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        server.stop()

if __name__ == "__main__":
    main()