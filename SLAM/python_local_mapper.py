from __future__ import annotations

import csv
import math
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Optional


@dataclass(frozen=True)
class Pose2D:
    x_cm: float
    y_cm: float
    yaw_deg: float


@dataclass(frozen=True)
class SensorFrame:
    timestamp_ms: int
    dt_s: float
    motion_mode: str
    front_cm: Optional[float]
    left_cm: Optional[float]
    right_cm: Optional[float]
    roll_deg: Optional[float] = None
    pitch_deg: Optional[float] = None
    gyro_z_deg_s: Optional[float] = None


@dataclass(frozen=True)
class MapperConfig:
    width_cells: int = 80
    height_cells: int = 80
    resolution_cm: float = 5.0
    nominal_forward_speed_cm_s: float = 10.0
    fallback_turn_rate_deg_s: float = 30.0
    gyro_bias_deg_s: float = 0.0
    min_valid_range_cm: float = 5.0
    max_valid_range_cm: float = 300.0
    max_tilt_for_mapping_deg: float = 15.0
    position_correction_gain: float = 0.25
    correction_gate_cm: float = 40.0
    occupied_threshold: int = 3
    free_step: int = -1
    occupied_step: int = 3
    min_log_odds: int = -40
    max_log_odds: int = 40
    min_occupied_cells_for_correction: int = 6
    front_sensor_offset_x_cm: float = 5.0
    front_sensor_offset_y_cm: float = 0.0
    front_sensor_yaw_deg: float = 0.0
    left_sensor_offset_x_cm: float = 0.0
    left_sensor_offset_y_cm: float = 5.0
    left_sensor_yaw_deg: float = 90.0
    right_sensor_offset_x_cm: float = 0.0
    right_sensor_offset_y_cm: float = -5.0
    right_sensor_yaw_deg: float = -90.0


@dataclass(frozen=True)
class MapBuildResult:
    final_pose: Pose2D
    grid: "OccupancyGridMap"
    frames_processed: int
    known_cells: int
    occupied_cells: int


@dataclass(frozen=True)
class SensorExtrinsics:
    offset_x_cm: float
    offset_y_cm: float
    yaw_deg: float


