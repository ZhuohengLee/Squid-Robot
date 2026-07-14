# SLAM Analysis for IMU + Left/Front/Right Ultrasonic Sensors

## 1. Current Project Situation

Based on the existing codebase:

- The project already has three ultrasonic sensors connected to the ESP32:
  - `front`
  - `left`
  - `right`
- The ultrasonic pipeline is implemented in:
  - [ESP32/UltrasonicManager.h](../ESP32/UltrasonicManager.h)
  - [ESP32/UltrasonicManager.cpp](../ESP32/UltrasonicManager.cpp)
- The current ultrasonic manager:
  - validates frames
  - rejects out-of-range readings
  - applies a per-sensor Kalman filter
  - treats valid range as roughly `50 mm` to `3000 mm`
- The navigation logic currently uses the three sensors only for obstacle avoidance, not for mapping.
- The repository currently does not contain a concrete IMU driver implementation, but the learning/telemetry side already expects IMU-related fields such as:
  - `imu_valid`
  - `roll_deg`
  - `pitch_deg`
  - `gyro_x_deg_s`
  - `gyro_y_deg_s`
  - `gyro_z_deg_s`

Relevant files:

- [ESP32/AutoNavigator.cpp](../ESP32/AutoNavigator.cpp)
- [ESP32/ESP32.ino](../ESP32/ESP32.ino)
- [learning/tests/test_data.py](../learning/tests/test_data.py)

This means the project is already close to a minimal sensor-fusion pipeline, but not yet a SLAM system.

## 2. What This Sensor Set Can and Cannot Do

### What is feasible

With:

- 1 IMU
- 3 single-beam ultrasonic range sensors
- a depth sensor already present in the project

you can build a **constrained, low-speed, local SLAM or mapping system** in environments such as:

- narrow corridors
- pool walls
- tanks
- structured indoor channels
- spaces where left/right/front boundaries are often visible

The most realistic target is:

- **2D SLAM on a fixed-depth slice**, or
- **2.5D local mapping** where depth is controlled separately

### What is not realistically feasible

This sensor set is not enough for robust general-purpose SLAM in:

- open water
- large empty spaces
- scenes with few nearby surfaces
- highly cluttered environments requiring dense geometry

The main limitation is observability:

- The IMU can stabilize attitude and short-term rotation, but position from pure IMU integration drifts very fast.
- Three ultrasonic sensors give only three sparse distance rays at each instant.
- There is no wheel encoder, DVL, visual odometry, scanning lidar, or multibeam sonar to provide reliable translational odometry.

So the correct framing is:

- **not full generic SLAM**
- **yes to structure-aware local mapping and pose correction**

## 3. Recommended Problem Formulation

For this robot, the most practical formulation is:

- Assume motion is mostly on a horizontal plane.
- Use the depth sensor to hold or annotate `z`.
- Use the IMU mainly for `roll`, `pitch`, and especially `yaw rate`.
- Use the three ultrasonic sensors as sparse wall observations.
- Build a **2D occupancy grid** or **line-based wall map**.

Recommended state:

```text
x = [px, py, yaw, v, gyro_bias_z]
```

If depth needs to be included:

```text
x = [px, py, pz, yaw, v, gyro_bias_z]
```

But in practice, keeping SLAM in 2D and treating depth separately is the safer first version.

## 4. Sensor Roles

### IMU

Use the IMU for:

- short-term orientation propagation
- yaw estimation from gyroscope integration
- roll/pitch compensation
- motion gating during rapid maneuvers

Do **not** rely on the IMU alone for long-term `x/y` position.

### Front ultrasonic

Use for:

- obstacle distance ahead
- front wall alignment
- validating corridor closure
- scan matching against forward obstacles

### Left and right ultrasonic

Use for:

- lateral wall distance
- corridor centering
- heading correction from wall symmetry
- detecting turns, corners, and junction asymmetry

### Depth sensor

Even though your question focused on IMU and ultrasonics, this project already has depth sensing, so it should be part of the final system:

- maintain a fixed slice of the environment
- reduce the SLAM problem from 3D to 2D
- tag the map with depth if you later want stacked slices

## 5. Core Idea: Use IMU for Prediction, Ultrasonic for Correction

The system should work as:

1. **Prediction step**
   - Use IMU gyro `gyro_z` to propagate heading.
   - Use a simple motion model to predict forward displacement.
2. **Correction step**
   - Compare predicted left/front/right distances against the map.
   - Use the difference to correct pose.
3. **Mapping step**
   - Cast three measurement rays into the map.
   - Mark free space along the ray.
   - Mark the end cell as occupied if the return is valid.

This is fundamentally a sensor-fusion problem:

- IMU handles short-term dynamics
- ultrasonic handles absolute geometric constraints

## 6. The Hard Part: Translational Odometry

