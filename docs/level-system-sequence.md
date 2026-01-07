# Level System Sequence Diagrams

## Overview

This document contains sequence diagrams showing the flow of data and control in the dynamic level system.

---

## 1. Build Time: Level Generation

```mermaid
sequenceDiagram
    participant YAML as level1.yaml
    participant Gen as generate_level.py
    participant Header as level1.h

    YAML->>Gen: read YAML file
    Gen->>Gen: parse segments
    Gen->>Gen: parse objects
    Gen->>Gen: pack data into structs
    Gen->>Header: write C header
```

---

## 2. Game Initialization

```mermaid
sequenceDiagram
    participant Main as main.c
    participant Game as game.c
    participant Level as level.c
    participant Tilemap as tilemap.c
    participant LevelDef as level1.h

    Main->>Game: game_init()
    Game->>Level: level_init(&level1_def)
    Level->>LevelDef: load segments
    LevelDef-->>Level: segment data
    Level->>LevelDef: load objects
    LevelDef-->>Level: object data
    Level->>Level: segment_idx = 0
    Level->>Level: block_counter = length
    Level->>Level: calculate boundaries
    Level->>Tilemap: tilemap_init()
    Tilemap->>Tilemap: generate initial rows
    Level-->>Game: ready
    Game-->>Main: ready
```

---

## 3. Game Loop: Frame Update

```mermaid
sequenceDiagram
    participant Main as main.c
    participant Game as game.c
    participant Level as level.c
    participant Player as player.c
    participant Collision as collision.c
    participant Tilemap as tilemap.c

    Main->>Main: wait_vblank()
    Main->>Game: game_update()

    Game->>Level: level_update()
    Level->>Level: block_counter--
    alt block_counter == 0
        Level->>Level: advance_segment()
    end
    Level->>Level: check_objects()
    Level-->>Game: done

    Game->>Player: player_update(input)
    Player-->>Game: done

    Game->>Player: player_check_level()
    Player->>Level: level_get_boundaries()
    Level-->>Player: left, right bounds
    Player->>Player: check vs boundaries
    Player-->>Game: CRASH_NONE or CRASH_LEVEL

    Game->>Collision: collision_check_hole()
    Collision->>Level: level_check_object()
    Level-->>Collision: object type
    Collision-->>Game: result

    Game->>Tilemap: tilemap_scroll(scroll_y)
    Tilemap->>Level: level_generate_row()
    Level-->>Tilemap: tile data
    Tilemap->>Tilemap: write to tilemap RAM

    Game-->>Main: done
```

---

## 4. Segment Transition

```mermaid
sequenceDiagram
    participant Level as level.c
    participant State as LevelState

    Note over Level: block_counter reaches 0

    Level->>State: segment_idx++
    Level->>State: load new segment
    State->>State: block_counter = segment.length
    State->>State: current_lanes = SEGMENT_LANES()
    State->>State: current_width = SEGMENT_WIDTH()

    alt lanes changed
        Level->>State: check for transition zone
        State->>State: in_transition = connect flag
        State->>State: transition_counter = 4
    end

    Level->>Level: recalculate boundaries

    alt LANE_CENTER
        Level->>State: left = center - width*8
        Level->>State: right = center + width*8
    else LANE_BOTH
        Level->>State: left_lane = center - gap - w*8
        Level->>State: right_lane = center + gap + w*8
    else LANE_LEFT
        Level->>State: offset left from center
    else LANE_RIGHT
        Level->>State: offset right from center
    end

    Level->>State: obj_idx = segment.obj_offset
    Level->>State: obj_segment_end = obj_idx + obj_count
```

---

## 5. Tilemap Row Generation

```mermaid
sequenceDiagram
    participant Tilemap as tilemap.c
    participant Level as level.c
    participant RAM as Tilemap RAM

    Note over Tilemap: new row scrolled in

    Tilemap->>Level: level_generate_row(row, scroll_y)

    Level->>Level: determine segment for this row
    Level->>Level: get lane config (lanes, width)
    Level->>Level: calculate tile positions
    Level->>Level: check for objects at this row
    Level-->>Tilemap: tile data array

    loop for each tile in row
        Tilemap->>RAM: tilemap[row][col] = tile_index
    end
```

