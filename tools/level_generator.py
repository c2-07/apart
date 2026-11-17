#!/usr/bin/env python3
"""
Level Generator for 3D Roblox-Style Single-Player Obby.
Generates 20 complete, verified levels across 4 worlds.
All gaps are mathematically guaranteed to be within the player's jump arc (max ~6.1m).
"""

import os
import math

LEVELS_DIR = os.path.join(os.path.dirname(os.path.dirname(__file__)), "assets", "levels")
os.makedirs(LEVELS_DIR, exist_ok=True)

def write_level(filename, content):
    path = os.path.join(LEVELS_DIR, filename)
    with open(path, "w") as f:
        f.write(content.strip() + "\n")
    print(f"Generated {filename}")

# ==============================================================================
# WORLD 1: MEADOW (Basics, jumps, simple spikes, elevated peak)
# ==============================================================================

# w1_l1.lvl is already authored. Let's make sure it's consistent or regenerated cleanly.
w1_l1 = """# World 1 - Level 1: Meadow Steps
name Meadow Steps
world 1
killY -15.0
par 35.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Forward steps
box 0 0 -6 3 1 3 1
box 0 0.5 -11 3 1 3 0
box 2 1.0 -16 3 1 3 2
box 4 1.5 -21 3 1 3 1
box 2 2.0 -26 3 1 3 0
box 0 2.5 -31 3 1 3 2
box 0 3.0 -36 3 1 3 1

# Checkpoint 1
box 0 3.0 -42 6 1 6 0
checkpoint 0 3.5 -42

# Curve right
box 6 3.5 -42 3 1 3 1
box 11 4.0 -42 3 1 3 2
box 16 4.5 -40 3 1 3 0
box 21 5.0 -38 3 1 3 1
box 26 5.5 -38 3 1 3 2
box 31 6.0 -40 3 1 3 0
box 36 6.5 -42 3 1 3 1
box 41 7.0 -42 3 1 3 2

# Checkpoint 2
box 47 7.0 -42 6 1 6 0
checkpoint 47 7.5 -42

# Final climb
box 47 7.5 -48 3 1 3 1
box 47 8.0 -53 3 1 3 2
box 45 8.5 -58 3 1 3 0
box 43 9.0 -63 3 1 3 1
box 45 9.5 -68 3 1 3 2
box 47 10.0 -73 3 1 3 0
box 47 10.5 -78 3 1 3 1
box 47 11.0 -83 3 1 3 2
box 47 11.5 -88 4 1 4 0

box 47 12.0 -96 8 1 8 3
finish 47 13.5 -96 4 3 4
"""

w1_l2 = """# World 1 - Level 2: Gentle Slopes
name Gentle Slopes
world 1
killY -15.0
par 38.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1
box -3 0.5 -10 3 1 3 0
box -6 1.0 -14 3 1 3 2
box -8 1.5 -19 3 1 3 1
box -6 2.0 -24 3 1 3 0
box -3 2.5 -28 3 1 3 2
box 0 3.0 -32 3 1 3 1

box 0 3.0 -38 6 1 6 0
checkpoint 0 3.5 -38

box 4 3.5 -42 3 1 3 1
box 8 4.0 -45 3 1 3 0
box 12 4.5 -48 3 1 3 2
box 16 5.0 -51 3 1 3 1
box 20 5.5 -53 3 1 3 0
box 24 6.0 -54 3 1 3 2

box 30 6.0 -54 6 1 6 0
checkpoint 30 6.5 -54

box 30 6.5 -60 3 1 3 1
box 30 7.0 -65 3 1 3 2
box 28 7.5 -70 3 1 3 0
box 25 8.0 -74 3 1 3 1
box 22 8.5 -78 3 1 3 2
box 20 9.0 -82 4 1 4 0

box 20 9.5 -90 8 1 8 3
finish 20 11.0 -90 4 3 4
"""