class OccupancyGridMap:
    def __init__(
        self,
        width_cells: int,
        height_cells: int,
        resolution_cm: float,
        occupied_threshold: int,
        free_step: int = -1,
        occupied_step: int = 3,
        min_log_odds: int = -40,
        max_log_odds: int = 40,
    ) -> None:
        if width_cells <= 0 or height_cells <= 0:
            raise ValueError("grid dimensions must be positive")
        if resolution_cm <= 0.0:
            raise ValueError("grid resolution must be positive")

        self.width_cells = width_cells
        self.height_cells = height_cells
        self.resolution_cm = resolution_cm
        self.occupied_threshold = occupied_threshold
        self.free_step = free_step
        self.occupied_step = occupied_step
        self.min_log_odds = min_log_odds
        self.max_log_odds = max_log_odds
        self._cells = [0] * (width_cells * height_cells)

    def clear(self) -> None:
        for index in range(len(self._cells)):
            self._cells[index] = 0

    def world_to_cell(self, x_cm: float, y_cm: float) -> Optional[tuple[int, int]]:
        half_width = self.width_cells * 0.5
        half_height = self.height_cells * 0.5

        cell_x = math.floor(x_cm / self.resolution_cm + half_width)
        cell_y = math.floor(y_cm / self.resolution_cm + half_height)
        if cell_x < 0 or cell_y < 0:
            return None
        if cell_x >= self.width_cells or cell_y >= self.height_cells:
            return None
        return int(cell_x), int(cell_y)

    def get_world_value(self, x_cm: float, y_cm: float) -> int:
        cell = self.world_to_cell(x_cm, y_cm)
        if cell is None:
            return 0
        return self.get_cell(*cell)

    def get_cell(self, cell_x: int, cell_y: int) -> int:
        if cell_x < 0 or cell_y < 0:
            return 0
        if cell_x >= self.width_cells or cell_y >= self.height_cells:
            return 0
        return self._cells[self._flatten_index(cell_x, cell_y)]

    def insert_ray(
        self,
        origin: Pose2D,
        beam_yaw_deg: float,
        range_cm: float,
        max_range_cm: float,
        hit: bool,
    ) -> None:
        clipped_range_cm = max(0.0, min(range_cm, max_range_cm))
        step_cm = max(1.0, self.resolution_cm * 0.5)
        beam_yaw_rad = math.radians(beam_yaw_deg)

        distance_cm = 0.0
        while distance_cm < clipped_range_cm:
            x_cm = origin.x_cm + math.cos(beam_yaw_rad) * distance_cm
            y_cm = origin.y_cm + math.sin(beam_yaw_rad) * distance_cm
            cell = self.world_to_cell(x_cm, y_cm)
            if cell is None:
                break
            self._add_log_odds(cell[0], cell[1], self.free_step)
            distance_cm += step_cm

        if not hit or clipped_range_cm >= max_range_cm:
            return

        hit_x_cm = origin.x_cm + math.cos(beam_yaw_rad) * clipped_range_cm
        hit_y_cm = origin.y_cm + math.sin(beam_yaw_rad) * clipped_range_cm
        hit_cell = self.world_to_cell(hit_x_cm, hit_y_cm)
        if hit_cell is not None:
            self._add_log_odds(hit_cell[0], hit_cell[1], self.occupied_step)

    def raycast_range_cm(
        self,
        origin: Pose2D,
        beam_yaw_deg: float,
        max_range_cm: float,
    ) -> float:
        step_cm = max(1.0, self.resolution_cm * 0.5)
        beam_yaw_rad = math.radians(beam_yaw_deg)

        distance_cm = 0.0
        while distance_cm <= max_range_cm:
            x_cm = origin.x_cm + math.cos(beam_yaw_rad) * distance_cm
            y_cm = origin.y_cm + math.sin(beam_yaw_rad) * distance_cm
            cell = self.world_to_cell(x_cm, y_cm)
            if cell is None:
                return distance_cm
            if self.get_cell(*cell) >= self.occupied_threshold:
                return distance_cm
            distance_cm += step_cm
        return max_range_cm

    def count_known_cells(self) -> int:
        return sum(1 for value in self._cells if value != 0)

    def count_occupied_cells(self) -> int:
        return sum(1 for value in self._cells if value >= self.occupied_threshold)

    def render_ascii(
        self,
        pose: Optional[Pose2D] = None,
        max_rows: int = 40,
        max_cols: int = 40,
    ) -> str:
        row_step = max(1, math.ceil(self.height_cells / max_rows))
        col_step = max(1, math.ceil(self.width_cells / max_cols))
        robot_cell = None if pose is None else self.world_to_cell(pose.x_cm, pose.y_cm)

        lines: list[str] = []
        for cell_y in range(self.height_cells - 1, -1, -row_step):
            row_chars: list[str] = []
            for cell_x in range(0, self.width_cells, col_step):
                if robot_cell is not None and abs(robot_cell[0] - cell_x) < col_step and abs(robot_cell[1] - cell_y) < row_step:
                    row_chars.append("R")
                    continue

                value = self.get_cell(cell_x, cell_y)
                if value >= self.occupied_threshold:
                    row_chars.append("#")
                elif value < 0:
                    row_chars.append(".")
                else:
                    row_chars.append(" ")
            lines.append("".join(row_chars))
        return "\n".join(lines)

    def _add_log_odds(self, cell_x: int, cell_y: int, delta: int) -> None:
        index = self._flatten_index(cell_x, cell_y)
        updated = self._cells[index] + delta
        updated = max(self.min_log_odds, updated)
        updated = min(self.max_log_odds, updated)
        self._cells[index] = updated

    def _flatten_index(self, cell_x: int, cell_y: int) -> int:
        return cell_y * self.width_cells + cell_x


