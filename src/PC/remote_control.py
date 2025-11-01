#!/usr/bin/env python3
import keyboard
import socket
import threading
import json
import time

class KeyEventServer:
    def __init__(self, host='0.0.0.0', port=8888):
        self.host = host
        self.port = port
        self.clients = []
        self.server_socket = None
        self.running = False
        
    def start_server(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        self.running = True
        
        print(f"TCP Server listening on {self.host}:{self.port}")
        
        # Start accepting clients in a separate thread
        accept_thread = threading.Thread(target=self.accept_clients)
        accept_thread.daemon = True
        accept_thread.start()
        
    def accept_clients(self):
        while self.running:
            try:
                client_socket, address = self.server_socket.accept()
                self.clients.append(client_socket)
                print(f"New client connected: {address}")
                self.broadcast_event({"type": "system", "message": "Connected to key event server"})
            except Exception as e:
                if self.running:
                    print(f"Error accepting client: {e}")
    
    def broadcast_event(self, event_data):
        """Send event to all connected clients"""
        message = json.dumps(event_data) + '\n'
        disconnected_clients = []
        
        for client in self.clients:
            try:
                client.send(message.encode('utf-8'))
            except (BrokenPipeError, ConnectionResetError, OSError):
                disconnected_clients.append(client)
        
        # Remove disconnected clients
        for client in disconnected_clients:
            self.clients.remove(client)
            print("Client disconnected")
    
    def stop_server(self):
        self.running = False
        for client in self.clients:
            client.close()
        if self.server_socket:
            self.server_socket.close()
        print("Server stopped")

class ArrowKeyHandler:
    def __init__(self):
        self.server = KeyEventServer()
        self.key_states = {
            'up': False,
            'down': False, 
            'left': False,
            'right': False
        }
        
    def on_key_event(self, event):
        arrow_keys = {
            'up': ['up', 'w'],
            'down': ['down', 's'], 
            'left': ['left', 'a'],
            'right': ['right', 'd']
        }
        
        # Check if event is for an arrow key
        key_direction = None
        for direction, keys in arrow_keys.items():
            if event.name in keys:
                key_direction = direction
                break
        
        if key_direction:
            if event.event_type == keyboard.KEY_DOWN:
                if not self.key_states[key_direction]:
                    self.key_states[key_direction] = True
                    event_data = {
                        "type": "key_press",
                        "key": key_direction,
                        "timestamp": time.time(),
                        "raw_key": event.name
                    }
                    print(f"Arrow {key_direction.upper()} PRESSED")
                    self.server.broadcast_event(event_data)
                    
            elif event.event_type == keyboard.KEY_UP:
                if self.key_states[key_direction]:
                    self.key_states[key_direction] = False
                    event_data = {
                        "type": "key_release", 
                        "key": key_direction,
                        "timestamp": time.time(),
                        "raw_key": event.name
                    }
                    print(f"Arrow {key_direction.upper()} RELEASED")
                    self.server.broadcast_event(event_data)
    
    def start(self):
        # Start TCP server
        self.server.start_server()
        
        # Start keyboard listener
        print("Listening for arrow keys (↑↓←→ or WASD)...")
        print("Press 'ESC' to exit")
        
        keyboard.hook(self.on_key_event)
        
        try:
            keyboard.wait('esc')
        except KeyboardInterrupt:
            print("\nInterrupted by user")
        finally:
            keyboard.unhook_all()
            self.server.stop_server()

if __name__ == "__main__":
    handler = ArrowKeyHandler()
    handler.start()