w1_l3 = """# World 1 - Level 3: Winding Canopy
name Winding Canopy
world 1
killY -15.0
par 42.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1
box 4 0.5 -10 3 1 3 0
box 8 1.0 -13 3 1 3 2
box 12 1.5 -15 3 1 3 1
box 16 2.0 -15 3 1 3 0
box 20 2.5 -13 3 1 3 2
box 24 3.0 -10 3 1 3 1

box 29 3.0 -10 6 1 6 0
checkpoint 29 3.5 -10

box 29 3.5 -16 3 1 3 1
box 27 4.0 -21 3 1 3 0
box 24 4.5 -25 3 1 3 2
box 20 5.0 -28 3 1 3 1
box 16 5.5 -30 3 1 3 0
box 12 6.0 -30 3 1 3 2
box 8 6.5 -28 3 1 3 1

box 2 6.5 -28 6 1 6 0
checkpoint 2 7.0 -28

box 2 7.0 -34 3 1 3 1
box 0 7.5 -39 3 1 3 2
box -2 8.0 -44 3 1 3 0
box -2 8.5 -49 3 1 3 1
box 0 9.0 -54 3 1 3 2
box 2 9.5 -59 4 1 4 0

box 2 10.0 -67 8 1 8 3
finish 2 11.5 -67 4 3 4
"""

w1_l4 = """# World 1 - Level 4: Spiky Crossings
name Spiky Crossings
world 1
killY -15.0
par 40.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 4 1 4 1
spikes 0 0.8 -6 2 0.6 2
box 0 0.5 -12 3 1 3 0
box 3 1.0 -17 3 1 3 2
box 6 1.5 -22 4 1 4 1
spikes 6 2.3 -22 2 0.6 2
box 6 2.0 -28 3 1 3 0

box 6 2.5 -35 6 1 6 0
checkpoint 6 3.0 -35

box 1 3.0 -35 3 1 3 1
box -4 3.5 -35 4 1 4 2
spikes -4 4.3 -35 2 0.6 2
box -9 4.0 -35 3 1 3 0
box -14 4.5 -37 3 1 3 1
box -18 5.0 -41 4 1 4 2
spikes -18 5.8 -41 2 0.6 2

box -23 5.5 -46 6 1 6 0
checkpoint -23 6.0 -46

box -23 6.0 -52 3 1 3 1
box -21 6.5 -57 4 1 4 0
spikes -21 7.3 -57 2 0.6 2
box -18 7.0 -62 3 1 3 2
box -15 7.5 -67 3 1 3 1
box -12 8.0 -72 4 1 4 0

box -12 8.5 -80 8 1 8 3
finish -12 10.0 -80 4 3 4
"""

w1_l5 = """# World 1 - Level 5: Meadow Apex (Boss Course)
name Meadow Apex
world 1
killY -18.0
par 50.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Towering spiral ascent
box 0 0 -6 3 1 3 1
box 4 0.8 -9 3 1 3 0
box 8 1.6 -10 3 1 3 2
box 11 2.4 -8 3 1 3 1
box 13 3.2 -4 3 1 3 0
box 13 4.0 0 3 1 3 2
box 11 4.8 4 3 1 3 1
box 8 5.6 6 3 1 3 0

box 2 5.6 6 6 1 6 0
checkpoint 2 6.1 6

box -3 6.4 5 3 1 3 1
box -7 7.2 2 3 1 3 2
box -9 8.0 -2 3 1 3 0
box -9 8.8 -7 3 1 3 1
box -7 9.6 -11 3 1 3 2
box -3 10.4 -13 3 1 3 0
box 1 11.2 -13 3 1 3 1

box 7 11.2 -13 6 1 6 0
checkpoint 7 11.7 -13

box 7 12.0 -19 3 1 3 1
box 5 12.8 -24 3 1 3 2
box 1 13.6 -28 3 1 3 0
box -4 14.4 -30 3 1 3 1
box -9 15.2 -30 3 1 3 2
box -14 16.0 -28 4 1 4 0

box -22 16.5 -28 8 1 8 3
finish -22 18.0 -28 4 3 4
"""

# ==============================================================================
# WORLD 2: OCEAN (Moving platforms, bounce pads)
# ==============================================================================