---

## 6. Collision Check Flow

```mermaid
sequenceDiagram
    participant Player as player.c
    participant Level as level.c
    participant Game as game.h

    Player->>Player: player_check_level()
    Player->>Level: level_get_boundaries(&left, &right)

    alt LANE_CENTER/LEFT/RIGHT
        Level-->>Player: left_lane_left, left_lane_right
    else LANE_BOTH + in_transition
        Level-->>Player: expanded bounds (lanes connected)
    else LANE_BOTH
        Level-->>Player: both lane boundaries
    end

    Player->>Player: player_center = x + w/2

    alt LANE_BOTH && !transition
        Player->>Player: check if in left OR right lane
    else single lane or transition
        Player->>Player: check if within bounds
    end

    alt outside bounds
        Player-->>Game: return CRASH_LEVEL
    else inside bounds
        Player-->>Game: return CRASH_NONE
    end
```

---

## 7. Object Trigger Flow

```mermaid
sequenceDiagram
    participant Level as level.c
    participant Collision as collision.c
    participant Game as game.c
    participant Sound as sound.c

    Note over Level: during level_update()

    Level->>Level: check objects at current scroll

    loop for obj in current segment
        alt obj.at == current_block
            Level->>Level: trigger object
        end
    end

    alt OBJ_HOLE
        Level->>Level: mark tiles as hole
    else OBJ_LASER
        Level->>Level: activate laser at position
    else OBJ_POWERUP
        Level->>Game: spawn powerup sprite
    else OBJ_ENEMY_SPAWN
        Level->>Game: trigger enemy spawn
        Game->>Game: enemies_spawn()
    end

    Note over Collision: collision detected with player

    Game->>Collision: collision_check_hole()
    Collision->>Level: level_check_object()
    Level-->>Collision: object at position

    alt over hole
        Collision-->>Game: hole collision
        Game->>Game: reduce score
        Game->>Game: shake screen
        Game->>Sound: sound_hole()
    end
```

---

## 8. Complete Frame Overview

```mermaid
flowchart TD
    subgraph Frame["GAME FRAME"]
        A[1. Wait for VBlank] --> B[2. Read Input]
        B --> C[3. Update Level State]
        C --> D[4. Update Player]
        D --> E[5. Update Game Objects]
        E --> F[6. Update Tilemap]
        F --> G[7. Render]
        G --> H[8. Update Sound]
    end

    subgraph LevelUpdate["3. Level Update"]
        C1[level_update] --> C2[decrement block_counter]
        C2 --> C3{block_counter == 0?}
        C3 -->|yes| C4[advance to next segment]
        C4 --> C5[update lanes/width]
        C5 --> C6[recalculate boundaries]
        C6 --> C7[reset object tracking]
        C3 -->|no| C8[trigger objects at position]
        C7 --> C8
    end

    subgraph PlayerUpdate["4. Player Update"]
        D1[player_update] --> D2[move based on input]
        D2 --> D3[player_check_level]
        D3 --> D4[level_get_boundaries]
        D4 --> D5{within bounds?}
        D5 -->|no| D6[player_hit, reset position]
        D5 -->|yes| D7[continue]
    end

    subgraph ObjectUpdate["5. Game Objects"]
        E1[bullets_update] --> E2[enemies_update]
        E2 --> E3[collision_bullets_enemies]
        E3 --> E4[collision_player_enemies]
        E4 --> E5[collision_check_hole]
        E5 --> E6[level_check_object]
    end

    subgraph TilemapUpdate["6. Tilemap"]
        F1[tilemap_scroll] --> F2{new row visible?}
        F2 -->|yes| F3[level_generate_row]
        F3 --> F4[get segment config]
        F4 --> F5[calculate tile positions]
        F5 --> F6[write tiles to RAM]
    end

    subgraph Render["7. Render"]
        G1[render_hud_text] --> G2[player_render]
        G2 --> G3[bullets_render]
        G3 --> G4[enemies_render_shadows]
        G4 --> G5[enemies_render]
    end

    C --> LevelUpdate
    D --> PlayerUpdate
    E --> ObjectUpdate
    F --> TilemapUpdate
    G --> Render
```

