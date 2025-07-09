#!/usr/bin/python

"""
Zappy AI System - Team Coordination

Architecture:
- First AI = SurvivorAI (stays level 1, spawns team, collects food)
- All spawned AIs = LevelerAI (focus on leveling up)
- Same-level coordination: Level N AIs coordinate to level up to Level N+1
- Resource sharing: One set of resources allows all players to level up together

Usage: ./zappy_ai -p port -n name -h machine
"""

import os
from queue import Queue
import sys
import signal
import random
import subprocess
import fcntl
import datetime
import time
import uuid
from ai import AI

class ElevationLogger:
    def __init__(self, log_file_path="zappy_log.log"):
        self.log_file_path = log_file_path
        self.process_id = os.getpid()

    def log(self, message):
        """Thread-safe logging with file locking"""
        timestamp = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
        log_entry = f"[{timestamp}] PID:{self.process_id} - {message}\n"

        try:
            with open(self.log_file_path, "a") as f:
                fcntl.flock(f.fileno(), fcntl.LOCK_EX)
                f.write(log_entry)
                f.flush()
                fcntl.flock(f.fileno(), fcntl.LOCK_UN)
        except Exception as e:
            print(f"Logging error: {e}")

class ZappyAI(AI):
    """Unified AI that determines its role based on team coordination"""

    def __init__(self, port=0, name="", machine="localhost"):
        super().__init__(port, name, machine)

        self.logger = ElevationLogger("zappy_ai_debug.log")
        self.unique_id = str(uuid.uuid4())[:8]

        # Role determination - starts as undetermined
        self.role = "undetermined"  # "survivor", "leveler"
        self.role_decision_time = 0

        # Common properties
        self.team_name = name
        self.action_counter = 0
        self.stuck_counter = 0
        self.facing_direction = 0

        # Caching system
        self.vision_cache = None
        self.inventory_cache = None
        self.vision_cache_counter = 0
        self.inventory_cache_counter = 0

        # Survivor-specific properties
        self.child_processes = []
        self.max_team_members = 20
        self.spawn_cooldown = 3
        self.last_spawn_time = 0
        self.fork_cooldown = 10
        self.last_fork_time = 0
        self.food_collection_threshold = 15  # Lowered from 25

        # Leveler-specific properties
        self.elevation_requirements = {
            1: {"players": 1, "linemate": 1, "deraumere": 0, "sibur": 0, "mendiane": 0, "phiras": 0, "thystame": 0},
            2: {"players": 2, "linemate": 1, "deraumere": 1, "sibur": 1, "mendiane": 0, "phiras": 0, "thystame": 0},
            3: {"players": 2, "linemate": 2, "deraumere": 0, "sibur": 1, "mendiane": 0, "phiras": 2, "thystame": 0},
            4: {"players": 4, "linemate": 1, "deraumere": 1, "sibur": 2, "mendiane": 0, "phiras": 1, "thystame": 0},
            5: {"players": 4, "linemate": 1, "deraumere": 2, "sibur": 1, "mendiane": 3, "phiras": 0, "thystame": 0},
            6: {"players": 6, "linemate": 1, "deraumere": 2, "sibur": 3, "mendiane": 0, "phiras": 1, "thystame": 0},
            7: {"players": 6, "linemate": 2, "deraumere": 2, "sibur": 2, "mendiane": 2, "phiras": 2, "thystame": 1}
        }

        self.elevation_state = "none"
        self.elevation_attempts = 0
        self.max_elevation_attempts = 2
        self.elevation_start_time = 0
        self.coordination_timeout = 30

        self.current_help_target = None
        self.target_direction = None
        self.steps_toward_target = 0
        self.max_seeking_steps = 40

        self.broadcast_cooldown = 8
        self.last_broadcast_time = 0
        self.last_help_broadcast = 0

        # Food thresholds
        self.critical_food_threshold = 5
        self.safe_food_threshold = 15
        self.min_food_for_elevation = 8

        # Rare resources
        self.rare_resources = ["thystame", "phiras"]

        # Role determination broadcast
        self.role_check_sent = False
        self.role_responses_received = 0

        self.logger.log(f"*** AI INITIALIZED *** - Team: {name}, ID: {self.unique_id}")

    def immediate_survivor_setup(self):
        """Immediate setup when becoming survivor - spawn aggressively if possible"""
        available_slots = self.connect_nbr()
        current_food = self.get_food_count()
        # If we have decent food and slots available, start spawning immediately
        if available_slots > 0:
            self.logger.log(f"IMMEDIATE_SPAWN_SETUP: {available_slots} slots, {current_food} food")
            for _ in range(available_slots):
                self.spawn_leveler()
            # Trigger immediate spawning in next cycle
            self.last_spawn_time = 0  # Reset cooldown
            self.spawn_cooldown = 1   # Reduce cooldown for immediate spawning


    def determine_role(self):
        """Determine if this AI should be survivor or leveler - FASTER DECISION"""
        if self.role != "undetermined":
            return

        # Check if we're early in the game (low action count)
        if self.action_counter < 3:  # Reduced from 5
            # Send a role check broadcast to see if there's already a survivor
            if not self.role_check_sent:
                self.send_role_check()
                self.role_check_sent = True
                self.role_decision_time = self.action_counter
                return

            # Wait shorter time for responses - FASTER DECISION
            if self.action_counter - self.role_decision_time < 2:  # Reduced from 3
                return

        # Decision logic: More aggressive role assignment
        # If no survivor responded, or we're the first AI, become survivor
        # If a survivor already exists, become leveler
        if self.role_responses_received == 0 and self.action_counter < 15:  # Reduced from 20
            self.role = "survivor"
            self.logger.log("*** ROLE DECIDED: SURVIVOR ***")
            # IMMEDIATE SPAWNING CHECK when becoming survivor
            self.immediate_survivor_setup()
            self.execute_survivor_strategy()
        else:
            self.role = "leveler"
            self.logger.log("*** ROLE DECIDED: LEVELER ***")

    def send_role_check(self):
        """Send broadcast to check if survivor exists"""
        message = f"[{self.team_name}]ROLE_CHECK_SURVIVOR_EXISTS_FROM_{self.unique_id}"
        self.broadcast_with_tracking(message)
        self.logger.log("ROLE_CHECK_SENT", "Checking if survivor exists")

    def execute_strategy(self):
        """Main strategy execution"""
        try:
            # Determine role if not yet determined
            if self.role == "undetermined":
                self.determine_role()
                return

            # Execute role-specific strategy
            if self.role == "survivor":
                self.execute_survivor_strategy()
            else:
                self.execute_leveler_strategy()

        except Exception as e:
            self.logger.log(f"STRATEGY_ERROR: {e}")
            self.emergency_food_collection()

    def execute_survivor_strategy(self):
        """Survivor AI strategy - FIXED FOR STARVATION PREVENTION - NO SPAWNING LOGIC CHANGES"""
        try:
            current_food = self.get_food_count()
            available_slots = self.connect_nbr()

            self.logger.log(f"SURVIVOR_STRATEGY: Food={current_food}, Slots={available_slots}, Actions={self.action_counter}")

            # PRIORITY 0: CRITICAL SURVIVAL - ABSOLUTE PRIORITY
            if current_food <= 2:
                self.logger.log("CRITICAL_STARVATION: Immediate food required!")
                return self.emergency_food_collection()

            # PRIORITY 1: Emergency food collection if very low
            if current_food <= 4:
                self.logger.log("EMERGENCY_FOOD_MODE: Very low food!")
                if self.collect_food_aggressively():
                    return True
                return self.explore_for_food_only()

            # PRIORITY 2: AGGRESSIVE SPAWNING when we have slots (user's original logic)
            if available_slots and available_slots > 0:
                # Keep original spawning logic but with minimum food safety
                if current_food >= 6:  # Minimum safety threshold
                    if self.should_spawn_team():
                        self.logger.log(f"SPAWN_MODE: {available_slots} slots available, proceeding with spawn")
                        self.spawn_team_members()
                        return True
                else:
                    self.logger.log(f"SPAWN_DELAYED: Need more food before spawning ({current_food} < 6)")

            # PRIORITY 3: Fork for team growth when no slots (user's original logic)
            if available_slots == 0 and current_food >= 15:
                if self.should_fork():
                    self.logger.log("FORK_MODE: No slots available, attempting to fork")
                    if self.fork_for_team():
                        return True

            # PRIORITY 4: Collect food if we're getting low
            if current_food < 12:
                self.logger.log("FOOD_COLLECTION_MODE: Building food reserves")
                if self.collect_food_aggressively():
                    return True

            # PRIORITY 5: Collect food for reserves (but don't over-collect)
            if current_food < 20:
                if self.collect_food_aggressively():
                    return True

            # PRIORITY 6: Explore and support (always check for food first)
            self.explore_and_support()
            return True

        except Exception as e:
            self.logger.log(f"SURVIVOR_ERROR: {e}")
            return self.emergency_food_collection()

    def should_spawn_team(self):
        """Check if we should spawn team members - MORE AGGRESSIVE"""
        available_slots = self.connect_nbr()
        if available_slots is None or available_slots == 0:
            return False

        food_count = self.get_food_count()

        # MUCH more aggressive food requirements
        min_food_for_spawn = 8   # Reduced from 20

        # Emergency spawning - if we have many slots but medium food, spawn anyway
        if available_slots >= 3 and food_count >= 12:
            min_food_for_spawn = 6  # Even more aggressive if many slots

        if food_count < min_food_for_spawn:
            return False

        self.cleanup_child_processes()
        active_children = len([p for p in self.child_processes if p.poll() is None])

        # Reduced spawn cooldown for more aggressive spawning
        spawn_cooldown = 2 if available_slots >= 3 else 3  # Reduced from 3

        return (active_children + 1 < self.max_team_members and
                self.action_counter - self.last_spawn_time > spawn_cooldown)

    def spawn_team_members(self):
        """Spawn multiple team members aggressively - OPTIMIZED FOR SURVIVAL"""
        available_slots = self.connect_nbr()
        active_children = len([p for p in self.child_processes if p.poll() is None])

        slots_to_fill = min(available_slots, self.max_team_members - (active_children + 1))

        # More aggressive spawning based on food reserves
        food_count = self.get_food_count()

        if food_count >= 20:
            spawn_count = min(slots_to_fill, 4)  # Spawn up to 4 if lots of food
        elif food_count >= 15:
            spawn_count = min(slots_to_fill, 3)  # Spawn up to 3 if good food
        elif food_count >= 10:
            spawn_count = min(slots_to_fill, 2)  # Spawn up to 2 if medium food
        else:
            spawn_count = min(slots_to_fill, 1)  # Spawn only 1 if low food

        # EMERGENCY LOGIC: If many slots available, spawn more aggressively
        if available_slots >= 4 and food_count >= 8:
            spawn_count = min(slots_to_fill, spawn_count + 1)
            self.logger.log(f"EMERGENCY_SPAWN_BOOST: Many slots ({available_slots}), boosting spawn count")

        spawned = 0
        min_reserve = 6 if spawn_count > 1 else 4  # Lower reserve requirement

        for i in range(spawn_count):
            current_food = self.get_food_count()
            if current_food >= min_reserve:  # Reduced from 15
                if self.spawn_leveler():
                    spawned += 1
                    time.sleep(0.1)  # Reduced delay
                else:
                    break
            else:
                self.logger.log(f"SPAWN_STOPPED: Food too low ({current_food})")
                break

        if spawned > 0:
            self.last_spawn_time = self.action_counter
            # Dynamic cooldown based on success
            self.spawn_cooldown = 2 if spawned >= 2 else 3
            self.logger.log(f"TEAM_SPAWNED: Created {spawned} leveler AIs (food: {self.get_food_count()})")
        elif food_count < 10:
            self.logger.log(f"SPAWN_DEFERRED: Food too low ({food_count}) for spawning")

    def spawn_leveler(self):
        """Spawn a leveler AI subprocess - with better error handling"""
        try:
            cmd = [
                sys.executable,
                __file__,
                "-p", str(self.port),
                "-n", self.team_name,
                "-h", self.machine
            ]

            process = subprocess.Popen(
                cmd,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL,
                start_new_session=True,
            )

            self.child_processes.append(process)
            self.logger.log(f"SUBPROCESS_SPAWNED: PID {process.pid}")
            return True

        except Exception as e:
            self.logger.log(f"SPAWN_ERROR: Failed to spawn leveler: {e}")
            return False

    def should_fork(self):
        """Check if survivor should fork - MORE AGGRESSIVE"""
        food_count = self.get_food_count()
        available_slots = self.connect_nbr()

        # Much more aggressive forking
        min_food_for_fork = 15  # Reduced from 25

        # Emergency forking if no slots but decent food
        if available_slots == 0 and food_count >= min_food_for_fork:
            fork_cooldown = 6  # Reduced from 10
            return self.action_counter - self.last_fork_time > fork_cooldown

        return False

    def fork_for_team(self):
        """Fork to create team slots"""
        self.logger.log("FORKING: Creating team slot")

        if self.fork():
            self.increment_action_counter()
            self.last_fork_time = self.action_counter
            self.logger.log("FORK_SUCCESS: Team slot created")
            return True
        else:
            self.logger.log("FORK_FAILED: Fork command failed")
            return False

    def should_collect_food_as_survivor(self):
        """Check if survivor should collect food - DYNAMIC BASED ON SPAWNING NEEDS"""
        current_food = self.get_food_count()
        available_slots = self.connect_nbr()

        # If we have slots to fill, don't over-collect food
        if available_slots and available_slots > 0:
            return current_food < 15  # Lower threshold when slots available

        # If no slots, can collect more food
        return current_food < self.food_collection_threshold  # 15

    def collect_food_aggressively(self):
        """VERY aggressive food collection for survivor - IMPROVED LOGGING"""
        found_food = False

        # FIRST: Check current tile for food
        current_items = self.get_current_tile_items()
        if 'food' in current_items:
            food_count_before = current_items.count('food')
            taken = 0
            # Take ALL food on current tile
            for _ in range(food_count_before):
                if self.take_with_tracking('food'):
                    taken += 1
                else:
                    break
            if taken > 0:
                self.logger.log(f"FOOD_COLLECTED: Took {taken} food from current tile (now have {self.get_food_count()})")
                found_food = True

        # SECOND: Look for food in vision and move to closest
        vision = self.get_vision_with_cache()
        closest_food_tile = None
        closest_distance = float('inf')

        # Priority: closer tiles first
        for tile_num in [1, 2, 3, 4, 5, 6, 7, 8]:
            if tile_num in vision and 'food' in vision[tile_num]:
                if tile_num < closest_distance:
                    closest_distance = tile_num
                    closest_food_tile = tile_num

        if closest_food_tile is not None:
            self.logger.log(f"FOOD_SPOTTED: Moving to food at tile {closest_food_tile}")
            self.move_towards_tile(closest_food_tile)
            found_food = True

        return found_food

        # SECOND: Look for food in vision and move to closest
        vision = self.get_vision_with_cache()
        closest_food_tile = None
        closest_distance = float('inf')

        # Priority: closer tiles first
        for tile_num in [1, 2, 3, 4, 5, 6, 7, 8]:  # Search in distance order
            if tile_num in vision and 'food' in vision[tile_num]:
                if tile_num < closest_distance:
                    closest_distance = tile_num
                    closest_food_tile = tile_num

        if closest_food_tile is not None:
            self.logger.log(f"FOOD_SPOTTED: Moving to food at tile {closest_food_tile}")
            self.move_towards_tile(closest_food_tile)
            return True

        return False

    def explore_for_food_only(self):
        """Exploration focused ONLY on finding food - EMERGENCY MODE"""
        self.logger.log(f"EXPLORE_FOOD_ONLY: Emergency food search (current: {self.get_food_count()})")

        # Check current tile first (in case we just moved)
        current_items = self.get_current_tile_items()
        if 'food' in current_items:
            # Take all food, ignore everything else
            food_taken = 0
            for _ in range(current_items.count('food')):
                if self.take_with_tracking('food'):
                    food_taken += 1
            if food_taken > 0:
                self.logger.log(f"EMERGENCY_FOOD_FOUND: {food_taken} food (now have {self.get_food_count()})")
                return True

        # Look for food in vision
        if self.collect_food_aggressively():
            return True

        # If no food in sight, move more systematically
        if self.stuck_counter > 2:
            # Turn to break out of stuck state
            for _ in range(random.randint(1, 2)):
                if random.choice([True, False]):
                    self.turn_right_with_tracking()
                else:
                    self.turn_left_with_tracking()
            self.stuck_counter = 0
            self.logger.log("UNSTUCK: Broke out of stuck state while searching for food")

        # More random movement when desperate for food
        if random.random() < 0.6:  # Higher chance to turn when desperate
            if random.choice([True, False]):
                self.turn_right_with_tracking()
            else:
                self.turn_left_with_tracking()

        # Always try to move forward
        success = self.forward_with_tracking()
        if not success:
            # If forward failed, turn and try again
            if random.choice([True, False]):
                self.turn_right_with_tracking()
            else:
                self.turn_left_with_tracking()

        return True

    def explore_and_support(self):
        """Survivor exploration - ALWAYS prioritize food first"""
        # ALWAYS check for food first, even during normal exploration
        current_items = self.get_current_tile_items()
        if 'food' in current_items:
            # Take the food regardless of other items
            food_taken = 0
            for _ in range(current_items.count('food')):
                if self.take_with_tracking('food'):
                    food_taken += 1
            if food_taken > 0:
                self.logger.log(f"FOOD_COLLECTED: {food_taken} food during exploration (now have {self.get_food_count()})")
                return

        # Anti-stuck mechanism
        if self.stuck_counter > 5:
            for _ in range(random.randint(1, 3)):
                if random.choice([True, False]):
                    self.turn_right_with_tracking()
                else:
                    self.turn_left_with_tracking()
            self.stuck_counter = 0

        # Check if there are elevation resources (but we already took any food)
        elevation_resources = ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]

        if any(res in current_items for res in elevation_resources):
            self.logger.log(f"AVOIDING_RESOURCES: Leaving for levelers: {current_items}")
            # Move away from elevation resources (but we already took any food)
            if random.choice([True, False]):
                self.turn_right_with_tracking()
            else:
                self.turn_left_with_tracking()

        # Random exploration
        if self.action_counter % 8 == 0 and random.random() < 0.3:
            if random.choice([True, False]):
                self.turn_right_with_tracking()
            else:
                self.turn_left_with_tracking()

        self.forward_with_tracking()

    def cleanup_child_processes(self):
        """Clean up dead processes"""
        active_processes = []
        for process in self.child_processes:
            if process.poll() is None:
                active_processes.append(process)

        removed = len(self.child_processes) - len(active_processes)
        if removed > 0:
            self.logger.log(f"PROCESS_CLEANUP: Cleaned up {removed} dead processes")

        self.child_processes = active_processes

    def execute_leveler_strategy(self):
        """Leveler AI strategy - focus on leveling up"""
        try:
            # PRIORITY 0: Survival
            if self.get_food_count() < self.critical_food_threshold:
                return self.emergency_food_collection()

            # PRIORITY 1: Immediate level 1 if possible
            if self.level == 1 and self.can_attempt_level_1_immediately():
                return self.attempt_immediate_level_1()

            # PRIORITY 2: Handle ongoing elevation
            if self.elevation_state != "none":
                if self.handle_elevation_process():
                    return

            # PRIORITY 3: Handle help requests
            if self.current_help_target:
                if self.handle_help_request():
                    return

            # PRIORITY 4: Start own elevation if ready
            if self.can_start_elevation():
                if self.start_elevation_process():
                    return

            # PRIORITY 5: Collect rare resources
            if self.should_collect_rare_resources():
                if self.collect_rare_resources():
                    return

            # PRIORITY 6: Gather elevation resources
            if self.should_gather_elevation_resources():
                if self.gather_elevation_resources():
                    return

            # PRIORITY 7: Collect food if needed
            if self.should_collect_food_as_leveler():
                if self.collect_food():
                    return

            # PRIORITY 8: Explore
            self.explore_for_resources()

            # FIX: If Survivor die create new one
            self.should_spawn_team()

        except Exception as e:
            self.logger.log(f"LEVELER_ERROR: {e}")
            self.emergency_food_collection()

    def can_attempt_level_1_immediately(self):
        """Check if can immediately attempt level 1"""
        current_items = self.get_current_tile_items()
        inventory = self.get_inventory_with_cache()

        has_linemate = ('linemate' in current_items) or (inventory.get('linemate', 0) > 0)
        has_enough_food = self.get_food_count() >= 3

        return has_linemate and has_enough_food

    def attempt_immediate_level_1(self):
        """Immediate level 1 elevation"""
        self.logger.log("IMMEDIATE_LEVEL_1: Attempting immediate elevation")

        current_items = self.get_current_tile_items()
        if 'linemate' in current_items:
            if not self.take_with_tracking('linemate'):
                return False

        if not self.set_down_with_tracking('linemate'):
            return False

        self.elevation_state = "executing"
        success = self.incantation()
        self.increment_action_counter()

        if success:
            self.logger.log(f"IMMEDIATE_SUCCESS: Level up to {self.level}")
            self.elevation_state = "none"
            return True
        else:
            self.logger.log("IMMEDIATE_FAILED: Incantation failed")
            self.elevation_state = "none"
            return False

    def should_collect_rare_resources(self):
        """Always prioritize rare resources"""
        current_items = self.get_current_tile_items()
        return any(rare in current_items for rare in self.rare_resources)

    def collect_rare_resources(self):
        """Collect rare resources immediately"""
        current_items = self.get_current_tile_items()

        for rare_resource in self.rare_resources:
            if rare_resource in current_items:
                if self.take_with_tracking(rare_resource):
                    self.logger.log(f"RARE_COLLECTED: {rare_resource}")
                    return True

        for rare_resource in self.rare_resources:
            rare_tile = self.find_resource_in_vision(rare_resource)
            if rare_tile is not None:
                self.move_towards_tile(rare_tile)
                return True

        return False

    def can_start_elevation(self):
        """Check if can start elevation"""
        if self.elevation_state != "none" or self.current_help_target:
            return False

        if self.get_food_count() < self.min_food_for_elevation:
            return False

        return self.has_all_elevation_resources()

    def has_all_elevation_resources(self):
        """Check if have all elevation resources"""
        if self.level not in self.elevation_requirements:
            return False

        requirements = self.elevation_requirements[self.level]
        inventory = self.get_inventory_with_cache()

        for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
            needed = requirements.get(resource, 0)
            have = inventory.get(resource, 0)
            if have < needed:
                return False

        return True

    def start_elevation_process(self):
        """Start elevation process"""
        requirements = self.elevation_requirements[self.level]
        players_needed = requirements["players"]

        self.elevation_state = "gathering"
        self.elevation_start_time = time.time()

        if players_needed == 1:
            return self.attempt_solo_elevation()
        else:
            if self.prepare_elevation_site():
                self.elevation_state = "broadcasting"
                return self.broadcast_for_help()

        return False

    def attempt_solo_elevation(self):
        """Solo elevation attempt"""
        self.logger.log(f"SOLO_ELEVATION: Level {self.level}")

        if not self.prepare_elevation_site():
            self.reset_elevation_state()
            return False

        self.elevation_state = "executing"
        success = self.incantation()
        self.increment_action_counter()

        if success:
            self.logger.log(f"SOLO_SUCCESS: Now level {self.level}")
            self.reset_elevation_state()
            return True
        else:
            self.logger.log("SOLO_FAILED: Incantation failed")
            self.elevation_attempts += 1
            if self.elevation_attempts >= self.max_elevation_attempts:
                self.reset_elevation_state()
            return False

    def prepare_elevation_site(self):
        """Drop required resources"""
        requirements = self.elevation_requirements[self.level]
        inventory = self.get_inventory_with_cache()
        dropped = []

        for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
            needed = requirements.get(resource, 0)
            have = inventory.get(resource, 0)

            for _ in range(min(needed, have)):
                if self.set_down_with_tracking(resource):
                    dropped.append(resource)
                    time.sleep(0.1)

        self.logger.log(f"SITE_PREPARED: Dropped {dropped}")
        return len(dropped) > 0

    def broadcast_for_help(self):
        """Broadcast for elevation help"""
        requirements = self.elevation_requirements[self.level]
        players_needed = requirements["players"]

        message = f"ELEV_FROM_L{self.level}_TO_L{self.level+1}_NEED_{players_needed-1}_ID{self.unique_id}"
        team_message = f"[{self.team_name}]{message}"

        self.logger.log(f"BROADCASTING: Need {players_needed-1} more level {self.level} players")

        if self.broadcast_with_tracking(team_message):
            self.last_help_broadcast = self.action_counter
            self.elevation_state = "waiting"
            return True
        return False

    def handle_elevation_process(self):
        """Handle ongoing elevation"""
        if time.time() - self.elevation_start_time > self.coordination_timeout:
            self.logger.log("ELEVATION_TIMEOUT")
            self.reset_elevation_state()
            return False

        if self.elevation_state == "waiting":
            return self.wait_for_helpers()
        elif self.elevation_state == "broadcasting":
            if self.action_counter - self.last_help_broadcast > 10:
                return self.broadcast_for_help()
            return True

        return False

    def wait_for_helpers(self):
        """Wait for helpers to arrive"""
        players_here = self.count_players_on_current_tile()
        required = self.elevation_requirements[self.level]["players"]

        if players_here >= required:
            if self.verify_elevation_requirements():
                return self.attempt_coordinated_elevation()
            else:
                return self.prepare_elevation_site()
        else:
            if self.action_counter - self.last_help_broadcast > 15:
                return self.broadcast_for_help()

        return True

    def attempt_coordinated_elevation(self):
        """Coordinated elevation attempt"""
        players_here = self.count_players_on_current_tile()
        required = self.elevation_requirements[self.level]["players"]

        self.logger.log(f"COORDINATED_ELEVATION: {players_here}/{required} players")

        self.elevation_state = "executing"
        success = self.incantation()
        self.increment_action_counter()

        if success:
            self.logger.log(f"COORDINATED_SUCCESS: Now level {self.level}")
            self.reset_elevation_state()
            return True
        else:
            self.logger.log("COORDINATED_FAILED")
            self.elevation_attempts += 1
            if self.elevation_attempts >= self.max_elevation_attempts:
                self.reset_elevation_state()
            return False

    def verify_elevation_requirements(self):
        """Verify elevation requirements"""
        requirements = self.elevation_requirements[self.level]

        players_here = self.count_players_on_current_tile()
        if players_here < requirements["players"]:
            return False

        resources_on_tile = self.get_resources_on_tile()
        for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
            needed = requirements.get(resource, 0)
            on_tile = resources_on_tile.get(resource, 0)
            if on_tile < needed:
                return False

        return True

    def handle_team_broadcast(self, direction, text):
        """Handle team broadcasts"""
        clean_text = text.strip().strip('"').strip("'")

        team_prefix = f"[{self.team_name}]"
        if not clean_text.startswith(team_prefix):
            return

        message = clean_text[len(team_prefix):]

        # Handle role check responses
        if "ROLE_CHECK_SURVIVOR_EXISTS" in message and self.role == "undetermined":
            # Don't respond to our own message
            if f"FROM_{self.unique_id}" not in message:
                self.role_responses_received += 1

        # Respond to role checks if we're survivor
        elif "ROLE_CHECK_SURVIVOR_EXISTS" in message and self.role == "survivor":
            if f"FROM_{self.unique_id}" not in message:
                response = f"[{self.team_name}]SURVIVOR_EXISTS_FROM_{self.unique_id}"
                self.broadcast_with_tracking(response)

        # Handle elevation requests
        elif "ELEV_FROM_L" in message:
            self.handle_elevation_request(message, direction)

    def handle_elevation_request(self, message, direction):
        """Handle elevation coordination request"""
        try:
            parts = message.replace("ELEV_FROM_L", "").split("_")
            from_level = int(parts[0].split("TO")[0])
            to_level = int(parts[0].split("TO")[1][1:])
            need_count = int(parts[2])
            requester_id = parts[3][2:]

            if self.can_help_with_elevation(from_level, direction, requester_id):
                self.accept_elevation_help(requester_id, direction, from_level)
                self.send_elevation_response(requester_id, "JOINING")

        except (ValueError, IndexError) as e:
            self.logger.log(f"PARSE_ERROR: {message} - {e}")

    def can_help_with_elevation(self, from_level, direction, requester_id):
        """Check if can help with elevation"""
        if self.level != from_level:
            return False

        if self.elevation_state != "none" or self.current_help_target:
            return False

        if self.get_food_count() < 5:
            return False

        if direction == 0:
            return False

        return True

    def accept_elevation_help(self, requester_id, direction, from_level):
        """Accept elevation help request"""
        self.current_help_target = requester_id
        self.target_direction = direction
        self.steps_toward_target = 0
        self.elevation_state = "joining"

        self.logger.log(f"JOINING_ELEVATION: Helping {requester_id} from level {from_level}")

    def send_elevation_response(self, requester_id, response):
        """Send elevation response"""
        response_message = f"[{self.team_name}]RESP_{response}_TO_{requester_id}_FROM_{self.unique_id}"
        self.broadcast_with_tracking(response_message)

    def handle_help_request(self):
        """Navigate to help another AI"""
        if not self.target_direction:
            return False

        moved = self.move_towards_broadcast_direction(self.target_direction)

        if moved:
            self.steps_toward_target += 1

            if self.steps_toward_target % 3 == 0:
                if self.detect_elevation_site():
                    self.logger.log(f"ARRIVED_AT_ELEVATION: Found site for {self.current_help_target}")
                    self.elevation_state = "waiting"
                    return True

            if self.steps_toward_target > self.max_seeking_steps:
                self.logger.log(f"HELP_TIMEOUT: Giving up on {self.current_help_target}")
                self.reset_help_state()
                return False

        return True

    def move_towards_broadcast_direction(self, direction):
        """Move towards broadcast direction"""
        if direction == 1:
            return self.forward_with_tracking()
        elif direction == 2:
            if random.choice([True, False]):
                return self.forward_with_tracking()
            else:
                self.turn_right_with_tracking()
                return self.forward_with_tracking()
        elif direction == 3:
            self.turn_right_with_tracking()
            return self.forward_with_tracking()
        elif direction == 4:
            self.turn_right_with_tracking()
            return self.forward_with_tracking()
        elif direction == 5:
            self.turn_right_with_tracking()
            self.turn_right_with_tracking()
            return self.forward_with_tracking()
        elif direction == 6:
            self.turn_left_with_tracking()
            return self.forward_with_tracking()
        elif direction == 7:
            self.turn_left_with_tracking()
            return self.forward_with_tracking()
        elif direction == 8:
            self.turn_left_with_tracking()
            return self.forward_with_tracking()
        else:
            return self.forward_with_tracking()

    def detect_elevation_site(self):
        """Detect elevation site"""
        players_here = self.count_players_on_current_tile()
        resources_here = self.get_resources_on_tile()
        return players_here > 1 and len(resources_here) > 0

    def should_gather_elevation_resources(self):
        """Check if should gather resources"""
        if self.get_food_count() < self.safe_food_threshold:
            return False
        return not self.has_all_elevation_resources()

    def gather_elevation_resources(self):
        """Gather needed resources"""
        needed = self.get_needed_resources()

        current_items = self.get_current_tile_items()

        priority_order = []
        for resource in needed:
            if resource in self.rare_resources:
                priority_order.insert(0, resource)
            else:
                priority_order.append(resource)

        for resource in priority_order:
            if resource in current_items:
                if self.take_with_tracking(resource):
                    self.logger.log(f"RESOURCE_COLLECTED: {resource}")
                    return True

        for resource in priority_order:
            resource_tile = self.find_resource_in_vision(resource)
            if resource_tile is not None:
                self.move_towards_tile(resource_tile)
                return True

        return False

    def get_needed_resources(self):
        """Get needed resources list"""
        if self.level not in self.elevation_requirements:
            return []

        requirements = self.elevation_requirements[self.level]
        inventory = self.get_inventory_with_cache()
        needed = []

        for resource in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
            required = requirements.get(resource, 0)
            have = inventory.get(resource, 0)
            if have < required:
                needed.append(resource)

        return needed

    def should_collect_food_as_leveler(self):
        """Check if leveler should collect food"""
        return self.get_food_count() < self.safe_food_threshold

    def collect_food(self):
        """Collect food"""
        current_items = self.get_current_tile_items()
        if 'food' in current_items:
            if self.take_with_tracking('food'):
                self.logger.log("FOOD_COLLECTED")
                return True

        food_tile = self.find_resource_in_vision('food')
        if food_tile is not None:
            self.move_towards_tile(food_tile)
            return True

        return False

    def emergency_food_collection(self):
        """CRITICAL emergency food collection - LAST RESORT"""
        current_food = self.get_food_count()
        self.logger.log(f"EMERGENCY_FOOD: CRITICAL - Only {current_food} food left!")

        # Reset any ongoing activities - survival is absolute priority
        self.reset_all_states()

        # Take ANY food on current tile immediately
        current_items = self.get_current_tile_items()
        if 'food' in current_items:
            food_taken = 0
            for _ in range(current_items.count('food')):
                if self.take_with_tracking('food'):
                    food_taken += 1
                    self.logger.log(f"EMERGENCY_SAVE: Took food #{food_taken} (now have {self.get_food_count()})")
                else:
                    break
            if food_taken > 0:
                return True

        # Aggressive search in vision
        if self.collect_food_aggressively():
            return True

        # Desperate exploration - move faster, more random
        self.logger.log("DESPERATE_SEARCH: No food visible, exploring frantically")

        # Very aggressive turning when desperate
        if random.random() < 0.8:  # Very high chance to change direction
            turns = random.randint(1, 3)
            for _ in range(turns):
                if random.choice([True, False]):
                    self.turn_right_with_tracking()
                else:
                    self.turn_left_with_tracking()

        # Always try to move
        if not self.forward_with_tracking():
            # If can't move forward, turn and try again
            self.turn_right_with_tracking()
            self.forward_with_tracking()

        return True

    def explore_for_resources(self):
        """Basic exploration"""
        if self.stuck_counter > 5:
            for _ in range(random.randint(1, 3)):
                if random.choice([True, False]):
                    self.turn_right_with_tracking()
                else:
                    self.turn_left_with_tracking()
            self.stuck_counter = 0

        if self.action_counter % 10 == 0 and random.random() < 0.4:
            if random.choice([True, False]):
                self.turn_right_with_tracking()
            else:
                self.turn_left_with_tracking()

        self.forward_with_tracking()

    def reset_elevation_state(self):
        """Reset elevation state"""
        self.elevation_state = "none"
        self.elevation_start_time = 0
        self.elevation_attempts = 0

    def reset_help_state(self):
        """Reset help state"""
        self.current_help_target = None
        self.target_direction = None
        self.steps_toward_target = 0

    def reset_all_states(self):
        """Reset all states"""
        self.reset_elevation_state()
        self.reset_help_state()

    # Utility methods
    def get_food_count(self):
        inventory = self.get_inventory_with_cache()
        return inventory.get('food', 0)

    def increment_action_counter(self):
        self.action_counter += 1

    def forward_with_tracking(self):
        success = self.forward()
        self.increment_action_counter()
        if success:
            self.stuck_counter = 0
            self.invalidate_caches()
        else:
            self.stuck_counter += 1
        return success

    def turn_right_with_tracking(self):
        success = self.turn_right()
        self.increment_action_counter()
        if success:
            self.facing_direction = (self.facing_direction + 1) % 4
        return success

    def turn_left_with_tracking(self):
        success = self.turn_left()
        self.increment_action_counter()
        if success:
            self.facing_direction = (self.facing_direction - 1) % 4
        return success

    def take_with_tracking(self, item):
        success = self.take(item)
        self.increment_action_counter()
        if success:
            self.invalidate_caches()
        return success

    def set_down_with_tracking(self, item):
        success = self.set_down(item)
        self.increment_action_counter()
        if success:
            self.invalidate_caches()
        return success

    def broadcast_with_tracking(self, message):
        success = self.broadcast(message)
        self.increment_action_counter()
        return success

    def get_inventory_with_cache(self):
        if (self.inventory_cache is None or
            self.action_counter - self.inventory_cache_counter > 2):
            result = self.inventory()
            if result:
                self.inventory_cache = result
                self.inventory_cache_counter = self.action_counter
            else:
                self.inventory_cache = self.inventory_items or {}
        return self.inventory_cache or {}

    def get_vision_with_cache(self):
        if (self.vision_cache is None or
            self.action_counter - self.vision_cache_counter > 1):
            look_response = self.look()
            if look_response:
                self.vision_cache = self._parse_look_response(look_response)
                self.vision_cache_counter = self.action_counter
            else:
                return {}
        return self.vision_cache or {}

    def invalidate_caches(self):
        self.vision_cache = None
        self.inventory_cache = None

    def get_current_tile_items(self):
        vision = self.get_vision_with_cache()
        if 0 in vision:
            return [item for item in vision[0] if item != "player"]
        return []

    def find_resource_in_vision(self, resource_name):
        vision = self.get_vision_with_cache()
        for tile_num in sorted(vision.keys()):
            if resource_name in vision[tile_num]:
                return tile_num
        return None

    def move_towards_tile(self, tile_num):
        if tile_num == 0:
            return True
        elif tile_num == 1:
            self.turn_left_with_tracking()
            return self.forward_with_tracking()
        elif tile_num == 2:
            return self.forward_with_tracking()
        elif tile_num == 3:
            self.turn_right_with_tracking()
            return self.forward_with_tracking()
        else:
            return self.forward_with_tracking()

    def count_players_on_current_tile(self):
        vision = self.get_vision_with_cache()
        if 0 in vision:
            return vision[0].count("player")
        return 1

    def get_resources_on_tile(self):
        current_items = self.get_current_tile_items()
        resources = {}
        for item in current_items:
            if item in ["linemate", "deraumere", "sibur", "mendiane", "phiras", "thystame"]:
                resources[item] = resources.get(item, 0) + 1
        return resources