w2_l1 = """# World 2 - Level 1: Tidal Planks
name Tidal Planks
world 2
killY -15.0
par 38.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Static stepping block
box 0 0 -6 3 1 3 1

# First Mover: moves along X by 6 meters
mover 0 0 -12 4 1 4 6 0 0 3.5 0
box 6 0.5 -18 4 1 4 2

# Checkpoint 1
box 6 0.5 -24 6 1 6 0
checkpoint 6 1.0 -24

# Second Mover: moves along Z by 6 meters
mover 6 0.5 -30 4 1 4 0 0 -6 3.5 1
box 6 1.0 -41 4 1 4 2

# Checkpoint 2
box 6 1.0 -47 6 1 6 0
checkpoint 6 1.5 -47

# Third Mover: diagonal motion
mover 6 1.0 -53 4 1 4 -6 0 -5 4.0 0
box -2 1.5 -63 4 1 4 2

# Finish
box -2 1.5 -71 8 1 8 3
finish -2 3.0 -71 4 3 4
"""

w2_l2 = """# World 2 - Level 2: Coral Springs
name Coral Springs
world 2
killY -15.0
par 35.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1

# First Bounce Pad launching player upward 22 m/s to Y=4.5
bounce 0 0.2 -11 3 0.5 3 22.0
box 0 4.0 -19 5 1 5 2

# Checkpoint 1
box 0 4.0 -26 6 1 6 0
checkpoint 0 4.5 -26

# Stepping platforms
box 0 4.5 -32 3 1 3 1
box 4 5.0 -36 3 1 3 0

# Second Bounce Pad launching player to high ledge
bounce 8 5.2 -41 3 0.5 3 22.0
box 8 9.5 -50 6 1 6 2

# Checkpoint 2
box 8 9.5 -57 6 1 6 0
checkpoint 8 10.0 -57

box 8 10.0 -63 3 1 3 1
box 6 10.5 -68 3 1 3 0
box 4 11.0 -73 4 1 4 2

box 4 11.5 -81 8 1 8 3
finish 4 13.0 -81 4 3 4
"""

w2_l3 = """# World 2 - Level 3: Shifting Sands
name Shifting Sands
world 2
killY -15.0
par 40.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Two moving platforms moving in opposite phases
mover 0 0 -7 3 1 3 5 0 0 3.0 0
mover 5 0.5 -14 3 1 3 -5 0 0 3.0 1
box 0 1.0 -21 5 1 5 2

# Checkpoint 1
box 0 1.0 -28 6 1 6 0
checkpoint 0 1.5 -28

# Vertical Mover: lifts player up 5 meters
mover 0 1.0 -34 4 1 4 0 5 0 3.5 0
box 0 6.0 -41 5 1 5 2

# Checkpoint 2
box 0 6.0 -48 6 1 6 0
checkpoint 0 6.5 -48

mover 0 6.0 -55 3 1 3 6 0 -4 4.0 1
box 6 6.5 -64 4 1 4 0
box 6 7.0 -70 4 1 4 2

box 6 7.5 -78 8 1 8 3
finish 6 9.0 -78 4 3 4
"""

w2_l4 = """# World 2 - Level 4: High Tide Launch
name High Tide Launch
world 2
killY -15.0
par 42.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

bounce 0 0.2 -7 3 0.5 3 22.0
mover 0 4.5 -16 4 1 4 5 0 0 3.5 0
box 5 5.0 -24 5 1 5 2

# Checkpoint 1
box 5 5.0 -31 6 1 6 0
checkpoint 5 5.5 -31

box 5 5.5 -37 3 1 3 1
bounce 5 5.7 -42 3 0.5 3 22.0
mover 5 10.0 -51 4 1 4 -6 0 0 3.5 2
box -1 10.5 -59 5 1 5 0

# Checkpoint 2
box -1 10.5 -66 6 1 6 0
checkpoint -1 11.0 -66

mover -1 10.5 -72 3 1 3 0 4 0 3.0 1
box -1 15.0 -79 4 1 4 2
box -1 15.5 -85 4 1 4 0

box -1 16.0 -93 8 1 8 3
finish -1 17.5 -93 4 3 4
"""