The hardest missing piece in your hardware is not heading, but translation.

You currently do not have a direct measurement of:

- forward speed
- lateral speed
- displacement per propulsion cycle

That means `px, py` cannot be estimated well from IMU alone.

### Practical ways to handle this

#### Option A: Command-model odometry

Estimate displacement from actuator commands:

- when forward propulsion runs, assume approximate speed `v_cmd`
- when turning runs, assume little forward displacement
- correct that estimate whenever wall observations disagree

This is simple, but drift will be large.

#### Option B: Wall-constrained odometry

When the robot is in a corridor or near walls:

- use left/right/front distances plus yaw
- infer motion from how those distances evolve over time

This is much better than pure IMU integration, but only works when nearby boundaries are visible.

#### Option C: Active scan motion

Periodically rotate the robot in place or do a small left-right sweep:

- collect multiple ultrasonic readings at different headings
- reconstruct a denser local scan
- use that scan for map matching

With only three beams, this is one of the highest-leverage upgrades you can do without new hardware.

## 7. Best SLAM Architecture for This Robot

### Recommended first version

Use:

- **EKF localization + occupancy grid mapping**

or, if the map matching becomes too nonlinear:

- **particle filter / FastSLAM-style localization**

Why this is better than jumping straight to pose-graph SLAM:

- the sensor set is sparse
- the environment is likely structured
- the robot speed is low
- the first goal should be stable local pose estimation, not elegant loop closure

### Suggested map representation

#### Choice 1: Occupancy grid

Best if you want something simple and robust.

- 2D grid
- each ultrasonic sensor contributes one ray
- maintain log-odds occupancy
- use ray casting for expected measurement

Pros:

- straightforward
- tolerant of noisy data
- easy to visualize

Cons:

- sparse beams make the map fill slowly
- open space remains uncertain

#### Choice 2: Line or wall-segment map

Best if the environment is corridor-like.

- represent walls as line segments
- fit lines from repeated left/right/front observations over motion
- localize by distance to expected lines

Pros:

- matches your sensor geometry well
- lighter than a dense grid

Cons:

- harder to initialize
- weaker in irregular environments

For your project, I would start with **occupancy grid first**, then add line fitting if the environment is strongly corridor-like.

## 8. Measurement Model

Assume the three ultrasonic sensors have fixed mounting angles in the robot frame:

- front: `0 deg`
- left: `+90 deg`
- right: `-90 deg`

If there is any mounting offset, those angles must be calibrated.

For each sensor:

1. Start from robot pose `(px, py, yaw)`.
2. Transform the sensor beam direction into the world frame.
3. Ray-cast into the map.
4. Compute expected range `z_hat`.
5. Compare with measured range `z`.
6. Update pose and map if the residual is reasonable.

Residual:

```text
r = z - z_hat
```

Use gating:

- reject stale data
- reject sudden impossible jumps
- reject returns during sharp roll/pitch
- reject measurements near sensor minimum/maximum range

## 9. Why Roll/Pitch Compensation Matters

In this robot, roll and pitch will change the effective beam direction.

Without compensation:

- a side sensor may falsely appear closer or farther
- front range can shift during pitch motion
- mapping quality will degrade badly during maneuvers

So before using an ultrasonic reading for SLAM:

1. read current IMU orientation
2. compensate the beam direction
3. only then project the beam into the map

If roll/pitch exceeds a threshold, it is often better to skip the update entirely.

## 10. Recommended Data Pipeline

### Stage 1: Sensor preprocessing

- synchronize timestamps for IMU, ultrasonic, depth
- estimate IMU gyro bias at startup
- calibrate ultrasonic extrinsics:
  - position offset from robot center
  - beam angle
- continue using the existing ultrasonic validity and Kalman filtering

### Stage 2: State propagation

At each control loop:

- update yaw from `gyro_z`
- estimate forward displacement from command-model odometry
- keep depth separately from the depth sensor

### Stage 3: Measurement update

When one or more ultrasonic readings are valid:

- transform beams using current attitude
- compare against expected ranges from the local map
- update pose estimate

### Stage 4: Mapping

- mark ray cells as free
- mark hit cells as occupied
- decay confidence slowly for old cells if desired

### Stage 5: Active scan

Every few seconds, or at corners:

- stop forward motion
- rotate a little left and right
- gather multiple readings
- perform local scan matching

This will improve map density more than almost any software-only tweak.

## 11. What to Implement First

### Phase 1: Local mapping only

Do not start with global SLAM.

Implement:

- local 2D occupancy grid centered on the robot
- IMU-based heading estimate
- front/left/right ray insertion
- visualization of the local map

Success criterion:

- the robot can produce a stable local obstacle map while moving slowly

### Phase 2: Pose correction

Add:

- predicted pose from heading + command-model displacement
- map-based correction from ultrasonic residuals