def display_status(ai):
    """Display AI status"""
    inventory = ai.get_inventory_with_cache()
    food_count = inventory.get('food', 0)

    print(f"\n=== {ai.role.upper()} AI STATUS (ID: {ai.unique_id}) ===")
    print(f"Level: {ai.level} | Food: {food_count} | Actions: {ai.action_counter}")

    if ai.role == "survivor" and hasattr(ai, 'child_processes'):
        active_children = len([p for p in ai.child_processes if p.poll() is None])
        print(f"Team Size: {active_children + 1}")

    if ai.role == "leveler":
        print(f"Elevation State: {ai.elevation_state}")
        if ai.current_help_target:
            print(f"Helping: {ai.current_help_target}")

        needed = ai.get_needed_resources()
        if needed:
            print(f"Needed: {needed}")

    rare_in_inventory = []
    for rare in ["phiras", "thystame"]:
        count = inventory.get(rare, 0)
        if count > 0:
            rare_in_inventory.append(f"{rare}:{count}")
    if rare_in_inventory:
        print(f"Rare Resources: {', '.join(rare_in_inventory)}")

    print("=" * 50)

def main():
    """Main function"""
    ai = ZappyAI()
    print(f"Created ZappyAI - ID: {ai.unique_id}")

    if len(sys.argv) == 2 and sys.argv[1] == "-help":
        print("USAGE: ./zappy_ai -p port -n name -h machine")
        exit(0)

    if len(sys.argv) < 5:
        print("Invalid args number, type -help.")
        exit(84)

    # Parse arguments
    i = 1
    while i < len(sys.argv):
        if i + 1 >= len(sys.argv):
            print("Invalid args number, type -help.")
            exit(84)

        try:
            if sys.argv[i] == "-p":
                ai.port = int(sys.argv[i + 1])
            elif sys.argv[i] == "-n":
                ai.name = str(sys.argv[i + 1])
                ai.team_name = ai.name
            elif sys.argv[i] == "-h":
                ai.machine = str(sys.argv[i + 1])
            else:
                print(f"Unknown argument: {sys.argv[i]}")
                exit(84)
        except ValueError:
            print("Could not convert data to an integer.")
            exit(84)

        i += 2

    if ai.port == 0 or ai.name == "":
        print("Invalid args number, type -help.")
        exit(84)

    ai.display()

    # Connect to server
    if not ai.connect():
        print("Failed to connect to server")
        exit(1)

    # Start message handler
    ai.start_message_handler()

    # Set up event handlers
    ai.on_broadcast(ai.handle_team_broadcast)

    # Signal handling
    def signal_handler(sig, frame):
        if hasattr(ai, 'cleanup_child_processes'):
            ai.cleanup_child_processes()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    signal.signal(signal.SIGPIPE, signal_handler)

    try:
        ai.logger.log(f"AI_STARTED: ID {ai.unique_id}")
        display_status(ai)
        last_status_time = 0

        while ai.is_alive() and ai.connected:
            try:
                ai.execute_strategy()

                # Display status every 50 actions
                if ai.action_counter - last_status_time >= 50:
                    display_status(ai)
                    last_status_time = ai.action_counter

            except Exception as e:
                ai.logger.log(f"MAIN_LOOP_ERROR: {e}")
                ai.emergency_food_collection()

    except KeyboardInterrupt:
        ai.logger.log("INTERRUPTED: AI stopped")
    finally:
        if hasattr(ai, 'cleanup_child_processes'):
            ai.cleanup_child_processes()

if __name__ == "__main__":
    main()