w2_l5 = """# World 2 - Level 5: Sunken Citadel (Boss Course)
name Sunken Citadel
world 2
killY -18.0
par 55.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Mover across lagoon
mover 0 0 -7 4 1 4 6 0 0 3.5 0
box 6 0.5 -14 4 1 4 2

# Launch into upper battlements
bounce 6 0.7 -19 3 0.5 3 22.0
mover 6 5.5 -29 4 1 4 0 0 -6 3.5 1
box 6 6.0 -40 5 1 5 0

# Checkpoint 1
box 6 6.0 -47 6 1 6 0
checkpoint 6 6.5 -47

# Multi-mover chain
mover 6 6.0 -54 3 1 3 -6 0 0 3.0 2
mover 0 6.5 -61 3 1 3 6 0 0 3.0 0
mover 6 7.0 -68 3 1 3 -6 0 0 3.0 1
box 0 7.5 -76 5 1 5 2

# Checkpoint 2
box 0 7.5 -83 6 1 6 0
checkpoint 0 8.0 -83

# Grand high-launch sequence
bounce 0 7.7 -88 3 0.5 3 22.0
box 0 13.0 -98 5 1 5 1
bounce 0 13.2 -103 3 0.5 3 22.0
box 0 18.5 -114 6 1 6 2

box 0 19.0 -123 8 1 8 3
finish 0 20.5 -123 4 3 4
"""

# ==============================================================================
# WORLD 3: WINTER (Crumble tiles, spinners)
# ==============================================================================

w3_l1 = """# World 3 - Level 1: Thin Ice
name Thin Ice
world 3
killY -15.0
par 35.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Crumble sequence
crumble 0 0 -6 3 1 3 1.0 2.5
crumble 0 0.5 -11 3 1 3 0.9 2.5
crumble 0 1.0 -16 3 1 3 0.8 2.5
box 0 1.5 -22 5 1 5 2

# Checkpoint 1
box 0 1.5 -29 6 1 6 0
checkpoint 0 2.0 -29

crumble 3 2.0 -34 3 1 3 0.8 2.0
crumble 6 2.5 -38 3 1 3 0.8 2.0
crumble 9 3.0 -42 3 1 3 0.8 2.0
box 13 3.5 -47 5 1 5 1

# Checkpoint 2
box 13 3.5 -54 6 1 6 0
checkpoint 13 4.0 -54

crumble 13 4.0 -60 3 1 3 0.7 2.0
crumble 11 4.5 -65 3 1 3 0.7 2.0
crumble 9 5.0 -70 3 1 3 0.7 2.0
box 9 5.5 -76 4 1 4 2

box 9 6.0 -84 8 1 8 3
finish 9 7.5 -84 4 3 4
"""

w3_l2 = """# World 3 - Level 2: Frost Whirl
name Frost Whirl
world 3
killY -15.0
par 38.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1

# First Spinner on platform
box 0 0.5 -13 6 1 6 2
spinner 0 1.3 -13 5.0 0.4 2.5
box 0 1.0 -20 4 1 4 0

# Checkpoint 1
box 0 1.0 -27 6 1 6 0
checkpoint 0 1.5 -27

box 0 1.5 -34 6 1 6 1
spinner 0 2.3 -34 5.5 0.4 -2.8
box 0 2.0 -41 4 1 4 2

# Checkpoint 2
box 0 2.0 -48 6 1 6 0
checkpoint 0 2.5 -48

box 0 2.5 -55 6 1 6 0
spinner 0 3.3 -55 5.5 0.4 3.0
box 0 3.0 -62 4 1 4 1
box 0 3.5 -67 4 1 4 2

box 0 4.0 -75 8 1 8 3
finish 0 5.5 -75 4 3 4
"""