Success criterion:

- the local map stops smearing badly during turns and straight motion

### Phase 3: Corridor localization

Add:

- wall symmetry logic from left/right distances
- heading correction when left/right geometry is stable
- front-wall alignment logic

Success criterion:

- the robot can stay centered and estimate pose better in structured spaces

### Phase 4: Limited loop closure

Add:

- recognition of repeated local wall signatures
- relocalization when re-entering a known corridor segment

This is optional and should come only after local mapping works.

## 12. Strong Recommendation for This Hardware

If you want useful results quickly, target this:

- **local obstacle mapping**
- **corridor localization**
- **heading stabilization**
- **wall-referenced motion correction**

Do not target:

- large-scale global SLAM
- full 3D SLAM
- precise long-range dead reckoning

That would require at least one of:

- scanning sonar
- DVL
- visual odometry camera
- more range sensors
- servo-scanned ultrasonic head

## 13. Concrete Conclusion

Using only IMU plus left/front/right ultrasonic sensors, your robot can do:

- local map building
- obstacle-aware localization in structured spaces
- wall-based pose correction
- limited SLAM under strong environmental constraints

It cannot do robust general-purpose global SLAM in open environments because:

- IMU translation drifts too fast
- three ultrasonic beams are too sparse
- there is no direct velocity or odometry source

So the right engineering path is:

1. add IMU driver and timestamped sensor fusion
2. build a 2D local occupancy grid
3. use IMU for heading propagation
4. use ultrasonic returns for map correction
5. add active yaw scanning to densify observations

## 14. Suggested Next Implementation Tasks

If you want to continue from this document, the next practical work items in this repository are:

1. Add a real IMU driver on the ESP32 side.
2. Extend telemetry/logging to include IMU timestamps and filtered yaw.
3. Export `front/left/right` ultrasonic data plus motion state from ESP32.
4. If possible, also export `imu_valid`, `roll_deg`, `pitch_deg`, and `gyro_z_deg_s`.
5. Feed those CSV rows into the Python mapper in this folder.
6. Start with local mapping visualization before attempting closed-loop autonomy.

## 15. Python Mapper For ESP32 Sensor Data

This folder now contains the Python implementation for the workflow you described:

- `ESP32` reads sensors
- you export or send the sensor data
- Python builds the local map

Main file:

- [python_local_mapper.py](</D:/working/squid robot/code/SLAM/python_local_mapper.py:1>)

Tests:

- [test_python_local_mapper.py](</D:/working/squid robot/code/SLAM/tests/test_python_local_mapper.py:1>)

## 16. Method Used In The Python Version

The Python mapper uses this practical SLAM approach:

- **local 2D occupancy-grid mapping**
- **IMU yaw propagation when available**
- **command-model odometry for forward motion**
- **three-beam ultrasonic ray insertion**
- **lightweight map-residual position correction**

This is the exact method:

1. Read one frame of ESP32 sensor data.
2. Use `gyro_z_deg_s` to propagate heading if IMU is available.
3. If the robot is moving forward, estimate short-horizon displacement from a nominal forward speed.
4. Project `front / left / right` ultrasonic readings as three rays into a local occupancy grid.
5. If the map already contains enough occupied structure, ray-cast from the predicted pose and compare map-expected range against measured range.
6. Apply a small translational correction along the beam direction.

So this Python version is still:

- not graph SLAM
- not loop-closure SLAM
- not particle-filter SLAM

It is:

- **a sparse-beam local SLAM / local mapping pipeline**

That is the right level for your current sensor set.

## 17. Supported Input Formats

The Python loader currently supports two practical CSV shapes.

### Format A: Current ESP32 `sensors.csv`

Current fields already supported:

```text
millis,depth_cm,vz_cms,az_cms2,us_front_cm,us_left_cm,us_right_cm,motion,batt_v
```

### Format B: Enriched telemetry with IMU

Also supported:

```text
timestamp_ms,us_front_cm,us_left_cm,us_right_cm,motion_mode,roll_deg,pitch_deg,gyro_z_deg_s
```

Optional IMU-related fields:

- `imu_valid`
- `roll_deg`
- `pitch_deg`
- `gyro_z_deg_s`

If IMU fields are missing, the mapper still works, but pose prediction is weaker.

## 18. How To Run The Python Mapper

Example:

```bash
python -m SLAM.python_local_mapper path\\to\\sensor_data.csv --ascii-map
```

It prints:

- processed frame count
- final estimated pose
- known cell count
- occupied cell count
- optional ASCII map preview

## 19. Current Python Verification

The Python version was verified with unit tests covering:

- loading the current ESP32 `sensors.csv` format
- loading enriched telemetry with IMU columns
- building a non-empty map from sequential frames
- marking occupied hit cells correctly
- building a non-empty map from a synthetic corridor test set