---

## 9. Data Flow Summary

```mermaid
flowchart LR
    subgraph BuildTime["BUILD TIME"]
        YAML[level1.yaml<br/>- segments<br/>- objects]
        Gen[generate_level.py]
        YAML --> Gen
    end

    subgraph ROM["ROM (Read Only)"]
        Header[level1.h]
        LevelDef[LevelDef<br/>8 bytes<br/>- name<br/>- segments ptr<br/>- objects ptr]
        Segments[LevelSegment array<br/>4 bytes each]
        Objects[LevelObject array<br/>2 bytes each]

        Gen --> Header
        Header --> LevelDef
        LevelDef --> Segments
        LevelDef --> Objects
    end

    subgraph RAM["RAM (~20 bytes)"]
        State[LevelState<br/>- segment_idx<br/>- block_counter<br/>- boundaries<br/>- transitions]
    end

    subgraph Runtime["RUNTIME"]
        Collision[collision.c]
        Player[player.c]
        Tilemap[tilemap.c]
    end

    Segments --> State
    Objects --> State
    State --> Collision
    State --> Player
    State --> Tilemap
```

---

## 10. Lane Configuration Visualization

```mermaid
flowchart TB
    subgraph CENTER["LANE_CENTER"]
        C1["        "]
        C2["[========]"]
        C3["        "]
    end

    subgraph LEFT["LANE_LEFT"]
        L1["    "]
        L2["[========]"]
        L3["            "]
    end

    subgraph RIGHT["LANE_RIGHT"]
        R1["            "]
        R2["[========]"]
        R3["    "]
    end

    subgraph BOTH["LANE_BOTH"]
        B1["    "]
        B2["[====]"]
        B3[" "]
        B4["[====]"]
        B5["    "]
    end
```

---

## 11. Segment Lifecycle State Machine

```mermaid
stateDiagram-v2
    [*] --> Loading: level_init()
    Loading --> Active: first segment loaded

    Active --> Active: block_counter > 0<br/>decrement counter
    Active --> Transitioning: block_counter == 0<br/>next segment has different lanes
    Active --> NextSegment: block_counter == 0<br/>same lane config

    Transitioning --> Active: transition_counter == 0
    Transitioning --> Transitioning: transition_counter > 0<br/>lanes connected

    NextSegment --> Active: load segment data<br/>recalculate bounds
    NextSegment --> [*]: segment.length == 0<br/>level complete
```

---

## 12. Object Processing Pipeline

```mermaid
flowchart LR
    subgraph Input["Object Definition (ROM)"]
        OBJ[LevelObject<br/>at: block position<br/>data: type/lane/size]
    end

    subgraph Check["Trigger Check"]
        TC{current_block<br/>== obj.at?}
    end

    subgraph Process["Object Processing"]
        HOLE[Write hole tiles<br/>to tilemap]
        LASER[Activate laser<br/>hazard]
        POWER[Spawn powerup<br/>sprite]
        ENEMY[Trigger enemy<br/>spawn]
        SPEED[Modify scroll<br/>speed]
    end

    subgraph Collision["Collision Detection"]
        COLL{Player<br/>overlaps?}
        DAMAGE[Apply damage<br/>or effect]
    end

    OBJ --> TC
    TC -->|yes| Process
    TC -->|no| Skip[Skip]

    Process --> HOLE & LASER & POWER & ENEMY & SPEED
    HOLE --> COLL
    LASER --> COLL
    POWER --> COLL

    COLL -->|yes| DAMAGE
    COLL -->|no| Continue[Continue]
```