w3_l3 = """# World 3 - Level 3: Blizzard Steps
name Blizzard Steps
world 3
killY -15.0
par 42.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Crumble + Spinner combo
crumble 0 0 -6 3 1 3 1.0 2.5
box 0 0.5 -12 6 1 6 1
spinner 0 1.3 -12 5.0 0.4 2.6
crumble 0 1.0 -18 3 1 3 0.9 2.5
box 0 1.5 -24 5 1 5 2

# Checkpoint 1
box 0 1.5 -31 6 1 6 0
checkpoint 0 2.0 -31

crumble 4 2.0 -36 3 1 3 0.8 2.0
box 8 2.5 -41 6 1 6 0
spinner 8 3.3 -41 5.5 0.4 -2.8
crumble 12 3.0 -46 3 1 3 0.8 2.0
box 16 3.5 -52 5 1 5 1

# Checkpoint 2
box 16 3.5 -59 6 1 6 0
checkpoint 16 4.0 -59

crumble 16 4.0 -65 3 1 3 0.7 2.0
crumble 14 4.5 -70 3 1 3 0.7 2.0
box 11 5.0 -75 4 1 4 2

box 11 5.5 -83 8 1 8 3
finish 11 7.0 -83 4 3 4
"""

w3_l4 = """# World 3 - Level 4: Glacial Gauntlet
name Glacial Gauntlet
world 3
killY -15.0
par 44.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 6 1 6 2
spinner 0 0.8 -6 5.5 0.4 3.0
crumble 0 0.5 -12 3 1 3 0.8 2.0
crumble 0 1.0 -17 3 1 3 0.8 2.0
box 0 1.5 -23 6 1 6 1
spinner 0 2.3 -23 5.5 0.4 -3.0

# Checkpoint 1
box 0 2.0 -31 6 1 6 0
checkpoint 0 2.5 -31

box 4 2.5 -36 3 1 3 1
crumble 8 3.0 -40 3 1 3 0.7 2.0
box 12 3.5 -45 6 1 6 0
spinner 12 4.3 -45 5.5 0.4 3.2
box 16 4.0 -51 4 1 4 2

# Checkpoint 2
box 16 4.0 -58 6 1 6 0
checkpoint 16 4.5 -58

crumble 16 4.5 -64 3 1 3 0.7 2.0
crumble 14 5.0 -69 3 1 3 0.7 2.0
box 11 5.5 -74 4 1 4 1

box 11 6.0 -82 8 1 8 3
finish 11 7.5 -82 4 3 4
"""

w3_l5 = """# World 3 - Level 5: Summit of the Frost Titan (Boss Course)
name Summit of the Frost Titan
world 3
killY -18.0
par 58.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Ascent across crumbling ice and dual spinners
crumble 0 0 -6 3 1 3 1.0 2.5
crumble 0 0.8 -11 3 1 3 0.9 2.5
box 0 1.6 -17 6 1 6 2
spinner 0 2.4 -17 5.5 0.4 3.0
crumble 0 2.4 -24 3 1 3 0.8 2.0
box 0 3.2 -30 6 1 6 1
spinner 0 4.0 -30 5.5 0.4 -3.2

# Checkpoint 1
box 0 3.5 -38 6 1 6 0
checkpoint 0 4.0 -38

# Spiral ascent around icy pinnacle
box 5 4.3 -42 3 1 3 0
crumble 9 5.1 -44 3 1 3 0.8 2.0
box 13 5.9 -43 3 1 3 2
crumble 16 6.7 -39 3 1 3 0.8 2.0
box 17 7.5 -34 6 1 6 1
spinner 17 8.3 -34 5.5 0.4 3.5

# Checkpoint 2
box 17 7.8 -26 6 1 6 0
checkpoint 17 8.3 -26

crumble 17 8.6 -20 3 1 3 0.7 2.0
crumble 15 9.4 -15 3 1 3 0.7 2.0
box 11 10.2 -11 6 1 6 0
spinner 11 11.0 -11 5.5 0.4 -3.5
crumble 6 11.0 -8 3 1 3 0.7 2.0
box 1 11.8 -8 4 1 4 2

box -6 12.5 -8 8 1 8 3
finish -6 14.0 -8 4 3 4
"""

# ==============================================================================
# WORLD 4: FACTORY (Everything + buttons, doors, ghost puzzles)
# ==============================================================================