def load_sensor_frames(csv_path: str | Path) -> list[SensorFrame]:
    path = Path(csv_path)
    frames: list[SensorFrame] = []
    previous_timestamp_ms: Optional[int] = None

    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        for raw_row in reader:
            row = {
                key.lstrip("\ufeff").strip(): value
                for key, value in raw_row.items()
                if key is not None
            }
            timestamp_ms = _read_required_int(row, ("timestamp_ms", "millis"))
            dt_s = 0.0
            if previous_timestamp_ms is not None:
                dt_s = max(0.0, (timestamp_ms - previous_timestamp_ms) * 0.001)
            previous_timestamp_ms = timestamp_ms

            imu_valid = _read_optional_flag(row, ("imu_valid",))
            roll_deg = _read_optional_float(row, ("roll_deg",))
            pitch_deg = _read_optional_float(row, ("pitch_deg",))
            gyro_z_deg_s = _read_optional_float(row, ("gyro_z_deg_s",))
            if imu_valid is False:
                roll_deg = None
                pitch_deg = None
                gyro_z_deg_s = None

            # 先尝试直接读 motion_mode 文本字段
            motion_text = _read_optional_text(row, ("motion_mode", "motion"))
            if motion_text is None:
                # learning 36 列格式没有 motion_mode，从指令拆分字段推断
                fwd_total = _read_optional_float(row, ("forward_cmd_total", "forward_cmd_base"))
                yaw_total = _read_optional_float(row, ("yaw_cmd_total", "yaw_cmd_base"))
                if fwd_total is not None and fwd_total > 5.0:
                    motion_text = "forward"
                elif yaw_total is not None and yaw_total > 5.0:
                    motion_text = "turn_left"
                elif yaw_total is not None and yaw_total < -5.0:
                    motion_text = "turn_right"
                else:
                    motion_text = "idle"

            frames.append(
                SensorFrame(
                    timestamp_ms=timestamp_ms,
                    dt_s=dt_s,
                    motion_mode=_normalize_motion_mode(motion_text, gyro_z_deg_s),
                    front_cm=_read_optional_range_cm(row, ("us_front_cm", "front_cm", "front_distance_cm")),
                    left_cm=_read_optional_range_cm(row, ("us_left_cm", "left_cm", "left_distance_cm")),
                    right_cm=_read_optional_range_cm(row, ("us_right_cm", "right_cm", "right_distance_cm")),
                    roll_deg=roll_deg,
                    pitch_deg=pitch_deg,
                    gyro_z_deg_s=gyro_z_deg_s,
                )
            )

    return frames


def build_map_from_frames(
    frames: Iterable[SensorFrame],
    config: Optional[MapperConfig] = None,
) -> MapBuildResult:
    mapper_config = config or MapperConfig()
    grid = OccupancyGridMap(
        width_cells=mapper_config.width_cells,
        height_cells=mapper_config.height_cells,
        resolution_cm=mapper_config.resolution_cm,
        occupied_threshold=mapper_config.occupied_threshold,
        free_step=mapper_config.free_step,
        occupied_step=mapper_config.occupied_step,
        min_log_odds=mapper_config.min_log_odds,
        max_log_odds=mapper_config.max_log_odds,
    )
    pose = Pose2D(0.0, 0.0, 0.0)

    front_sensor = SensorExtrinsics(
        mapper_config.front_sensor_offset_x_cm,
        mapper_config.front_sensor_offset_y_cm,
        mapper_config.front_sensor_yaw_deg,
    )
    left_sensor = SensorExtrinsics(
        mapper_config.left_sensor_offset_x_cm,
        mapper_config.left_sensor_offset_y_cm,
        mapper_config.left_sensor_yaw_deg,
    )
    right_sensor = SensorExtrinsics(
        mapper_config.right_sensor_offset_x_cm,
        mapper_config.right_sensor_offset_y_cm,
        mapper_config.right_sensor_yaw_deg,
    )

    frame_count = 0
    for frame in frames:
        frame_count += 1
        pose = _propagate_pose(pose, frame, mapper_config)

        if _should_skip_mapping(frame, mapper_config):
            continue

        if grid.count_occupied_cells() >= mapper_config.min_occupied_cells_for_correction:
            pose = _apply_position_correction(
                pose=pose,
                frame=frame,
                grid=grid,
                config=mapper_config,
                front_sensor=front_sensor,
                left_sensor=left_sensor,
                right_sensor=right_sensor,
            )

        _insert_measurement(grid, pose, frame.front_cm, front_sensor, mapper_config)
        _insert_measurement(grid, pose, frame.left_cm, left_sensor, mapper_config)
        _insert_measurement(grid, pose, frame.right_cm, right_sensor, mapper_config)

    return MapBuildResult(
        final_pose=pose,
        grid=grid,
        frames_processed=frame_count,
        known_cells=grid.count_known_cells(),
        occupied_cells=grid.count_occupied_cells(),
    )


