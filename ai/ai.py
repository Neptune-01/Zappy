import socket
import random
import threading
import time
from queue import Queue

class AI:
    def __init__(self, port=0, name="", machine="localhost"):
        self.port = port
        self.name = name
        self.machine = machine
        self.unused_slot = None
        self.x = None
        self.y = None
        self.inventory_items = {}
        self.look_result = {}
        self.level = 1
        self.life_units = 10

        # Network components
        self.socket = None
        self.message_buffer = ""
        self.connected = False

        # Message handling
        self.message_queue = Queue()
        self.broadcast_callbacks = []
        self.eject_callbacks = []

        # Threading for message handling
        self.message_thread = None
        self.running = False

    def display(self):
        print(f"AI Config: Port={self.port}, Name={self.name}, Machine={self.machine}")
        if self.x and self.y:
            print(f"Map: {self.x}x{self.y}, Level={self.level}, Life={self.life_units}")

    def connect(self):
        """Connect to the server and handle initial handshake"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            print(f"Connecting to {self.machine}:{self.port}...")
            self.socket.connect((self.machine, self.port))
            print("Connected!")

            if self._handle_handshake():
                self.connected = True
                print("Handshake successful! AI is ready.")
                return True
            else:
                print("Handshake failed!")
                return False

        except Exception as e:
            print(f"Connection error: {e}")
            return False

    def start_message_handler(self):
        """Start the background message handler thread"""
        if not self.message_thread or not self.message_thread.is_alive():
            self.running = True
            self.message_thread = threading.Thread(target=self._message_handler, daemon=True)
            self.message_thread.start()
            print("Message handler started")

    def disconnect(self):
        """Disconnect from server"""
        self.running = False
        self.connected = False
        if self.socket:
            self.socket.close()
        print("Disconnected from server")

    def _handle_handshake(self):
        """Handle initial connection handshake"""
        try:
            # Step 1: Receive WELCOME
            message = self._receive_message(timeout=5.0)
            if message != "WELCOME":
                print(f"Expected WELCOME, got: {message}")
                return False
            print(f"Received: {message}")

            # Step 2: Send team name
            if not self._send_command(self.name):
                return False

            # Step 3: Receive CLIENT-NUM
            message = self._receive_message(timeout=5.0)
            if message and message.isdigit():
                self.unused_slot = int(message)
                print(f"Available slots: {self.unused_slot}")
            else:
                print(f"Expected CLIENT-NUM, got: {message}")
                return False

            # Step 4: Receive map dimensions (X Y)
            message = self._receive_message(timeout=5.0)
            if message and " " in message:
                try:
                    x, y = message.split()
                    self.x = int(x)
                    self.y = int(y)
                    print(f"Map dimensions: {self.x} x {self.y}")
                    return True
                #handshake OK
                except ValueError:
                    print(f"Could not parse map dimensions: {message}")
                    return False
            else:
                print(f"Expected map dimensions, got: {message}")
                return False

        except Exception as e:
            print(f"Handshake error: {e}")
            return False

    def _send_command(self, command):
        """Send a command to the server"""
        if not self.socket:
            print("No socket connection")
            return False

        try:
            message = (command + "\n").encode('utf-8')
            self.socket.send(message)
            print(f"Sent: {command}")
            return True
        except Exception as e:
            print(f"Error sending command '{command}': {e}")
            return False

    def _receive_message(self, timeout=2.0):
        """Receive a single message from the server with timeout"""
        if not self.socket:
            return None

        self.socket.settimeout(timeout)
        try:
            # Check if we already have a complete message in buffer
            if "\n" in self.message_buffer:
                message, self.message_buffer = self.message_buffer.split("\n", 1)
                return message.strip()

            # Read more data from socket
            while "\n" not in self.message_buffer:
                data = self.socket.recv(1024).decode('utf-8')
                if not data:
                    return None  # Connection closed
                self.message_buffer += data

            # Extract the first complete message
            message, self.message_buffer = self.message_buffer.split("\n", 1)
            return message.strip()

        except socket.timeout:
            return None
        except Exception as e:
            print(f"Error receiving message: {e}")
            return None

    def _message_handler(self):
        """Background thread to handle incoming messages"""
        while self.running and self.connected:
            try:
                message = self._receive_message(timeout=1.0)
                if message:
                    self._process_message(message)
            except Exception as e:
                print(f"Message handler error: {e}")
                time.sleep(0.1)

    def _process_message(self, message):
        """Process incoming messages and handle broadcasts/ejections"""
        if message.startswith("message "):
            # Broadcast message: "message K, text"
            try:
                parts = message[8:].split(", ", 1)  # Remove "message " prefix
                direction = int(parts[0])
                text = parts[1] if len(parts) > 1 else ""
                self._handle_broadcast(direction, text)
            except (ValueError, IndexError):
                print(f"Invalid broadcast format: {message}")

        elif message.startswith("eject: "):
            # Ejection message: "eject: K"
            try:
                direction = int(message[7:])  # Remove "eject: " prefix
                self._handle_ejection(direction)
            except ValueError:
                print(f"Invalid eject format: {message}")

        elif message == "dead":
            print("Player is dead!")
            self.life_units = 0

        else:
            # Regular command response - add to queue
            self.message_queue.put(message)

    def _handle_broadcast(self, direction, text):
        """Handle incoming broadcast messages"""
        print(f"Broadcast from direction {direction}: {text}")
        for callback in self.broadcast_callbacks:
            try:
                callback(direction, text)
            except Exception as e:
                print(f"Broadcast callback error: {e}")

    def _handle_ejection(self, direction):
        """Handle ejection events"""
        print(f"Ejected from direction {direction}!")
        for callback in self.eject_callbacks:
            try:
                callback(direction)
            except Exception as e:
                print(f"Ejection callback error: {e}")

    def _send_game_command(self, command):
        """Send a game command to the server (requires established connection)"""
        if not self.connected:
            print("Not connected to server")
            return False

        return self._send_command(command)

    def _wait_for_response(self, timeout=5.0):
        """Wait for a response from the message queue"""
        try:
            return self.message_queue.get(timeout=timeout)
        except:
            return None

    def forward(self):
        """Move forward one tile"""
        if self._send_command("Forward"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    def turn_right(self):
        """Turn 90° right"""
        if self._send_command("Right"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    def turn_left(self):
        """Turn 90° left"""
        if self._send_command("Left"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    # Information Commands
    def look(self):
        """Look around and return visible tiles - FIXED VERSION"""
        if self._send_command("Look"):
            response = self._wait_for_response(timeout=8.0)
            if response:
                # Return the raw response string, not parsed dict
                return response
        return None

    def inventory(self):
        """Check inventory and return items"""
        if self._send_command("Inventory"):
            response = self._wait_for_response(timeout=3.0)
            if response:
                self.inventory_items = self._parse_inventory(response)  # Fixed: assign to inventory_items
                return self.inventory_items
        return None

    def connect_nbr(self):
        """Get number of unused team slots"""
        if self._send_command("Connect_nbr"):
            response = self._wait_for_response(timeout=3.0)
            if response and response.isdigit():
                self.unused_slot = int(response)
                return self.unused_slot
        return None

    # Object Management Commands
    def take(self, object_name):
        """Take an object from current tile"""
        if self._send_command(f"Take {object_name}"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    def set_down(self, object_name):
        """Set down an object on current tile"""
        if self._send_command(f"Set {object_name}"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    # Communication Commands
    def broadcast(self, text):
        """Broadcast a message to all players"""
        formatted_text = f'"{text}"'  # Enclosing the message in quotes
        if self._send_command(f"Broadcast {formatted_text}"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False


    # Action Commands
    def fork(self):
        """Fork a new player (create egg)"""
        if self._send_command("Fork"):
            response = self._wait_for_response(timeout=45.0)  # 42/f seconds
            return response == "ok"
        return False

    def eject(self):
        """Eject all players from current tile"""
        if self._send_command("Eject"):
            response = self._wait_for_response(timeout=8.0)
            return response == "ok"
        return False

    def incantation(self):
        """Start incantation for elevation - FIXED VERSION"""
        if not self._send_command("Incantation"):
            return False

        # Wait for first response - should be "Elevation underway" or "ko"
        first_response = self._wait_for_response(timeout=305.0)  # 300/f seconds
        if hasattr(self, 'log_elevation_event'):
            self.log_elevation_event("INCANTATION_FIRST_RESPONSE", f"Response: '{first_response}'")
        else:
            print(f"INCANTATION_FIRST_RESPONSE: {first_response}")

        if not first_response:
            if hasattr(self, 'log_elevation_event'):
                self.log_elevation_event("INCANTATION_TIMEOUT", "No response received")
            else:
                print("INCANTATION_TIMEOUT: No response received")
            return False

        # Check for immediate failure
        if first_response.strip().lower() == "ko":
            if hasattr(self, 'log_elevation_event'):
                self.log_elevation_event("INCANTATION_FAILED", "Server returned 'ko'")
            else:
                print("INCANTATION_FAILED: Server returned 'ko'")
            return False

        # Check if it's the success case
        if "Elevation underway" in first_response:
            # Wait for the second response with level information
            second_response = self._wait_for_response(timeout=305.0)
            if hasattr(self, 'log_elevation_event'):
                self.log_elevation_event("INCANTATION_SECOND_RESPONSE", f"Response: '{second_response}'")
            else:
                print(f"INCANTATION_SECOND_RESPONSE: {second_response}")

            if second_response and "Current level:" in second_response:
                try:
                    # Extract level number from response
                    level_text = second_response.split("Current level:")[1].strip()
                    # Handle potential extra text after the level number
                    level_number = level_text.split()[0] if level_text.split() else level_text
                    new_level = int(level_number)

                    old_level = self.level
                    self.level = new_level
                    if hasattr(self, 'log_elevation_event'):
                        self.log_elevation_event("LEVEL_UP_SUCCESS",
                            f"*** LEVEL UP! {old_level} -> {new_level} ***")
                    else:
                        print(f"*** LEVEL UP! {old_level} -> {new_level} ***")
                    return True

                except (IndexError, ValueError, AttributeError) as e:
                    if hasattr(self, 'log_elevation_event'):
                        self.log_elevation_event("INCANTATION_PARSE_ERROR",
                            f"Failed to parse level from '{second_response}': {e}")
                    else:
                        print(f"INCANTATION_PARSE_ERROR: Failed to parse level from '{second_response}': {e}")
                    return False
            else:
                if hasattr(self, 'log_elevation_event'):
                    self.log_elevation_event("INCANTATION_NO_LEVEL",
                        f"No level info in second response: '{second_response}'")
                else:
                    print(f"INCANTATION_NO_LEVEL: No level info in second response: '{second_response}'")
                return False

        # Handle single-line response case (some servers might return it all at once)
        elif "Current level:" in first_response:
            try:
                # Extract level from single response
                level_text = first_response.split("Current level:")[1].strip()
                level_number = level_text.split()[0] if level_text.split() else level_text
                new_level = int(level_number)

                old_level = self.level
                self.level = new_level
                if hasattr(self, 'log_elevation_event'):
                    self.log_elevation_event("LEVEL_UP_SUCCESS",
                        f"*** LEVEL UP! {old_level} -> {new_level} ***")
                else:
                    print(f"*** LEVEL UP! {old_level} -> {new_level} ***")
                return True

            except (IndexError, ValueError, AttributeError) as e:
                if hasattr(self, 'log_elevation_event'):
                    self.log_elevation_event("INCANTATION_PARSE_ERROR",
                        f"Failed to parse level from single response '{first_response}': {e}")
                else:
                    print(f"INCANTATION_PARSE_ERROR: Failed to parse level from single response '{first_response}': {e}")
                return False

        # Unexpected response format
        if hasattr(self, 'log_elevation_event'):
            self.log_elevation_event("INCANTATION_UNEXPECTED",
                f"Unexpected response format: '{first_response}'")
        else:
            print(f"INCANTATION_UNEXPECTED: Unexpected response format: '{first_response}'")
        return False

    # Helper Methods
    def _parse_look_response(self, look_result):
        """Parse the look command response"""
        parsed_tiles = {}
        if look_result.startswith('[') and look_result.endswith(']'):
            tiles = look_result[1:-1].split(',')
            tiles = [tile.strip() for tile in tiles]

            for i, tile in enumerate(tiles):
                items = tile.split() if tile else []
                parsed_tiles[i] = items

        return parsed_tiles

    def _parse_inventory(self, inventory_str):
        """Parse inventory response"""
        inventory = {}
        try:
            if inventory_str.startswith('[') and inventory_str.endswith(']'):
                content = inventory_str[1:-1]
                items = content.split(',')

                for item in items:
                    item = item.strip()
                    if ' ' in item:
                        name, count = item.rsplit(' ', 1)
                        inventory[name.strip()] = int(count)

                        # Update life units if food is present
                        if name.strip() == 'food':
                            self.life_units = int(count)

        except Exception as e:
            print(f"Error parsing inventory: {e}")

        return inventory

    # Event Handling
    def on_broadcast(self, callback):
        """Register callback for broadcast messages"""
        self.broadcast_callbacks.append(callback)

    def on_eject(self, callback):
        """Register callback for ejection events"""
        self.eject_callbacks.append(callback)

    # Convenience function
    def move_random(self):
        """Move in a random direction"""
        if random.randint(1, 5) == 1:
            return self.forward()
        else:
            return self.turn_right()

    def collect_food(self):
        """Try to collect food from current tile"""
        return self.take("food")

    def collect_stone(self, stone_name):
        """Try to collect a specific stone"""
        valid_stones = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]
        if stone_name in valid_stones:
            return self.take(stone_name)
        else:
            print(f"Invalid stone name: {stone_name}")
            return False

    def get_current_tile_items(self):
        """Get items on current tile (tile 0 from look result)"""
        if 0 in self.look_result:
            return self.look_result[0]
        return []

    def has_food(self):
        """Check if player has food"""
        return self.inventory_items.get('food', 0) > 0

    def get_food_count(self):
        """Get current food count"""
        return self.inventory_items.get('food', 0)

    def is_alive(self):
        """Check if player is still alive"""
        return self.life_units > 0