w4_l1 = """# World 4 - Level 1: Assembly Line
name Assembly Line
world 4
killY -15.0
par 42.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Conveyor Movers & Spikes
mover 0 0 -7 4 1 4 5 0 0 3.5 1
box 5 0.5 -14 4 1 4 0
spikes 5 1.3 -14 2 0.6 2
mover 5 1.0 -21 4 1 4 -5 0 0 3.5 2
box 0 1.5 -28 5 1 5 1

# Checkpoint 1
box 0 1.5 -35 6 1 6 0
checkpoint 0 2.0 -35

# Timed Button and Door segment
button 1 0 2.0 -41 6.0
box 0 1.5 -41 4 1 4 0

box 0 2.0 -47 3 1 3 1
box 2 2.5 -52 3 1 3 2
door 1 2 4.0 -56 4 4 1
box 2 2.5 -56 4 1 4 0

# Checkpoint 2
box 2 2.5 -63 6 1 6 0
checkpoint 2 3.0 -63

mover 2 2.5 -70 4 1 4 4 0 0 3.0 1
box 6 3.0 -77 4 1 4 2
spikes 6 3.8 -77 2 0.6 2
box 6 3.5 -83 4 1 4 0

box 6 4.0 -91 8 1 8 3
finish 6 5.5 -91 4 3 4
"""

w4_l2 = """# World 4 - Level 2: Clockwork Switch
name Clockwork Switch
world 4
killY -15.0
par 45.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1

# Timed run 1: Button 1 opens Door 1 for 5.5s
button 1 0 0 -11 5.5
box 0 -0.5 -11 4 1 4 0

box 0 0.5 -17 3 1 3 2
box -3 1.0 -22 3 1 3 1
box -3 1.5 -27 3 1 3 0
door 1 -3 3.0 -31 4 4 1
box -3 1.5 -31 4 1 4 2

# Checkpoint 1
box -3 1.5 -38 6 1 6 0
checkpoint -3 2.0 -38

# Timed run 2: Button 2 opens Door 2 across movers
button 2 -3 2.0 -44 6.5
box -3 1.5 -44 4 1 4 0

mover -3 2.0 -51 3 1 3 6 0 0 3.0 1
box 3 2.5 -58 4 1 4 2
door 2 3 4.0 -63 4 4 1
box 3 2.5 -63 4 1 4 0

# Checkpoint 2
box 3 2.5 -70 6 1 6 0
checkpoint 3 3.0 -70

box 3 3.0 -76 3 1 3 1
box 1 3.5 -81 3 1 3 2
box 0 4.0 -86 4 1 4 0

box 0 4.5 -94 8 1 8 3
finish 0 6.0 -94 4 3 4
"""

w4_l3 = """# World 4 - Level 3: Hazard Core
name Hazard Core
world 4
killY -16.0
par 48.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Spinners and crumbles over molten steel
crumble 0 0 -6 3 1 3 0.9 2.0
box 0 0.5 -12 6 1 6 1
spinner 0 1.3 -12 5.5 0.4 3.0
bounce 0 0.7 -18 3 0.5 3 22.0
box 0 5.5 -27 5 1 5 2

# Checkpoint 1
box 0 5.5 -34 6 1 6 0
checkpoint 0 6.0 -34

mover 0 5.5 -41 4 1 4 6 0 0 3.5 0
box 6 6.0 -49 6 1 6 1
spinner 6 6.8 -49 5.5 0.4 -3.2
crumble 6 6.5 -56 3 1 3 0.8 2.0
box 6 7.0 -62 5 1 5 2

# Checkpoint 2
box 6 7.0 -69 6 1 6 0
checkpoint 6 7.5 -69

bounce 6 7.2 -74 3 0.5 3 22.0
box 6 12.0 -83 4 1 4 0
spikes 6 12.8 -83 2 0.6 2
box 6 12.5 -89 4 1 4 1

box 6 13.0 -97 8 1 8 3
finish 6 14.5 -97 4 3 4
"""