def _propagate_pose(pose: Pose2D, frame: SensorFrame, config: MapperConfig) -> Pose2D:
    yaw_rate_deg_s = 0.0
    if frame.gyro_z_deg_s is not None:
        yaw_rate_deg_s = frame.gyro_z_deg_s - config.gyro_bias_deg_s
    elif frame.motion_mode == "turn_left":
        yaw_rate_deg_s = config.fallback_turn_rate_deg_s
    elif frame.motion_mode == "turn_right":
        yaw_rate_deg_s = -config.fallback_turn_rate_deg_s

    yaw_deg = _wrap_degrees(pose.yaw_deg + yaw_rate_deg_s * frame.dt_s)

    forward_speed_cm_s = 0.0
    if frame.motion_mode == "forward":
        forward_speed_cm_s = config.nominal_forward_speed_cm_s

    yaw_rad = math.radians(yaw_deg)
    x_cm = pose.x_cm + math.cos(yaw_rad) * forward_speed_cm_s * frame.dt_s
    y_cm = pose.y_cm + math.sin(yaw_rad) * forward_speed_cm_s * frame.dt_s
    return Pose2D(x_cm=x_cm, y_cm=y_cm, yaw_deg=yaw_deg)


def _apply_position_correction(
    pose: Pose2D,
    frame: SensorFrame,
    grid: OccupancyGridMap,
    config: MapperConfig,
    front_sensor: SensorExtrinsics,
    left_sensor: SensorExtrinsics,
    right_sensor: SensorExtrinsics,
) -> Pose2D:
    correction_x_cm = 0.0
    correction_y_cm = 0.0
    correction_count = 0

    for range_cm, sensor in (
        (frame.front_cm, front_sensor),
        (frame.left_cm, left_sensor),
        (frame.right_cm, right_sensor),
    ):
        if not _is_valid_range(range_cm, config):
            continue

        sensor_pose = _compute_sensor_pose(pose, sensor)
        beam_yaw_deg = pose.yaw_deg + sensor.yaw_deg
        expected_cm = grid.raycast_range_cm(
            origin=sensor_pose,
            beam_yaw_deg=beam_yaw_deg,
            max_range_cm=config.max_valid_range_cm,
        )
        if expected_cm >= (config.max_valid_range_cm - config.resolution_cm):
            continue

        residual_cm = range_cm - expected_cm
        if abs(residual_cm) > config.correction_gate_cm:
            continue

        correction_cm = (expected_cm - range_cm) * config.position_correction_gain
        beam_yaw_rad = math.radians(beam_yaw_deg)
        correction_x_cm += math.cos(beam_yaw_rad) * correction_cm
        correction_y_cm += math.sin(beam_yaw_rad) * correction_cm
        correction_count += 1

    if correction_count == 0:
        return pose

    scale = 1.0 / correction_count
    return Pose2D(
        x_cm=pose.x_cm + correction_x_cm * scale,
        y_cm=pose.y_cm + correction_y_cm * scale,
        yaw_deg=pose.yaw_deg,
    )


def _insert_measurement(
    grid: OccupancyGridMap,
    pose: Pose2D,
    range_cm: Optional[float],
    sensor: SensorExtrinsics,
    config: MapperConfig,
) -> None:
    if not _is_valid_range(range_cm, config):
        return

    sensor_pose = _compute_sensor_pose(pose, sensor)
    grid.insert_ray(
        origin=sensor_pose,
        beam_yaw_deg=pose.yaw_deg + sensor.yaw_deg,
        range_cm=range_cm,
        max_range_cm=config.max_valid_range_cm,
        hit=True,
    )


def _compute_sensor_pose(pose: Pose2D, sensor: SensorExtrinsics) -> Pose2D:
    robot_yaw_rad = math.radians(pose.yaw_deg)
    sensor_x_cm = pose.x_cm + math.cos(robot_yaw_rad) * sensor.offset_x_cm - math.sin(robot_yaw_rad) * sensor.offset_y_cm
    sensor_y_cm = pose.y_cm + math.sin(robot_yaw_rad) * sensor.offset_x_cm + math.cos(robot_yaw_rad) * sensor.offset_y_cm
    return Pose2D(
        x_cm=sensor_x_cm,
        y_cm=sensor_y_cm,
        yaw_deg=_wrap_degrees(pose.yaw_deg + sensor.yaw_deg),
    )