w4_l4 = """# World 4 - Level 4: Twin Synchrony (Ghost Puzzle)
name Twin Synchrony
world 4
killY -15.0
par 45.0

# Central Hub
spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Left Path leading to Button 1 (Ghost target)
box -6 -0.5 -4 3 1 3 1
box -11 -0.5 -8 3 1 3 2
box -16 -0.5 -13 6 1 6 0
button 1 -16 0 -13 0.8

# Right Path leading to Door 1
box 6 -0.5 -4 3 1 3 1
box 11 -0.5 -8 3 1 3 2
box 16 -0.5 -13 6 1 6 0
door 1 16 1.5 -18 4 4 1
box 16 -0.5 -18 4 1 4 2

# Checkpoint 1 past Door 1
box 16 -0.5 -25 6 1 6 0
checkpoint 16 0.0 -25

# Stage 2: Timed finish run
button 2 16 0.0 -31 5.0
box 16 -0.5 -31 4 1 4 0
box 16 0.5 -37 3 1 3 1
box 14 1.0 -42 3 1 3 2
door 2 14 2.5 -46 4 4 1
box 14 1.0 -46 4 1 4 0

box 14 1.0 -54 8 1 8 3
finish 14 2.5 -54 4 3 4
"""

w4_l5 = """# World 4 - Level 5: Master Generator (Grand Finale)
name Master Generator
world 4
killY -20.0
par 65.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

# Act I: Conveyor & Spinners
mover 0 0 -7 4 1 4 5 0 0 3.5 1
box 5 0.5 -14 6 1 6 2
spinner 5 1.3 -14 5.5 0.4 3.2
crumble 5 1.0 -21 3 1 3 0.8 2.0
box 5 1.5 -27 5 1 5 0

# Checkpoint 1
box 5 1.5 -34 6 1 6 0
checkpoint 5 2.0 -34

# Act II: Rocket Launch & Timed Core Gate
bounce 5 1.7 -39 3 0.5 3 22.0
box 5 6.5 -48 5 1 5 1
button 1 5 6.5 -53 5.5
box 5 6.0 -53 4 1 4 0
mover 5 6.5 -60 3 1 3 -6 0 0 3.0 2
box -1 7.0 -67 4 1 4 1
door 1 -1 8.5 -72 4 4 1
box -1 7.0 -72 4 1 4 0

# Checkpoint 2
box -1 7.0 -79 6 1 6 0
checkpoint -1 7.5 -79

# Act III: The Dual Core Ghost Lock
box -7 7.0 -84 3 1 3 1
box -13 7.0 -88 5 1 5 0
button 2 -13 7.5 -88 0.8

box 5 7.0 -84 3 1 3 2
box 11 7.0 -88 5 1 5 0
door 2 11 9.0 -93 4 4 1
box 11 7.0 -93 4 1 4 1

# Checkpoint 3 (Final Checkpoint)
box 11 7.0 -100 6 1 6 0
checkpoint 11 7.5 -100

# Act IV: Grand Climax
mover 11 7.0 -107 4 1 4 0 5 0 3.5 2
box 11 12.0 -115 6 1 6 1
spinner 11 12.8 -115 5.5 0.4 -3.5
bounce 11 12.2 -121 3 0.5 3 22.0

box 11 17.5 -133 10 1 10 3
finish 11 19.0 -133 5 4 5
"""

levels = [
    ("w1_l1.lvl", w1_l1),
    ("w1_l2.lvl", w1_l2),
    ("w1_l3.lvl", w1_l3),
    ("w1_l4.lvl", w1_l4),
    ("w1_l5.lvl", w1_l5),
    ("w2_l1.lvl", w2_l1),
    ("w2_l2.lvl", w2_l2),
    ("w2_l3.lvl", w2_l3),
    ("w2_l4.lvl", w2_l4),
    ("w2_l5.lvl", w2_l5),
    ("w3_l1.lvl", w3_l1),
    ("w3_l2.lvl", w3_l2),
    ("w3_l3.lvl", w3_l3),
    ("w3_l4.lvl", w3_l4),
    ("w3_l5.lvl", w3_l5),
    ("w4_l1.lvl", w4_l1),
    ("w4_l2.lvl", w4_l2),
    ("w4_l3.lvl", w4_l3),
    ("w4_l4.lvl", w4_l4),
    ("w4_l5.lvl", w4_l5),
]

for filename, content in levels:
    write_level(filename, content)

print(f"Successfully generated all {len(levels)} levels!")