def _should_skip_mapping(frame: SensorFrame, config: MapperConfig) -> bool:
    if frame.roll_deg is not None and abs(frame.roll_deg) > config.max_tilt_for_mapping_deg:
        return True
    if frame.pitch_deg is not None and abs(frame.pitch_deg) > config.max_tilt_for_mapping_deg:
        return True
    return False


def _is_valid_range(range_cm: Optional[float], config: MapperConfig) -> bool:
    return range_cm is not None and config.min_valid_range_cm <= range_cm <= config.max_valid_range_cm


def _normalize_motion_mode(raw_motion: Optional[str], gyro_z_deg_s: Optional[float]) -> str:
    if raw_motion is None:
        return _infer_turn_from_gyro(gyro_z_deg_s)

    normalized = raw_motion.strip().lower()
    if normalized == "":
        return _infer_turn_from_gyro(gyro_z_deg_s)

    if normalized in {"forward", "fwd", "w", "1"}:
        return "forward"
    if normalized in {"left", "turn_left", "left_turn", "a"}:
        return "turn_left"
    if normalized in {"right", "turn_right", "right_turn", "d"}:
        return "turn_right"
    if normalized in {"idle", "stop", "s", "0"}:
        return "idle"

    if normalized.isdigit():
        motion_flags = int(normalized)
        if motion_flags & 0x01:
            return "forward"
        if motion_flags & 0x02:
            return _infer_turn_from_gyro(gyro_z_deg_s)

    if normalized == "turn":
        return _infer_turn_from_gyro(gyro_z_deg_s)

    return "idle"


def _infer_turn_from_gyro(gyro_z_deg_s: Optional[float]) -> str:
    if gyro_z_deg_s is None:
        return "idle"
    if gyro_z_deg_s > 0.5:
        return "turn_left"
    if gyro_z_deg_s < -0.5:
        return "turn_right"
    return "idle"


def _wrap_degrees(angle_deg: float) -> float:
    while angle_deg > 180.0:
        angle_deg -= 360.0
    while angle_deg < -180.0:
        angle_deg += 360.0
    return angle_deg


def _read_required_int(row: dict[str, str], field_names: tuple[str, ...]) -> int:
    text = _read_optional_text(row, field_names)
    if text is None:
        raise ValueError(f"missing required field from {field_names}")
    return int(float(text))


def _read_optional_float(row: dict[str, str], field_names: tuple[str, ...]) -> Optional[float]:
    text = _read_optional_text(row, field_names)
    if text is None:
        return None
    stripped = text.strip()
    if stripped in ("--", "nan", "NaN", "NAN", ""):
        return None
    try:
        return float(stripped)
    except ValueError:
        return None


def _read_optional_range_cm(row: dict[str, str], field_names: tuple[str, ...]) -> Optional[float]:
    value = _read_optional_float(row, field_names)
    if value is None or value <= 0.0:
        return None
    return value


def _read_optional_text(row: dict[str, str], field_names: tuple[str, ...]) -> Optional[str]:
    for field_name in field_names:
        value = row.get(field_name)
        if value is None:
            continue
        if value.strip() == "":
            continue
        return value
    return None


def _read_optional_flag(row: dict[str, str], field_names: tuple[str, ...]) -> Optional[bool]:
    text = _read_optional_text(row, field_names)
    if text is None:
        return None
    lowered = text.strip().lower()
    if lowered in {"1", "true", "yes"}:
        return True
    if lowered in {"0", "false", "no"}:
        return False
    return None


def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(
        description="Build a local occupancy map from ESP32 sensor CSV data."
    )
    parser.add_argument("csv_path", help="Path to ESP32 sensor CSV or enriched telemetry CSV.")
    parser.add_argument(
        "--ascii-map",
        action="store_true",
        help="Print an ASCII preview of the local occupancy map.",
    )
    args = parser.parse_args()

    frames = load_sensor_frames(args.csv_path)
    result = build_map_from_frames(frames)

    print(f"frames_processed={result.frames_processed}")
    print(
        "final_pose_cm="
        f"({result.final_pose.x_cm:.2f}, {result.final_pose.y_cm:.2f}, yaw={result.final_pose.yaw_deg:.2f} deg)"
    )
    print(f"known_cells={result.known_cells}")
    print(f"occupied_cells={result.occupied_cells}")

    if args.ascii_map:
        print(result.grid.render_ascii(result.final_pose))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
